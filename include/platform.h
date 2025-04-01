#ifndef asteroid_h
#define asteroid_h

typedef struct platformImage PlatformImage;
typedef struct platform Platform;

PlatformImage *createAsteroidImage(SDL_Renderer *pRenderer);
Platform *createPlatform(PlatformImage *pAsteroidImage, int window_width, int window_height);
void updateAsteroid(Platform *pAsteroid);
void drawPlatform(Platform *pAsteroid);
void destroyPlatform(Platform *pAsteroid);
SDL_Rect getRectPlatform(Platform *pAsteroid);
void destroyPlatformImage(PlatformImage *pAsteroidImage);

#endif