#ifndef platform_h
#define platform_h

#include "../include/common.h"

#define BOX_ROW 140
#define BOX_COL 26
#define BOX_SCREEN_Y 14
#define BLOCK_SCALEFACTOR 0.072f

typedef struct block Block;

float getShiftLength(Block *pBlock);
Block *createBlock(SDL_Renderer *pRenderer, SDL_Rect *pGameAreaRect);
void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY,float shiftLength);
void drawBlock(Block *pBlock, int block_type);
void destroyBlock(Block *pBlock);
SDL_Rect getBlockRect(Block *pBlock);

#endif