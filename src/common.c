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

SDL_Rect stretchRectToScreen(const SDL_Rect screenRect) {
    SDL_Rect dstRect;
    
    dstRect.w = screenRect.w;
    dstRect.h = screenRect.h;
    dstRect.x = screenRect.x;
    dstRect.y = screenRect.y;
    printf("Rect size (stretched version): w: %d, h: %d\n", dstRect.w, dstRect.h);
    printf("Rect position: x: %d, y: %d\n", dstRect.x, dstRect.y);

    return dstRect; 
}

void drawPadding(SDL_Renderer *pRenderer, const SDL_Rect screenRect) {
    SDL_Rect leftPadding = {
        .x = 0,
        .y = screenRect.y,
        .w = screenRect.x,
        .h = screenRect.h
    };

    SDL_Rect rightPadding = {
        .x = screenRect.x + screenRect.w,
        .y = screenRect.y,
        .w = screenRect.x,
        .h = screenRect.h
    };

    SDL_Rect topPadding = {
        .x = 0,
        .y = 0,
        .w = screenRect.x * 2 + screenRect.w,
        .h = screenRect.y
    };

    SDL_Rect bottomPadding = {
        .x = 0,
        .y = screenRect.y + screenRect.h,
        .w = screenRect.x * 2 + screenRect.w,
        .h = screenRect.y
    };

    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(pRenderer, &leftPadding);
    SDL_RenderFillRect(pRenderer, &rightPadding);
    SDL_RenderFillRect(pRenderer, &topPadding);
    SDL_RenderFillRect(pRenderer, &bottomPadding);
}