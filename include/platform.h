#ifndef platform_h
#define platform_h

#define BOX_ROW 220
#define BOX_COL 26
#define BOX_SCREEN_Y 22
#define BLOCK_SCALEFACTOR 0.04f

typedef struct block Block;

Block *createBlock(SDL_Renderer *pRenderer, SDL_Rect *pGameAreaRect);
void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY);
void drawBlock(Block *pBlock, int block_type, SDL_Rect *dstRect);
void destroyBlock(Block *pBlock);
SDL_Rect getBlockRect(Block *pBlock);

#endif