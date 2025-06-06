#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "../include/video_player.h"
#include "../include/common.h"

// FFmpeg video player state
static AVFormatContext *pFormatCtx = NULL;
static AVCodecContext *pVideoCodecCtx = NULL;
static AVCodecContext *pAudioCodecCtx = NULL;
static AVStream *pVideoStream = NULL;
static AVStream *pAudioStream = NULL;
static AVFrame *pFrame = NULL;
static AVFrame *pFrameYUV = NULL;
static AVFrame *pAudioFrame = NULL;
static AVPacket packet;
static struct SwsContext *sws_ctx = NULL;
static struct SwrContext *swr_ctx = NULL;

// SDL_mixer sound
static Mix_Chunk *pSoundTrack = NULL;
static SDL_Texture *texture = NULL;
static int videoStream = -1;
static int audioStream = -1;
static Uint32 video_start_time = 0;
static double video_clock = 0.0;
static double audio_clock = 0.0;
static bool quit = false;

// Audio buffer
#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

// Audio callback structure
typedef struct PacketQueue {
    AVPacket *packets;
    int size;
    int capacity;
    int read_index;
    int write_index;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

static PacketQueue audioq;
static Uint8 *audio_buf = NULL;
static unsigned int audio_buf_size = 0;
static unsigned int audio_buf_index = 0;
static SDL_AudioDeviceID audio_dev = 0;

// Function declarations for audio
static void packet_queue_init(PacketQueue *q);
static int packet_queue_put(PacketQueue *q, AVPacket *pkt);
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
static void packet_queue_flush(PacketQueue *q);
int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);
void audio_callback(void *userdata, Uint8 *stream, int len);

// Audio callback function
void audio_callback(void *userdata, Uint8 *stream, int len) {
    static int callback_count = 0;
    AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
    int len1, audio_size;
    
    // Log every 100th callback to avoid flooding the console
    if (callback_count % 100 == 0) {
        printf("Audio callback: len=%d, buf_index=%d, buf_size=%d\n", 
               len, audio_buf_index, audio_buf_size);
    }
    callback_count++;
    
    // Clear the stream with silence first
    memset(stream, 0, len);
    
    // While we have space in the output stream buffer and haven't quit
    while (len > 0 && !quit) {
        // If we have run out of audio data in our buffer, get more
        if (audio_buf_index >= audio_buf_size) {
            audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
            if (audio_size < 0) {
                // Error, output silence
                audio_buf_size = 1024;
                memset(audio_buf, 0, audio_buf_size);
                if (callback_count % 100 == 0) {
                    printf("Audio decode error, outputting silence\n");
                }
            } else {
                audio_buf_size = audio_size;
                if (callback_count % 100 == 0) {
                    printf("Decoded audio: size=%d\n", audio_size);
                }
            }
            audio_buf_index = 0;
        }
        
        // How much data we have left to copy
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len)
            len1 = len;
        
        // Copy data to output stream
        memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
        
        // Update pointers and counters
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

// Audio decoding function
int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {
    static AVPacket pkt;
    static AVFrame frame;
    int data_size = 0;
    int ret;
    
    // Get a packet from the queue
    if (packet_queue_get(&audioq, &pkt, 1) < 0) {
        return -1;
    }
    
    // Send packet to decoder
    ret = avcodec_send_packet(aCodecCtx, &pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending packet for audio decoding\n");
        av_packet_unref(&pkt);
        return -1;
    }
    
    // Initialize frame
    av_frame_unref(&frame);
    
    // Receive frame from decoder
    ret = avcodec_receive_frame(aCodecCtx, &frame);
    if (ret < 0) {
        if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error during audio decoding\n");
        }
        av_packet_unref(&pkt);
        return 0;
    }
    
    // Calculate the number of samples in the frame
    int nb_samples = frame.nb_samples;
    int channels = frame.ch_layout.nb_channels;
    
    // Calculate the size of the audio data
    data_size = av_samples_get_buffer_size(NULL, channels, nb_samples, AV_SAMPLE_FMT_S16, 1);
    if (data_size <= 0) {
        fprintf(stderr, "av_samples_get_buffer_size failed\n");
        av_packet_unref(&pkt);
        return 0;
    }
    
    // Make sure we don't overflow the buffer
    if (data_size > buf_size) {
        fprintf(stderr, "Audio buffer too small\n");
        data_size = buf_size;
    }
    
    // Convert the samples from the decoded format to S16
    if (swr_ctx) {
        const uint8_t **in = (const uint8_t **)frame.extended_data;
        uint8_t *out[] = {audio_buf};
        int out_count = buf_size / channels / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        
        ret = swr_convert(swr_ctx, out, out_count, in, nb_samples);
        if (ret < 0) {
            fprintf(stderr, "swr_convert() failed\n");
            av_packet_unref(&pkt);
            return 0;
        }
        
        data_size = ret * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    } else {
        // No resampling needed, just copy
        memcpy(audio_buf, frame.data[0], data_size);
    }
    
    // Update audio clock with pts
    if (pkt.pts != AV_NOPTS_VALUE) {
        audio_clock = av_q2d(pAudioStream->time_base) * pkt.pts;
    }
    
    // Free the packet
    av_packet_unref(&pkt);
    
    return data_size;
}

