#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <SDL.h>
#include <stdbool.h>

// FFmpeg libraries
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>

// Error handling macro for FFmpeg functions
#define FFMPEG_ERROR(msg) {\
    char error_buf[AV_ERROR_MAX_STRING_SIZE];\
    av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);\
    fprintf(stderr, "%s: %s\n", msg, error_buf);\
    return false;\
}

// Initialize the video player
bool initVideoPlayer(SDL_Renderer *renderer);

// Play the specified video file
bool playVideo(SDL_Renderer *renderer, const char *videoPath);

// Clean up video player resources
void cleanupVideoPlayer();

#endif // VIDEO_PLAYER_H
