#ifndef COMMON_H
#define COMMON_H
#include <SDL.h>

typedef enum state State;

enum state {
    MENU =  0,
    GAME =  1
};

SDL_Rect scaleAndCenterRect(const SDL_Rect srcRect, const SDL_Rect screenRect, float scaleFactor);
SDL_Rect stretchRectToScreen(const SDL_Rect screenRect);
void drawPadding(SDL_Renderer *pRenderer, const SDL_Rect screenRect);

#endif
