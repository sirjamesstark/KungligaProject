#include "../include/scaling.h"
#include <SDL.h>

SDL_Rect getScreenRect(SDL_Window *pWindow) {
    SDL_Rect screenRect;
    int window_width, window_height;

    SDL_GetWindowSize(pWindow, &window_width, &window_height);
    float currentAspect = (float)window_width / window_height;
    int targetWidth = window_width;
    int targetHeight = window_height;

    if (currentAspect > TARGET_ASPECT_RATIO) {
        targetWidth = (int)(window_height * TARGET_ASPECT_RATIO + 0.5f);
    }
    else if (currentAspect < TARGET_ASPECT_RATIO) {
        targetHeight = (int)(window_width / TARGET_ASPECT_RATIO + 0.5f);
    }

    screenRect.w = (int)(targetWidth + 0.5f);
    screenRect.h = (int)(targetHeight + 0.5f);
    screenRect.x = (int)((window_width - targetWidth) / 2 + 0.5f);
    screenRect.y = (int)((window_height - targetHeight) / 2 + 0.5f);

    printf("Original window size: w: %d, h: %d \n", window_width, window_height);
    printf("screenRect: x=%d, y=%d, w=%d, h=%d\n", screenRect.x, screenRect.y, screenRect.w, screenRect.h);

    return screenRect; 
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

    // We're not using the bottom padding anymore as requested by the user
    SDL_Rect bottomPadding = {
         .x = 0,
         .y = screenRect.y + screenRect.h,
         .w = screenRect.x * 2 + screenRect.w,
         .h = screenRect.y + 10
    };

    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(pRenderer, &leftPadding);
    SDL_RenderFillRect(pRenderer, &rightPadding);
    SDL_RenderFillRect(pRenderer, &topPadding);
    // Not drawing the bottom padding anymore
    SDL_RenderFillRect(pRenderer, &bottomPadding);
}

void stretchRectToFitTarget(SDL_Rect *pDstRect, const SDL_Rect targetRect) {
    pDstRect->x = targetRect.x;
    pDstRect->y = targetRect.y;
    pDstRect->w = targetRect.w;
    pDstRect->h = targetRect.h;
    printf("Rect size (stretched version): w: %d, h: %d\n", pDstRect->w, pDstRect->h);
}

SDL_Rect scaleRect(const SDL_Rect srcRect, const SDL_Rect screenRect, float scaleFactor) {
    SDL_Rect dstRect;
    float scale = fmin((float)screenRect.w / (float)srcRect.w, (float)screenRect.h / (float)srcRect.h) * scaleFactor;

    dstRect.w = (int)(srcRect.w * scale + 0.5f); 
    dstRect.h = (int)(srcRect.h * scale + 0.5f);
    dstRect.x = 0;
    dstRect.y = 0;
    
    printf("Rect size (before scaling): w: %d, h: %d\n", srcRect.w, srcRect.h);
    printf("Rect size (after scaling): w: %d, h: %d\n", dstRect.w, dstRect.h);

    return dstRect; 
}

void centerRectOnTargetRect(SDL_Rect *pDstRect, const SDL_Rect targetRect) {
    pDstRect->x = targetRect.x + (targetRect.w - pDstRect->w) / 2;
    pDstRect->y = targetRect.y + (targetRect.h - pDstRect->h) / 2;
}