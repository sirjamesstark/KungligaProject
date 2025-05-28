#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

// FFmpeg kütüphaneleri
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>

#include "../include/video_player.h"

// FFmpeg video oynatıcı durumu
static AVFormatContext *pFormatCtx = NULL;
static AVCodecContext *pVideoCodecCtx = NULL;
static AVStream *pVideoStream = NULL;
static AVFrame *pFrame = NULL;
static AVFrame *pFrameYUV = NULL;
static AVPacket packet;
static struct SwsContext *sws_ctx = NULL;

// SDL nesneleri
static SDL_Texture *texture = NULL;
static int videoStream = -1;
static Uint32 video_start_time = 0;
static double video_clock = 0.0;
static bool quit = false;

// Ses dosyası
static Mix_Chunk *pSoundTrack = NULL;

// Video oynatıcıyı başlat
bool initVideoPlayer(SDL_Renderer *renderer) {
    // FFmpeg durumunu sıfırla
    pFormatCtx = NULL;
    pVideoCodecCtx = NULL;
    pVideoStream = NULL;
    pFrame = NULL;
    pFrameYUV = NULL;
    sws_ctx = NULL;
    texture = NULL;
    videoStream = -1;
    video_clock = 0.0;
    quit = false;
    
    // Frame'leri oluştur
    pFrame = av_frame_alloc();
    if (!pFrame) {
        fprintf(stderr, "Video frame oluşturulamadı\n");
        return false;
    }
    
    pFrameYUV = av_frame_alloc();
    if (!pFrameYUV) {
        fprintf(stderr, "YUV frame oluşturulamadı\n");
        av_frame_free(&pFrame);
        return false;
    }
    
    return true;
}

