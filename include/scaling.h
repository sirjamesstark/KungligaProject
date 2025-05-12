#ifndef SCALING_H
#define SCALING_H
#include <SDL.h>

#define TARGET_ASPECT_RATIO (16.0f / 9.0f)
#define BUTTON_SCALEFACTOR 0.35f
#define NROFBUTTONS 3

enum state {
    MENU =  0,
    GAME =  1
};
typedef enum state State;

enum buttontype {
    START   = 0,
    EXIT    = 1,
    SOUND   = 2,
    JOIN    = 3,
    BACK    = 4
};
typedef enum buttontype ButtonType;

SDL_Rect getScreenRect(SDL_Window *pWindow);
void drawPadding(SDL_Renderer *pRenderer, const SDL_Rect screenRect);
void stretchRectToFitTarget(SDL_Rect *pDstRect, const SDL_Rect targetRect);
SDL_Rect scaleRect(const SDL_Rect srcRect, const SDL_Rect screenRect, float scaleFactor);
void centerRectOnTargetRect(SDL_Rect *pDstRect, const SDL_Rect targetRect);

#endif
