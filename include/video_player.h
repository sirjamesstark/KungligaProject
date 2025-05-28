#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <SDL.h>
#include <stdbool.h>

// FFmpeg ile video oynatma için fonksiyon tanımlamaları
// Video oynatıcıyı başlatır
bool initVideoPlayer(SDL_Renderer *renderer);

// Belirtilen video dosyasını oynatır
bool playVideo(SDL_Renderer *renderer, const char *videoPath);

// Video oynatıcı kaynaklarını temizler
void cleanupVideoPlayer();

// Intro videosunu oynatır (resources/intro.mov)
bool playIntroVideo(SDL_Renderer *renderer);

#endif /* VIDEO_PLAYER_H */
