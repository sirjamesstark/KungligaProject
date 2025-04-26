#ifndef platform_h
#define platform_h

#define BOX_ROW 22
#define BOX_COL 26
#define BLOCK_SCALEFACTOR 0.045f

//typedef struct blockImage BlockImage;
typedef struct block Block;

//BlockImage *createBlockImage(SDL_Renderer *pRenderer);
Block *createBlock(SDL_Renderer *pRenderer, int window_width, int window_height);
void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY);
void drawBlock(Block *pBlock, int block_type);
void destroyBlock(Block *pBlock);
SDL_Rect getRectBlock(Block *pBlock);
//void destroyBlockImage(BlockImage *pBlockImage);

#endif