#ifndef theme_h
#define theme_h

#include "../include/common.h"

typedef struct background Background;
typedef struct button Button;
typedef struct audio Audio;

Background *createBackground(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type);
void drawBackground(Background *pBackground, int CamX, int CamY);
void destroyBackground(Background *pBackground);
Button *createButton(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, ButtonType button_type);
int setButtonPlacement(Button *pButton, ButtonType button_type);
void drawButton(Button *pButton);
void makeButtonHoverd(Button *pButton);
void makeButtonNotHovered(Button *pButton);
void toggleHoveredButton(Button *pButton);
bool isButtonHovered(Button *pButton);
bool isMouseOverButton(int x, int y, Button *pButton);
void destroyButton(Button *pButton);
Audio *createAudio(State theme_type);
void playMusic(Audio *pAudio);
void playButtonSound(Audio *pAudio);
void playJumpSound(Audio *pAudio);
void playDeathSound(Audio *pAudio);
void toggleMuteAudio(Audio *pAudio);
bool isMusicMuted(Audio *pAudio);
void destroyAudio(Audio *pAudio);
SDL_Cursor *initCursor();
void destroyCursor(SDL_Cursor *pCursor);

#endif