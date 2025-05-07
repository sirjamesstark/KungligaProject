#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "../include/video_player.h"
#include "../include/common.h"

// Only include FFmpeg code if USE_FFMPEG is defined
#ifdef USE_FFMPEG

// Paths may be different on Windows, so we try both paths
#ifdef _WIN32
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#else
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#endif

// FFmpeg video player state
static AVFormatContext *pFormatCtx = NULL;
static AVCodecContext *pVideoCodecCtx = NULL;
static AVCodecContext *pAudioCodecCtx = NULL;
static AVStream *pVideoStream = NULL;
static AVStream *pAudioStream = NULL;
static const AVCodec *pVideoCodec = NULL;
static const AVCodec *pAudioCodec = NULL;
static AVFrame *pFrame = NULL;
static AVFrame *pFrameYUV = NULL;
static AVPacket packet;
static struct SwsContext *sws_ctx = NULL;
static struct SwrContext *swr_ctx = NULL;

// SDL_mixer sound
static Mix_Chunk *pSoundTrack = NULL;
static SDL_Texture *texture = NULL;
static int videoStream = -1;
static int audioStream = -1;
static bool quit = false;
static Uint32 video_start_time = 0;
static double video_clock = 0.0;

// Function declarations
bool initVideoPlayer(SDL_Renderer *renderer);
bool playVideo(SDL_Renderer *renderer, const char *videoPath);
void cleanupVideoPlayer();

bool initVideoPlayer(SDL_Renderer *renderer) {
    // Initialize FFmpeg state
    pFormatCtx = NULL;
    pVideoCodecCtx = NULL;
    pAudioCodecCtx = NULL;
    pVideoStream = NULL;
    pAudioStream = NULL;
    pVideoCodec = NULL;
    pAudioCodec = NULL;
    pFrame = NULL;
    pFrameYUV = NULL;
    sws_ctx = NULL;
    swr_ctx = NULL;
    texture = NULL;
    
    // Allocate video frame
    pFrame = av_frame_alloc();
    if (!pFrame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return false;
    }
    
    // Allocate YUV frame
    pFrameYUV = av_frame_alloc();
    if (!pFrameYUV) {
        fprintf(stderr, "Could not allocate YUV frame\n");
        return false;
    }
    
    videoStream = -1;
    audioStream = -1;
    quit = false;
    video_clock = 0.0;
    
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
    uint8_t *buffer = NULL;
    int numBytes;
    
    // Initialize the video player
    if (!initVideoPlayer(renderer)) {
        return false;
    }
    
    // Open the video file
    ret = avformat_open_input(&pFormatCtx, videoPath, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open video file\n");
        return false;
    }
    
    // Retrieve stream information
    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return false;
    }
    
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, videoPath, 0);
    
    // Find the first video stream
    videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    
    if (videoStream == -1) {
        fprintf(stderr, "Could not find video stream\n");
        return false;
    }
    
    // Get a pointer to the codec context for the video stream
    pVideoCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if (!pVideoCodec) {
        fprintf(stderr, "Unsupported codec\n");
        return false;
    }
    
    // Create codec context
    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
    if (!pVideoCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }
    
    // Copy codec parameters from input stream to output codec context
    ret = avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
    if (ret < 0) {
        fprintf(stderr, "Failed to copy codec parameters to codec context\n");
        return false;
    }
    
    // Open codec
    ret = avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec\n");
        return false;
    }
    
    // Save the video stream
    pVideoStream = pFormatCtx->streams[videoStream];
    
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
        }
        
        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }
    
    // Wait a moment for the sound to finish
    if (pSoundTrack) {
        SDL_Delay(500);  // Give half a second for audio to finish
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
    // Free the sound resource
    if (pSoundTrack) {
        Mix_HaltChannel(0);  // Stop the sound on channel 0
        Mix_FreeChunk(pSoundTrack);
        pSoundTrack = NULL;
        printf("Sound resource freed\n");
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
    video_clock = 0.0;
}

#else

// Simplified versions of the video player functions when FFmpeg is not available

bool initVideoPlayer(SDL_Renderer *renderer) {
    // No initialization needed when FFmpeg is not available
    return true;
}

bool playVideo(SDL_Renderer *renderer, const char *videoPath) {
    printf("\n*** FALLBACK MODE: FFmpeg not available ***\n");
    printf("Attempting to play intro sound only...\n");
    
    // Try to load the sound file
    Mix_Chunk *pSoundTrack = Mix_LoadWAV("resources/KungligaProjectSound.wav");
    
    // If the first attempt fails, try an alternative path
    if (!pSoundTrack) {
        printf("Could not find sound file in resources directory, trying alternative path...\n");
        pSoundTrack = Mix_LoadWAV("../resources/KungligaProjectSound.wav");
    }
    
    // If we found the sound file, play it
    if (pSoundTrack) {
        printf("Sound file found, playing audio...\n");
        Mix_PlayChannel(0, pSoundTrack, 0);
        
        // Display a black screen while the sound plays
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        
        // Wait for a few seconds to let the sound play
        SDL_Delay(3000);
        
        // Clean up
        Mix_FreeChunk(pSoundTrack);
    } else {
        fprintf(stderr, "ERROR: Could not load sound file: %s\n", Mix_GetError());
        
        // Just show a black screen for a moment
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
    }
    
    printf("Sound playback started\n");
    
    // Display a black screen while the sound plays
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    // Wait for the sound to finish (or at least a few seconds)
    SDL_Delay(5000); // Wait for 5 seconds to ensure sound plays
    
    // Clean up
    Mix_HaltChannel(0);
    printf("Sound playback stopped\n");
    
    return true;
}

void cleanupVideoPlayer() {
    // No cleanup needed when FFmpeg is not available
}

#endif // USE_FFMPEG
