#ifndef asteroid_h
#define asteroid_h

#define BOX_ROW 22
#define BOX_COL 26

typedef struct blockImage BlockImage;
typedef struct blocks Block;

BlockImage *createBlockImage(SDL_Renderer *pRenderer);
Block *createBlock(BlockImage *pBlockImage, int window_width, int window_height);
void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY);
void drawBlock(Block *pBlock);
void destroyBlock(Block *pBlock);
SDL_Rect getRectBlock(Block *pBlock);
void destroyBlockImage(BlockImage *pBlockImage);

#endif