// Packet queue functions
static void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->capacity = 100; // Initial capacity
    q->packets = av_malloc(sizeof(AVPacket) * q->capacity);
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    // Lock the queue
    SDL_LockMutex(q->mutex);
    
    // Check if we need to resize the queue
    if (q->size >= q->capacity) {
        int new_capacity = q->capacity * 2;
        AVPacket *new_packets = av_realloc(q->packets, sizeof(AVPacket) * new_capacity);
        if (!new_packets) {
            SDL_UnlockMutex(q->mutex);
            return -1;
        }
        q->packets = new_packets;
        q->capacity = new_capacity;
        
        // Rearrange the packets if needed
        if (q->read_index > q->write_index) {
            // Queue has wrapped around, need to make it contiguous
            int old_capacity = q->capacity / 2;
            for (int i = 0; i < q->read_index; i++) {
                q->packets[old_capacity + i] = q->packets[i];
            }
            q->write_index += old_capacity;
        }
    }
    
    // Make a copy of the packet
    if (av_packet_make_refcounted(pkt) < 0) {
        SDL_UnlockMutex(q->mutex);
        return -1;
    }
    
    // Add the packet to the queue
    av_packet_move_ref(&q->packets[q->write_index], pkt);
    q->write_index = (q->write_index + 1) % q->capacity;
    q->size++;
    
    // Signal that a new packet is available
    SDL_CondSignal(q->cond);
    
    // Unlock the queue
    SDL_UnlockMutex(q->mutex);
    
    return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    int ret;
    
    // Lock the queue
    SDL_LockMutex(q->mutex);
    
    for (;;) {
        // Check if we're quitting
        if (quit) {
            ret = -1;
            break;
        }
        
        // Check if there are packets in the queue
        if (q->size > 0) {
            // Get the packet at the read index
            av_packet_move_ref(pkt, &q->packets[q->read_index]);
            q->read_index = (q->read_index + 1) % q->capacity;
            q->size--;
            ret = 1;
            break;
        } else if (!block) {
            // No packet and we're not blocking
            ret = 0;
            break;
        } else {
            // No packet, but we're blocking, so wait
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    
    // Unlock the queue
    SDL_UnlockMutex(q->mutex);
    
    return ret;
}

static void packet_queue_flush(PacketQueue *q) {
    // Lock the queue
    SDL_LockMutex(q->mutex);
    
    // Free all packets in the queue
    for (int i = 0; i < q->size; i++) {
        int index = (q->read_index + i) % q->capacity;
        av_packet_unref(&q->packets[index]);
    }
    
    // Reset the queue
    q->read_index = 0;
    q->write_index = 0;
    q->size = 0;
    
    // Unlock the queue
    SDL_UnlockMutex(q->mutex);
}

bool initVideoPlayer(SDL_Renderer *renderer) {
    // Initialize FFmpeg
    static bool ffmpeg_initialized = false;
    if (!ffmpeg_initialized) {
        // Register all formats and codecs
        #if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
            av_register_all();
        #endif
        ffmpeg_initialized = true;
    }
    
    // Reset state variables
    pFormatCtx = NULL;
    pVideoCodecCtx = NULL;
    pAudioCodecCtx = NULL;
    pVideoStream = NULL;
    pAudioStream = NULL;
    pFrame = NULL;
    pFrameYUV = NULL;
    pAudioFrame = NULL;
    sws_ctx = NULL;
    swr_ctx = NULL;
    texture = NULL;
    audio_dev = 0;
    videoStream = -1;
    audioStream = -1;
    quit = false;
    audio_buf = NULL;
    audio_buf_size = 0;
    audio_buf_index = 0;
    video_clock = 0.0;
    audio_clock = 0.0;
    
    // Initialize packet queue
    packet_queue_init(&audioq);
    
    // Allocate audio buffer
    audio_buf = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE);
    if (!audio_buf) {
        fprintf(stderr, "Could not allocate audio buffer\n");
        return false;
    }
    
    return true;
}

