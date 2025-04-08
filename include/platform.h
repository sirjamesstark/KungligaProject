#ifndef asteroid_h
#define asteroid_h

typedef struct blockImage BlockImage;
typedef struct blocks Blocks;

BlockImage *createBlockImage(SDL_Renderer *pRenderer);
Blocks *createBlock(BlockImage *pAsteroidImage, int window_width, int window_height);
void updateAsteroid(Blocks *pAsteroid);
void drawBlock(Blocks *pAsteroid);
void destroyBlock(Blocks *pAsteroid);
SDL_Rect getRectBlock(Blocks *pAsteroid);
void destroyBlockImage(BlockImage *pAsteroidImage);

#endif