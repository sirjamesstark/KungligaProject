#ifndef MENU_H
#define MENU_H

#define NROFBUTTONS 3
#define BOTTON_SCALEFACTOR 0.35f

#include "../include/theme.h"
#include "../include/common.h"

typedef enum buttons Buttons;
typedef struct button Button;
typedef struct menu Menu;

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
Button *createButton(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, Buttons button_type);
void destroyMenu(Menu *pMenu);
void destroyButton(Button *pButton);
void drawButton(Button *pButton);
bool runMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
void handleKeyDown(SDL_Event *pEvent, Menu *pMenu);
void handleMouseMotion(SDL_Event *pEvent, Menu *pMenu);
void handleMouseButtonDown(SDL_Event *pEvent, Menu *pMenu);
bool isMouseOverButton(int x, int y, Button *pButton);

#endif