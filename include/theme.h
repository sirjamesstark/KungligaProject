#ifndef theme_h
#define theme_h

#include "../include/common.h"

#define BACKGROUND_SCALEFACTOR 1.0f

typedef struct background Background;
typedef struct cursor Cursor;
typedef struct theme Theme;

Theme *createTheme(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type);
Background *createBackground(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type);
Mix_Music *initMusic(State theme_type);
void drawBackground(Theme *pTheme, int CamX, int CamY);
void playMusic(Theme *pTheme);
void destroyBackground(Background *pBackground);
void destroyTheme(Theme *pTheme);

#endif