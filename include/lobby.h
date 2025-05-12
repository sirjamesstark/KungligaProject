#ifndef LOBBY_H
#define LOBBY_H
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <string.h>
#include <stdbool.h>

int runningLobby(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect,bool *pMenuRunning,
                bool *pStartGame, Audio *pAudio, Background *pBackground, char pIPinput[]) ;
void handleMouseInLobby(SDL_Event *pEvent, Button *pButtons[], bool *pMenuRunning, bool *pStartGame, Audio *pAudio, bool *pLobbyRunning);
void handleMouseClickInLobby(Button *pButtons[], bool *pMenuRunning, bool *pStartGame, Audio *pAudio, bool *pLobbyRunning);
SDL_Rect createInputBox(char inputText[],TTF_Font *pFont, SDL_Renderer *pRenderer, SDL_Rect screenRect);
void drawInputBox(SDL_Renderer* pRenderer, TTF_Font* pFont, char* inputText, SDL_Rect* box);
void cleanUpLobby(Button *pLobbyButtons[], TTF_Font *pFont, SDL_Rect *pTextBox);

#endif