bool playVideo(SDL_Renderer *renderer, const char *videoPath) {
    // Load the separate sound file
    pSoundTrack = Mix_LoadWAV("resources/KungligaProjectSound.wav");
    if (!pSoundTrack) {
        fprintf(stderr, "Could not load sound file: %s\n", Mix_GetError());
        // Continue without sound
    } else {
        printf("Sound file loaded successfully\n");
    }
    int ret;
    AVCodec *pVideoCodec = NULL;
    AVCodec *pAudioCodec = NULL;
    uint8_t *buffer = NULL;
    int numBytes;
    SDL_AudioSpec wanted_spec, spec;
    
    // Initialize the video player
    if (!initVideoPlayer(renderer)) {
        return false;
    }
    
    // Open the video file
    ret = avformat_open_input(&pFormatCtx, videoPath, NULL, NULL);
    if (ret < 0) {
        FFMPEG_ERROR("Could not open video file");
    }
    
    // Retrieve stream information
    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0) {
        FFMPEG_ERROR("Could not find stream information");
    }
    
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, videoPath, 0);
    
    // Find the first video and audio stream
    videoStream = -1;
    audioStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0) {
            videoStream = i;
        }
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStream < 0) {
            audioStream = i;
        }
    }
    
    if (videoStream == -1) {
        fprintf(stderr, "Could not find video stream\n");
        return false;
    }
    
    // Get a pointer to the codec context for the video stream
    pVideoCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if (pVideoCodec == NULL) {
        fprintf(stderr, "Unsupported video codec!\n");
        return false;
    }
    
    // Create codec context from codec parameters
    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
    if (!pVideoCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }
    
    ret = avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
    if (ret < 0) {
        fprintf(stderr, "Could not copy video codec parameters to context\n");
        return false;
    }
    
    // Open video codec
    ret = avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL);
    if (ret < 0) {
        FFMPEG_ERROR("Could not open video codec");
    }
    
    // Get the video stream
    pVideoStream = pFormatCtx->streams[videoStream];
    
    // Set up audio if available
    if (audioStream != -1) {
        // Get a pointer to the codec context for the audio stream
        pAudioCodec = avcodec_find_decoder(pFormatCtx->streams[audioStream]->codecpar->codec_id);
        if (pAudioCodec == NULL) {
            fprintf(stderr, "Unsupported audio codec!\n");
            // Continue without audio
            audioStream = -1;
        } else {
            // Create codec context from codec parameters
            pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
            if (!pAudioCodecCtx) {
                fprintf(stderr, "Could not allocate audio codec context\n");
                // Continue without audio
                audioStream = -1;
            } else {
                ret = avcodec_parameters_to_context(pAudioCodecCtx, pFormatCtx->streams[audioStream]->codecpar);
                if (ret < 0) {
                    fprintf(stderr, "Could not copy audio codec parameters to context\n");
                    // Continue without audio
                    audioStream = -1;
                } else {
                    // Open audio codec
                    ret = avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);
                    if (ret < 0) {
                        fprintf(stderr, "Could not open audio codec\n");
                        // Continue without audio
                        audioStream = -1;
                    } else {
                        // Get the audio stream
                        pAudioStream = pFormatCtx->streams[audioStream];
                        
                        // Set up audio resampling if needed
                        if (pAudioCodecCtx->sample_fmt != AV_SAMPLE_FMT_S16) {
                            swr_ctx = swr_alloc();
                            if (!swr_ctx) {
                                fprintf(stderr, "Could not allocate resampler context\n");
                                // Continue without audio
                                audioStream = -1;
                            } else {
                                // Set up the resampler with the appropriate API
                                int out_sample_rate = pAudioCodecCtx->sample_rate;
                                int out_channels = pAudioCodecCtx->ch_layout.nb_channels;
                                
                                // Create a new resampler context
                                swr_ctx = swr_alloc();
                                if (!swr_ctx) {
                                    fprintf(stderr, "Could not allocate resampler context\n");
                                    // Continue without audio
                                    audioStream = -1;
                                } else {
                                    // Set up a simpler resampler configuration
                                    // First, free the existing context and create a new one
                                    swr_free(&swr_ctx);
                                    
                                    // Get the number of channels
                                    int in_channels = pAudioCodecCtx->ch_layout.nb_channels;
                                    if (in_channels <= 0) {
                                        fprintf(stderr, "Invalid number of channels: %d\n", in_channels);
                                        in_channels = 2; // Default to stereo if invalid
                                    }
                                    
                                    // Create a new resampler context
                                    swr_ctx = swr_alloc();
                                    if (!swr_ctx) {
                                        fprintf(stderr, "Could not allocate resampler context\n");
                                        audioStream = -1;
                                    } else {
                                        // Force a standard channel layout based on channel count
                                        uint64_t in_layout = 0;
                                        uint64_t out_layout = 0;
                                        
                                        // Set appropriate channel layout based on channel count
                                        if (in_channels == 1) {
                                            in_layout = AV_CH_LAYOUT_MONO;
                                            out_layout = AV_CH_LAYOUT_MONO;
                                            printf("Using MONO channel layout\n");
                                        } else {
                                            // Default to stereo for 2 or more channels
                                            in_layout = AV_CH_LAYOUT_STEREO;
                                            out_layout = AV_CH_LAYOUT_STEREO;
                                            printf("Using STEREO channel layout\n");
                                        }
                                        
                                        // Set the options
                                        av_opt_set_int(swr_ctx, "in_channel_layout", in_layout, 0);
                                        av_opt_set_int(swr_ctx, "out_channel_layout", out_layout, 0);
                                        av_opt_set_int(swr_ctx, "in_sample_rate", pAudioCodecCtx->sample_rate, 0);
                                        av_opt_set_int(swr_ctx, "out_sample_rate", pAudioCodecCtx->sample_rate, 0);
                                        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", pAudioCodecCtx->sample_fmt, 0);
                                        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
                                    
                                    // Set the sample formats
                                    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", pAudioCodecCtx->sample_fmt, 0);
                                    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
                                
                                    // Initialize the resampling context
                                    ret = swr_init(swr_ctx);
                                    if (ret < 0) {
                                        fprintf(stderr, "Failed to initialize the resampling context\n");
                                        // Continue without audio
                                        audioStream = -1;
                                    }
                                }
                            }
                        }
                        
                        // Set up SDL audio
                        if (audioStream != -1) {
                            wanted_spec.freq = pAudioCodecCtx->sample_rate;
                            wanted_spec.format = AUDIO_S16SYS;
                            wanted_spec.channels = pAudioCodecCtx->ch_layout.nb_channels;
                            wanted_spec.silence = 0;
                            wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
                            wanted_spec.callback = audio_callback;
                            wanted_spec.userdata = pAudioCodecCtx;
                            
                            audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, 0);
                            if (audio_dev == 0) {
                                fprintf(stderr, "SDL_OpenAudioDevice: %s\n", SDL_GetError());
                                // Continue without audio
                                audioStream = -1;
                            } else {
                                // Initialize audio queue
                                packet_queue_init(&audioq);
                                
                                // Print audio format information
                                printf("Audio format: channels=%d, sample_rate=%d, format=%d\n", 
                                       pAudioCodecCtx->ch_layout.nb_channels, 
                                       pAudioCodecCtx->sample_rate,
                                       pAudioCodecCtx->sample_fmt);
                                printf("SDL Audio device: %d, channels=%d, freq=%d\n", 
                                       audio_dev, spec.channels, spec.freq);
                                
                                // Start playing audio
                                SDL_PauseAudioDevice(audio_dev, 0);
                                printf("Audio device started: %d\n", audio_dev);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Allocate video frame
    pFrame = av_frame_alloc();
    if (pFrame == NULL) {
        fprintf(stderr, "Could not allocate frame\n");
        return false;
    }
    
    // Allocate an AVFrame structure for the converted YUV frame
    pFrameYUV = av_frame_alloc();
    if (pFrameYUV == NULL) {
        fprintf(stderr, "Could not allocate frame\n");
        return false;
    }
    
    // Determine required buffer size and allocate buffer
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pVideoCodecCtx->width, pVideoCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) {
        fprintf(stderr, "Could not allocate buffer\n");
        return false;
    }
    
    // Assign appropriate parts of buffer to image planes in pFrameYUV
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, buffer,
                         AV_PIX_FMT_YUV420P, pVideoCodecCtx->width, pVideoCodecCtx->height, 1);
    
    // Initialize SWS context for software scaling
    sws_ctx = sws_getContext(pVideoCodecCtx->width, pVideoCodecCtx->height,
                             pVideoCodecCtx->pix_fmt, pVideoCodecCtx->width, pVideoCodecCtx->height,
                             AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        fprintf(stderr, "Could not initialize the conversion context\n");
        return false;
    }
    
    // Create texture for video
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
                               SDL_TEXTUREACCESS_STREAMING,
                               pVideoCodecCtx->width, pVideoCodecCtx->height);
    if (!texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        return false;
    }
    
    // Initialize packet
    av_packet_unref(&packet);
    packet.data = NULL;
    packet.size = 0;
    
    // Get window dimensions
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    // Calculate aspect ratio and scaling
    float video_aspect = (float)pVideoCodecCtx->width / (float)pVideoCodecCtx->height;
    float window_aspect = (float)windowWidth / (float)windowHeight;
    
    SDL_Rect rect;
    if (video_aspect > window_aspect) {
        // Video is wider than window
        rect.w = windowWidth;
        rect.h = (int)(windowWidth / video_aspect);
        rect.x = 0;
        rect.y = (windowHeight - rect.h) / 2;
    } else {
        // Video is taller than window
        rect.h = windowHeight;
        rect.w = (int)(windowHeight * video_aspect);
        rect.x = (windowWidth - rect.w) / 2;
        rect.y = 0;
    }
    
    // Start audio playback
    if (audioStream != -1) {
        SDL_PauseAudioDevice(audio_dev, 0);
    }
    
    // Play the separate WAV file alongside the video
    if (pSoundTrack) {
        // Play the sound on channel 0, loop 0 times (play once)
        if (Mix_PlayChannel(0, pSoundTrack, 0) == -1) {
            fprintf(stderr, "Could not play sound file: %s\n", Mix_GetError());
        } else {
            printf("Sound playback started\n");
        }
    }
    
    // Main loop
    quit = false;
    video_start_time = SDL_GetTicks();
    SDL_Event event;
    int frameFinished;
    
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                // Skip video on key press or mouse click
                quit = true;
                break;
            }
        }
        
        // Read a packet
        if (av_read_frame(pFormatCtx, &packet) < 0) {
            // End of file
            break;
        }
        
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            ret = avcodec_send_packet(pVideoCodecCtx, &packet);
            if (ret < 0) {
                fprintf(stderr, "Error sending packet for decoding\n");
                break;
            }
            
            while (ret >= 0) {
                ret = avcodec_receive_frame(pVideoCodecCtx, pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    quit = true;
                    break;
                }
                
                // Convert the image from its native format to YUV
                sws_scale(sws_ctx, (const uint8_t * const *)pFrame->data,
                          pFrame->linesize, 0, pVideoCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                
                // Calculate timing
                double pts = 0.0;
                if (pFrame->pts != AV_NOPTS_VALUE) {
                    pts = av_q2d(pVideoStream->time_base) * pFrame->pts;
                }
                video_clock = pts;
                
                // Update the texture with the new YUV frame
                SDL_UpdateYUVTexture(texture, NULL,
                                     pFrameYUV->data[0], pFrameYUV->linesize[0],
                                     pFrameYUV->data[1], pFrameYUV->linesize[1],
                                     pFrameYUV->data[2], pFrameYUV->linesize[2]);
                
                // Clear the renderer
                SDL_RenderClear(renderer);
                
                // Copy the texture to the renderer
                SDL_RenderCopy(renderer, texture, NULL, &rect);
                
                // Update the screen
                SDL_RenderPresent(renderer);
                
                // Control playback speed
                Uint32 current_time = SDL_GetTicks();
                double actual_delay = (pts - (current_time - video_start_time) / 1000.0);
                if (actual_delay > 0.01) {
                    SDL_Delay((Uint32)(actual_delay * 1000.0));
                }
            }
        } else if (packet.stream_index == audioStream) {
            // Queue audio packet
            int result = packet_queue_put(&audioq, &packet);
            if (result < 0) {
                fprintf(stderr, "Error queuing audio packet\n");
            } else {
                printf("Queued audio packet: size=%d\n", packet.size);
            }
        } else {
            // Free the packet that was allocated by av_read_frame
            av_packet_unref(&packet);
        }
    }
    
    // Wait for the audio to finish
    if (audioStream != -1) {
        SDL_Delay(1000);  // Give a second for audio to finish
        SDL_PauseAudioDevice(audio_dev, 1);
    }
    
    // Clean up resources
    if (buffer) {
        av_free(buffer);
        buffer = NULL;
    }
    
    // Clear the screen after the video
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    // Call cleanup to free resources
    cleanupVideoPlayer();
    
    return true;
}

