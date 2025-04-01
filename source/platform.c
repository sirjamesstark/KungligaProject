#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "../include/platform.h"

struct asteroidImage{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;    
};

struct asteroid{
    float x, y;
    int window_width,window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
};

static void getStartValues(Asteroid *a);

AsteroidImage *createAsteroidImage(SDL_Renderer *pRenderer){
    static AsteroidImage* pAsteroidImage = NULL;
    if(pAsteroidImage==NULL){
        pAsteroidImage = malloc(sizeof(struct asteroidImage));
        SDL_Surface *surface = IMG_Load("resources/block.png");
        if(!surface){
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
        pAsteroidImage->pRenderer = pRenderer;
        pAsteroidImage->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
        SDL_FreeSurface(surface);
        if(!pAsteroidImage->pTexture){
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
    }
    return pAsteroidImage;
}

Asteroid *createAsteroid(AsteroidImage *pAsteroidImage, int window_width, int window_height){
    Asteroid *pAsteroid = malloc(sizeof(struct asteroid));
    pAsteroid->pRenderer = pAsteroidImage->pRenderer;
    pAsteroid->pTexture = pAsteroidImage->pTexture;
    pAsteroid->window_width = window_width;
    pAsteroid->window_height = window_height;
    SDL_QueryTexture(pAsteroidImage->pTexture,NULL,NULL,&(pAsteroid->rect.w),&(pAsteroid->rect.h));
    pAsteroid->rect.w=16;
    pAsteroid->rect.h=16;

    return pAsteroid;
}

SDL_Rect getRectAsteroid(Asteroid *pAsteroid){
    return pAsteroid->rect;
}

void drawAsteroid(Asteroid *pAsteroid){
    SDL_RenderCopyEx(pAsteroid->pRenderer,pAsteroid->pTexture,NULL,&(pAsteroid->rect),0,NULL,SDL_FLIP_NONE);
}

void destroyAsteroid(Asteroid *pAsteroid){
    free(pAsteroid);
}

void destroyAsteroidImage(AsteroidImage *pAsteroidImage){
    SDL_DestroyTexture(pAsteroidImage->pTexture);
}
