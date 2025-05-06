/**
 * TheoraPlay; multithreaded Ogg Theora/Ogg Vorbis decoding.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

// I'm ignoring THEORAPLAY_INTERNAL for the sake of this implementation
// This is a simplified version of the original theoraplay.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include <theoraplay.h>
#include <theora/theoradec.h>
#include <vorbis/codec.h>
#include <ogg/ogg.h>

// !!! FIXME: these all count on the pixel format being TH_PF_420 for now.
static unsigned char *THEORAPLAY_CVT_FNPTR_420_rgb(const th_info *tinfo,
                                                  const th_ycbcr_buffer buffer,
                                                  unsigned char *pixels);
static unsigned char *THEORAPLAY_CVT_FNPTR_420_rgba(const th_info *tinfo,
                                                   const th_ycbcr_buffer buffer,
                                                   unsigned char *pixels);
static unsigned char *THEORAPLAY_CVT_FNPTR_420_bgra(const th_info *tinfo,
                                                   const th_ycbcr_buffer buffer,
                                                   unsigned char *pixels);
static unsigned char *THEORAPLAY_CVT_FNPTR_420_rgb565(const th_info *tinfo,
                                                     const th_ycbcr_buffer buffer,
                                                     unsigned char *pixels);

typedef unsigned char *(*ConvertVideoFrameFn)(const th_info *tinfo,
                                             const th_ycbcr_buffer buffer,
                                             unsigned char *pixels);

// !!! FIXME: these volatiles really need to become atomics.
typedef struct TheoraDecoder
{
    // Thread wrangling...
    int thread_created;
    pthread_mutex_t lock;
    volatile int halt;
    pthread_t worker;

    // API state...
    THEORAPLAY_Io *io;
    unsigned int maxframes;
    volatile unsigned int seek_generation;
    volatile int prepped;
    volatile int videostream;
    volatile int audiostream;
    volatile int decode_error;

    ConvertVideoFrameFn vidcvt;

    // Theora data...
    th_info videoinfo;
    th_comment videocomment;
    th_setup_info *videoctx;
    th_dec_ctx *videodec;
    ogg_int64_t videogranulepos;
    double fps;
    int vidfmt;

    // Vorbis data...
    vorbis_info audioinfo;
    vorbis_comment audiocomment;
    vorbis_dsp_state vorbisdsp;
    vorbis_block vorbisblock;
    int vorbis_init_info;
    int vorbis_init_dsp;
    int vorbis_init_block;
    ogg_int64_t audiogranulepos;

    // Ogg and codec state...
    ogg_packet packet;
    ogg_sync_state sync;
    ogg_page page;
    ogg_stream_state vstream;
    ogg_stream_state astream;
    int astream_active;
    int vstream_active;
    int sync_init;
    int astream_init;
    int vstream_init;

    // Linked list of decoded frames/audio...
    volatile THEORAPLAY_VideoFrame *videolist;
    volatile THEORAPLAY_VideoFrame *videolisttail;
    volatile int videocount;
    volatile THEORAPLAY_AudioPacket *audiolist;
    volatile THEORAPLAY_AudioPacket *audiolisttail;
    volatile int audiocount;

    // The THEORAPLAY_Allocator for this instance
    THEORAPLAY_Allocator allocator;
} TheoraDecoder;

static void *allocate_default(const THEORAPLAY_Allocator *allocator, unsigned int len)
{
    return malloc(len);
}

static void deallocate_default(const THEORAPLAY_Allocator *allocator, void *ptr)
{
    free(ptr);
}

static THEORAPLAY_Allocator default_allocator = {
    allocate_default,
    deallocate_default,
    NULL
};

static void initAllocator(TheoraDecoder *decoder, const THEORAPLAY_Allocator *allocator)
{
    if (allocator) {
        decoder->allocator = *allocator;
    } else {
        decoder->allocator = default_allocator;
    }
}

static void *decode_thread(void *_this);  // predeclare.

static void *malloc_decode(TheoraDecoder *decoder, unsigned int len)
{
    return decoder->allocator.allocate(&decoder->allocator, len);
}

static void free_decode(TheoraDecoder *decoder, void *ptr)
{
    decoder->allocator.deallocate(&decoder->allocator, ptr);
}

THEORAPLAY_Decoder *THEORAPLAY_startDecodeFile(const char *fname,
                                               const unsigned int maxframes,
                                               THEORAPLAY_VideoFormat vidfmt,
                                               const THEORAPLAY_Allocator *allocator,
                                               const int multithreaded)
{
    FILE *io = fopen(fname, "rb");
    if (io == NULL)
        return NULL;

    THEORAPLAY_Io *tio = (THEORAPLAY_Io *) malloc(sizeof (THEORAPLAY_Io));
    if (tio == NULL)
    {
        fclose(io);
        return NULL;
    }

    // !!! FIXME: this is going to leak if TheoraDecoder malloc fails.
    // !!! FIXME:  (but you'll have bigger problems if you can't malloc a few bytes.)
    tio->userdata = io;
    tio->read = file_read;
    tio->seek = file_seek;
    tio->streamlen = file_streamlen;
    tio->close = file_close;

    THEORAPLAY_Decoder *retval = THEORAPLAY_startDecode(tio, maxframes, vidfmt, allocator, multithreaded);
    if (retval == NULL)
        tio->close(tio);  // don't leak on error.

    return retval;
}

static long file_read(THEORAPLAY_Io *io, void *buf, long buflen)
{
    FILE *f = (FILE *) io->userdata;
    size_t br = fread(buf, 1, buflen, f);
    if ((br == 0) && ferror(f))
        return -1;
    return (long) br;
}

static int file_seek(THEORAPLAY_Io *io, long absolute_offset)
{
    FILE *f = (FILE *) io->userdata;
    return (fseek(f, absolute_offset, SEEK_SET) == 0) ? 1 : 0;
}

static long file_streamlen(THEORAPLAY_Io *io)
{
    FILE *f = (FILE *) io->userdata;
    const long origpos = ftell(f);
    long retval = -1;
    if (fseek(f, 0, SEEK_END) == 0)
    {
        retval = ftell(f);
        fseek(f, origpos, SEEK_SET);
    }
    return retval;
}

static void file_close(THEORAPLAY_Io *io)
{
    FILE *f = (FILE *) io->userdata;
    fclose(f);
    free(io);
}

THEORAPLAY_Decoder *THEORAPLAY_startDecode(THEORAPLAY_Io *io,
                                           const unsigned int maxframes,
                                           THEORAPLAY_VideoFormat vidfmt,
                                           const THEORAPLAY_Allocator *allocator,
                                           const int multithreaded)
{
    TheoraDecoder *decoder = NULL;
    ConvertVideoFrameFn vidcvt = NULL;

    switch (vidfmt)
    {
        // !!! FIXME: current expects TH_PF_420.
        #define VIDCVT(t) case THEORAPLAY_VIDFMT_##t: vidcvt = THEORAPLAY_CVT_FNPTR_420_##t; break;
        VIDCVT(YV12);
        VIDCVT(IYUV);
        VIDCVT(RGB);
        VIDCVT(RGBA);
        VIDCVT(BGRA);
        VIDCVT(RGB565);
        #undef VIDCVT
        default: goto startdecode_failed;  // invalid/unsupported format.
    }

    decoder = (TheoraDecoder *) malloc(sizeof (TheoraDecoder));
    if (decoder == NULL)
        goto startdecode_failed;

    memset(decoder, '\0', sizeof (TheoraDecoder));
    initAllocator(decoder, allocator);
    decoder->maxframes = maxframes;
    decoder->vidfmt = vidfmt;
    decoder->vidcvt = vidcvt;
    decoder->io = io;

    if (pthread_mutex_init(&decoder->lock, NULL) == 0)
    {
        decoder->thread_created = (pthread_create(&decoder->worker, NULL, decode_thread, decoder) == 0);
        if (decoder->thread_created)
            return (THEORAPLAY_Decoder *) decoder;
        pthread_mutex_destroy(&decoder->lock);
    }

    // !!! FIXME: we're leaking all kinds of memory here.
    free(decoder);

startdecode_failed:
    return NULL;
}

void THEORAPLAY_stopDecode(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    if (!_this)
        return;

    if (_this->thread_created)
    {
        _this->halt = 1;
        pthread_join(_this->worker, NULL);
        pthread_mutex_destroy(&_this->lock);
    }

    if (_this->io)
        _this->io->close(_this->io);

    // !!! FIXME: need to clean up more stuff.
    free(_this);
}

void THEORAPLAY_pumpDecode(THEORAPLAY_Decoder *decoder, const int maxframes)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    if (!_this || _this->thread_created || !_this->prepped)
        return;

    // !!! FIXME: this is a mess.
    // !!! FIXME:  need to clean up the thread stuff.
    decode_thread(_this);
}

int THEORAPLAY_isDecoding(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = ( (!_this->halt) || (_this->videolist) || (_this->audiolist) );
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = ( (_this->videolist) || (_this->audiolist) );
        }
    }

    return retval;
}

int THEORAPLAY_decodingError(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = _this->decode_error;
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = _this->decode_error;
        }
    }

    return retval;
}

int THEORAPLAY_isInitialized(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = _this->prepped;
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = _this->prepped;
        }
    }

    return retval;
}

int THEORAPLAY_hasVideoStream(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = _this->videostream;
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = _this->videostream;
        }
    }

    return retval;
}

int THEORAPLAY_hasAudioStream(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = _this->audiostream;
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = _this->audiostream;
        }
    }

    return retval;
}

unsigned int THEORAPLAY_availableVideo(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    unsigned int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = _this->videocount;
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = _this->videocount;
        }
    }

    return retval;
}

unsigned int THEORAPLAY_availableAudio(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    unsigned int retval = 0;

    if (_this)
    {
        if (_this->thread_created)
        {
            pthread_mutex_lock(&_this->lock);
            retval = _this->audiocount;
            pthread_mutex_unlock(&_this->lock);
        }
        else
        {
            retval = _this->audiocount;
        }
    }

    return retval;
}

const THEORAPLAY_AudioPacket *THEORAPLAY_getAudio(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    THEORAPLAY_AudioPacket *retval;

    if (!_this)
        return NULL;

    if (_this->thread_created)
        pthread_mutex_lock(&_this->lock);

    retval = (THEORAPLAY_AudioPacket *) _this->audiolist;
    if (retval)
    {
        _this->audiolist = retval->next;
        retval->next = NULL;
        if (_this->audiolist == NULL)
            _this->audiolisttail = NULL;
        assert(_this->audiocount > 0);
        _this->audiocount--;
    }

    if (_this->thread_created)
        pthread_mutex_unlock(&_this->lock);

    return retval;
}

void THEORAPLAY_freeAudio(const THEORAPLAY_AudioPacket *item)
{
    if (item != NULL)
    {
        THEORAPLAY_AudioPacket *_item = (THEORAPLAY_AudioPacket *) item;
        free(_item->samples);
        free(_item);
    }
}

const THEORAPLAY_VideoFrame *THEORAPLAY_getVideo(THEORAPLAY_Decoder *decoder)
{
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    THEORAPLAY_VideoFrame *retval;

    if (!_this)
        return NULL;

    if (_this->thread_created)
        pthread_mutex_lock(&_this->lock);

    retval = (THEORAPLAY_VideoFrame *) _this->videolist;
    if (retval)
    {
        _this->videolist = retval->next;
        retval->next = NULL;
        if (_this->videolist == NULL)
            _this->videolisttail = NULL;
        assert(_this->videocount > 0);
        _this->videocount--;
    }

    if (_this->thread_created)
        pthread_mutex_unlock(&_this->lock);

    return retval;
}

void THEORAPLAY_freeVideo(const THEORAPLAY_VideoFrame *item)
{
    if (item != NULL)
    {
        THEORAPLAY_VideoFrame *_item = (THEORAPLAY_VideoFrame *) item;
        free(_item->pixels);
        free(_item);
    }
}

unsigned int THEORAPLAY_seek(THEORAPLAY_Decoder *decoder, unsigned long mspos)
{
    // !!! FIXME: this needs to be way more robust.
    TheoraDecoder *_this = (TheoraDecoder *) decoder;
    unsigned int generation = 0;

    if (!_this)
        return 0;

    if (_this->thread_created)
        pthread_mutex_lock(&_this->lock);

    if (_this->io->seek)
    {
        const long origpos = _this->io->streamlen ? _this->io->streamlen(_this->io) : -1;
        if (origpos != -1)
        {
            if (_this->io->seek(_this->io, 0))
            {
                // !!! FIXME: this will lose pending audio/video fragments.
                // !!! FIXME:  we need to decide what to do with those.
                generation = ++_this->seek_generation;
            }
        }
    }

    if (_this->thread_created)
        pthread_mutex_unlock(&_this->lock);

    return generation;
}

// This is a simple SDL_RWops-style i/o thing.
// !!! FIXME: we can use a callback for i/o, but you'd have to reimplement all of this.
// !!! FIXME:  later on, we can use the Ogg library's callbacks to bridge it, though.

static void *decode_thread(void *data)
{
    // !!! FIXME: this function is a big mess.
    TheoraDecoder *_this = (TheoraDecoder *) data;
    // !!! FIXME: lots of other variables...

    // !!! FIXME: this all needs to be cleaned up at the end.
    ogg_sync_init(&_this->sync);
    _this->sync_init = 1;

    // Main loop.
    while (!_this->halt)
    {
        // !!! FIXME: this is a mess.
        // !!! FIXME:  need to clean up the thread stuff.
        // !!! FIXME:  need to implement the actual decoding.
    }

    return NULL;
}

// YUV420 to RGB conversion functions
static unsigned char *THEORAPLAY_CVT_FNPTR_420_rgb(const th_info *tinfo,
                                                  const th_ycbcr_buffer buffer,
                                                  unsigned char *pixels)
{
    // !!! FIXME: implement this
    return NULL;
}

static unsigned char *THEORAPLAY_CVT_FNPTR_420_rgba(const th_info *tinfo,
                                                   const th_ycbcr_buffer buffer,
                                                   unsigned char *pixels)
{
    // !!! FIXME: implement this
    return NULL;
}

static unsigned char *THEORAPLAY_CVT_FNPTR_420_bgra(const th_info *tinfo,
                                                   const th_ycbcr_buffer buffer,
                                                   unsigned char *pixels)
{
    // !!! FIXME: implement this
    return NULL;
}

static unsigned char *THEORAPLAY_CVT_FNPTR_420_rgb565(const th_info *tinfo,
                                                     const th_ycbcr_buffer buffer,
                                                     unsigned char *pixels)
{
    // !!! FIXME: implement this
    return NULL;
}

// End of theoraplay.c ...
