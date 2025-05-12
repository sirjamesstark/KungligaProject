#ifndef MENU_H
#define MENU_H

#include "../include/theme.h"
#include "../include/common.h"

typedef struct menu Menu;

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
void destroyMenu(Menu *pMenu);
bool runMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, char IPinput[15]) ;
void handleKey(SDL_Event *pEvent, Menu *pMenu, bool *pMenuRunning, bool *pStartGame, bool *pRunLobby);
void handleMouse(SDL_Event *pEvent, Menu *pMenu, bool *pMenuRunning, bool *pStartGame, bool *pRunLobby);
void handleEnterOrMouseClick(Menu *pMenu, bool *pMenuRunning, bool *pStartGame, bool *pRunLobby);

#endif