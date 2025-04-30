#include "../include/common.h"
#include <SDL.h>

SDL_Rect scaleAndCenterRect(const SDL_Rect srcRect, const SDL_Rect screenRect, float scaleFactor) {
    SDL_Rect dstRect;
    float scale = fmin((float)screenRect.w / (float)srcRect.w, (float)screenRect.h / (float)srcRect.h) * scaleFactor;
    
    dstRect.w = (int)(srcRect.w * scale + 0.5f); 
    dstRect.h = (int)(srcRect.h * scale + 0.5f);
    dstRect.x = (int)((float)((screenRect.x * 2 + screenRect.w - dstRect.w) / 2) + 0.5f);
    dstRect.y = (int)((float)((screenRect.y * 2 + screenRect.h - dstRect.h) / 2) + 0.5f);
    printf("Rect size (before scaling): w: %d, h: %d\n", srcRect.w, srcRect.h);
    printf("Rect size (after scaling): w: %d, h: %d\n", dstRect.w, dstRect.h);

    return dstRect; 
}