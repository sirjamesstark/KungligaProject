#ifndef COMMON_H
#define COMMON_H
#include <SDL.h>

#define BUTTON_SCALEFACTOR 0.35f
#define NROFBUTTONSMENU 4

enum state {
    MENU =  0,
    GAME =  1
};
typedef enum state State;

enum buttontype {
    START    = 0,
    EXIT     = 1,
    SOUND    = 2,
    JOIN     = 3,
    BACK     = 4
};
typedef enum buttontype ButtonType;

SDL_Rect scaleAndCenterRect(const SDL_Rect srcRect, const SDL_Rect screenRect, float scaleFactor);
SDL_Rect stretchRectToScreen(const SDL_Rect screenRect);
void drawPadding(SDL_Renderer *pRenderer, const SDL_Rect screenRect);

#endif