void cleanupVideoPlayer() {
    // Free packet queue
    packet_queue_flush(&audioq);
    
    // Free the sound resource
    if (pSoundTrack) {
        Mix_HaltChannel(0);  // Stop the sound on channel 0
        Mix_FreeChunk(pSoundTrack);
        pSoundTrack = NULL;
        printf("Sound resource freed\n");
    }
    
    // Free audio buffer
    if (audio_buf) {
        av_free(audio_buf);
        audio_buf = NULL;
    }
    
    // Close audio device
    if (audio_dev) {
        SDL_CloseAudioDevice(audio_dev);
        audio_dev = 0;
    }
    
    // Free video and audio resources
    if (pFrameYUV) {
        av_frame_free(&pFrameYUV);
        pFrameYUV = NULL;
    }
    
    if (pFrame) {
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    
    if (pAudioFrame) {
        av_frame_free(&pAudioFrame);
        pAudioFrame = NULL;
    }
    
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = NULL;
    }
    
    if (swr_ctx) {
        swr_free(&swr_ctx);
        swr_ctx = NULL;
    }
    
    if (pVideoCodecCtx) {
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
    }
    
    if (pAudioCodecCtx) {
        avcodec_free_context(&pAudioCodecCtx);
        pAudioCodecCtx = NULL;
    }
    
    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    
    // Reset state variables
    videoStream = -1;
    audioStream = -1;
    quit = false;
    audio_buf_size = 0;
    audio_buf_index = 0;
    video_clock = 0.0;
    audio_clock = 0.0;
}