#ifndef MENU_H
#define MENU_H

#include "../include/theme.h"
#include "../include/scaling.h"

#define NROFBUTTONS_MENU 3
#define OFFSET_MENU 0

typedef struct menu Menu;

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
void destroyMenu(Menu *pMenu);
bool runMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
void handleKey(SDL_Event *pEvent, Menu *pMenu, bool *pMenuRunning, bool *pStartGame);
void handleMouse(SDL_Event *pEvent, Menu *pMenu, bool *pMenuRunning, bool *pStartGame);
void handlePushedButton(Menu *pMenu, bool *pMenuRunning, bool *pStartGame);

#endif