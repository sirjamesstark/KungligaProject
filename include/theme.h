#ifndef theme_h
#define theme_h

#include "../include/scaling.h"

#define LAVA_SCALEFACTOR 0.45f

typedef struct background Background;
typedef struct lava Lava;
typedef struct button Button;
typedef struct cursor Cursor;
typedef struct audio Audio;

Background *createBackground(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type);
void drawBackground(Background *pBackground);
void destroyBackground(Background *pBackground);

Lava *createLava(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
void updateLavaFrame(Lava *pLava);
void drawLava(Lava *pLava);
void destroyLava(Lava *pLava);

Button *createButton(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, ButtonType button_type);
int setButtonPlacement(Button *pButton, ButtonType button_type);
void makeButtonHoverd(Button *pButton);
void makeButtonNotHovered(Button *pButton);
void toggleHoveredButton(Button *pButton);
bool isButtonPushed(Button *pButton);
void checkMouseOverButton(Button *pButton, Cursor *pCursor);
void drawButton(Button *pButton);
void destroyButton(Button *pButton);

Cursor *createCursor();
void updateCursorPosition(Cursor *pCursor);
void toggleCursorVisibility(Cursor *pCursor);
void destroyCursor(Cursor *pCursor);

Audio *createAudio(State theme_type);
void playMusic(Audio *pAudio);
void playButtonSound(Audio *pAudio);
void playJumpSound(Audio *pAudio);
void playDeathSound(Audio *pAudio);
void playWinningSound(Audio *pAudio);
void toggleMuteAudio(Audio *pAudio);
bool isMusicMuted(Audio *pAudio);
void destroyAudio(Audio *pAudio);

#endif