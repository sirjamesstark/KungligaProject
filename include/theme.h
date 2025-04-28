#ifndef theme_h
#define theme_h

typedef struct background Background;

Mix_Music *initiateMusic();
void playMusic(Mix_Music *pGameMusic);
Background *createBackground(SDL_Renderer *pRenderer, int window_width, int window_height);
void drawBackground(Background *pBackground, int CamX, int CamY);
void destroyBackground(Background *pBackground);
#endif