// Video oynatma
bool playVideo(SDL_Renderer *renderer, const char *videoPath) {
    printf("Video oynatılıyor: %s\n", videoPath);
    
    // Intro ses dosyası yolu
    char soundPath[256];
    snprintf(soundPath, sizeof(soundPath), "resources/intro_sound.wav");
    
    // Ses dosyasını yükle
    pSoundTrack = Mix_LoadWAV(soundPath);
    if (!pSoundTrack) {
        fprintf(stderr, "Intro ses dosyası yüklenemedi: %s\n", Mix_GetError());
        // Alternatif ses dosyasını deneyelim
        snprintf(soundPath, sizeof(soundPath), "resources/KungligaProjectSound.wav");
        pSoundTrack = Mix_LoadWAV(soundPath);
        
        if (!pSoundTrack) {
            fprintf(stderr, "Alternatif ses dosyası da yüklenemedi: %s\n", Mix_GetError());
            // Ses olmadan devam edebiliriz
        } else {
            printf("Alternatif ses dosyası başarıyla yüklendi\n");
        }
    } else {
        printf("Intro ses dosyası başarıyla yüklendi\n");
    }
    
    int ret;
    uint8_t *buffer = NULL;
    int numBytes;
    
    // Video oynatıcıyı başlat
    if (!initVideoPlayer(renderer)) {
        return false;
    }
    
    // Video dosyasını aç
    ret = avformat_open_input(&pFormatCtx, videoPath, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Video dosyası açılamadı: %s\n", videoPath);
        return false;
    }
    
    // Stream bilgilerini al
    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Stream bilgileri alınamadı\n");
        avformat_close_input(&pFormatCtx);
        return false;
    }
    
    // Dosya bilgilerini yazdır
    av_dump_format(pFormatCtx, 0, videoPath, 0);
    
    // İlk video stream'ini bul
    videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            pVideoStream = pFormatCtx->streams[i];
            break;
        }
    }
    
    if (videoStream == -1) {
        fprintf(stderr, "Video stream bulunamadı\n");
        avformat_close_input(&pFormatCtx);
        return false;
    }
    
    // Video codec'ini bul
    const AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if (!pCodec) {
        fprintf(stderr, "Codec bulunamadı\n");
        avformat_close_input(&pFormatCtx);
        return false;
    }
    
    // Codec context oluştur
    pVideoCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pVideoCodecCtx) {
        fprintf(stderr, "Codec context oluşturulamadı\n");
        avformat_close_input(&pFormatCtx);
        return false;
    }
    
    // Codec parametrelerini kopyala
    ret = avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
    if (ret < 0) {
        fprintf(stderr, "Codec parametreleri kopyalanamadı\n");
        avcodec_free_context(&pVideoCodecCtx);
        avformat_close_input(&pFormatCtx);
        return false;
    }
    
    // Codec'i aç
    ret = avcodec_open2(pVideoCodecCtx, pCodec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Codec açılamadı\n");
        avcodec_free_context(&pVideoCodecCtx);
        avformat_close_input(&pFormatCtx);
        return false;
    }
    
    // YUV buffer oluştur
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pVideoCodecCtx->width, pVideoCodecCtx->height, 32);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) {
        fprintf(stderr, "Buffer oluşturulamadı\n");
        avcodec_free_context(&pVideoCodecCtx);
        avformat_close_input(&pFormatCtx);
        av_frame_free(&pFrame);
        av_frame_free(&pFrameYUV);
        return false;
    }
    
    // YUV frame'i ayarla
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, buffer, AV_PIX_FMT_YUV420P, pVideoCodecCtx->width, pVideoCodecCtx->height, 32);
    
    // Ölçeklendirme bağlamını oluştur
    sws_ctx = sws_getContext(
        pVideoCodecCtx->width, pVideoCodecCtx->height, pVideoCodecCtx->pix_fmt,
        pVideoCodecCtx->width, pVideoCodecCtx->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL
    );
    
    if (!sws_ctx) {
        fprintf(stderr, "Ölçeklendirme bağlamı oluşturulamadı\n");
        av_free(buffer);
        avcodec_free_context(&pVideoCodecCtx);
        avformat_close_input(&pFormatCtx);
        av_frame_free(&pFrame);
        av_frame_free(&pFrameYUV);
        return false;
    }
    
    // SDL texture oluştur
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        pVideoCodecCtx->width,
        pVideoCodecCtx->height
    );
    
    if (!texture) {
        fprintf(stderr, "SDL texture oluşturulamadı: %s\n", SDL_GetError());
        sws_freeContext(sws_ctx);
        av_free(buffer);
        avcodec_free_context(&pVideoCodecCtx);
        avformat_close_input(&pFormatCtx);
        av_frame_free(&pFrame);
        av_frame_free(&pFrameYUV);
        return false;
    }
    
    // Ses dosyasını oynat
    if (pSoundTrack) {
        Mix_PlayChannel(0, pSoundTrack, 0);
    }
    
    // Video oynatmayı başlat
    video_start_time = SDL_GetTicks();
    
    // Ana döngü
    int frameFinished;
    SDL_Event event;
    
    while (!quit) {
        // SDL olaylarını kontrol et
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            } else if (event.type == SDL_KEYDOWN) {
                // Herhangi bir tuşa basıldığında videoyu atla
                quit = true;
                break;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                // Herhangi bir fare tıklamasında videoyu atla
                quit = true;
                break;
            }
        }
        
        if (quit) {
            break;
        }
        
        // Paket oku
        if (av_read_frame(pFormatCtx, &packet) < 0) {
            // Dosya sonu
            break;
        }
        
        // Video paketi
        if (packet.stream_index == videoStream) {
            // Paketi gönder
            avcodec_send_packet(pVideoCodecCtx, &packet);
            
            // Frame al
            frameFinished = avcodec_receive_frame(pVideoCodecCtx, pFrame);
            
            if (frameFinished == 0) {
                // Frame'i YUV'a dönüştür
                sws_scale(
                    sws_ctx,
                    (uint8_t const * const *)pFrame->data,
                    pFrame->linesize,
                    0,
                    pVideoCodecCtx->height,
                    pFrameYUV->data,
                    pFrameYUV->linesize
                );
                
                // Video saatini güncelle
                if (packet.pts != AV_NOPTS_VALUE) {
                    video_clock = av_q2d(pVideoStream->time_base) * packet.pts;
                }
                
                // Texture'ı güncelle
                SDL_UpdateYUVTexture(
                    texture,
                    NULL,
                    pFrameYUV->data[0], pFrameYUV->linesize[0],
                    pFrameYUV->data[1], pFrameYUV->linesize[1],
                    pFrameYUV->data[2], pFrameYUV->linesize[2]
                );
                
                // Ekranı temizle
                SDL_RenderClear(renderer);
                
                // Texture'ı çiz
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                
                // Ekranı güncelle
                SDL_RenderPresent(renderer);
                
                // Kare hızını kontrol et
                double frame_delay = av_q2d(pVideoStream->time_base) * pVideoStream->avg_frame_rate.den / pVideoStream->avg_frame_rate.num;
                double current_time = (double)(SDL_GetTicks() - video_start_time) / 1000.0;
                double diff = video_clock - current_time;
                
                if (diff > 0) {
                    // Gecikme ekle
                    SDL_Delay((Uint32)(diff * 1000.0));
                }
            }
        }
        
        // Paketi serbest bırak
        av_packet_unref(&packet);
    }
    
    // Temizle
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    
    if (pSoundTrack) {
        Mix_HaltChannel(0);
        Mix_FreeChunk(pSoundTrack);
        pSoundTrack = NULL;
    }
    
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = NULL;
    }
    
    if (buffer) {
        av_free(buffer);
        buffer = NULL;
    }
    
    if (pFrame) {
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    
    if (pFrameYUV) {
        av_frame_free(&pFrameYUV);
        pFrameYUV = NULL;
    }
    
    if (pVideoCodecCtx) {
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
    }
    
    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }
    
    return true;
}

// Kaynakları temizle
void cleanupVideoPlayer() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    
    if (pFrame) {
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    
    if (pFrameYUV) {
        av_frame_free(&pFrameYUV);
        pFrameYUV = NULL;
    }
    
    if (pVideoCodecCtx) {
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
    }
    
    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }
    
    if (pSoundTrack) {
        Mix_FreeChunk(pSoundTrack);
        pSoundTrack = NULL;
    }
}

// Intro videosunu oynat
bool playIntroVideo(SDL_Renderer *renderer) {
    printf("Intro videosu oynatılıyor...\n");
    
    // Ekranı siyaha boyayarak başla
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    // Video dosyasının yolunu oluştur
    const char *videoPath = "resources/intro.mov";
    
    // Videoyu oynat
    bool result = playVideo(renderer, videoPath);
    
    // Video bittikten sonra ekranı tekrar temizle
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    return result;
}
