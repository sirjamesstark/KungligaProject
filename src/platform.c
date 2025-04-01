#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "../include/platform.h"

struct platformImage{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;    
};

struct platform{
    float x, y;
    int window_width,window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
};

static void getStartValues(Platform *a);

PlatformImage *createPlatformImage(SDL_Renderer *pRenderer){
    static PlatformImage* pPlatformImage = NULL;
    if(pPlatformImage==NULL){
        pPlatformImage = malloc(sizeof(struct platformImage));
        SDL_Surface *surface = IMG_Load("resources/block.png");
        if(!surface){
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
        pPlatformImage->pRenderer = pRenderer;
        pPlatformImage->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
        SDL_FreeSurface(surface);
        if(!pPlatformImage->pTexture){
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
    }
    return pPlatformImage;
}

Platform *createPlatform(PlatformImage *pPlatformImage, int window_width, int window_height){
    Platform *pPlatform = malloc(sizeof(struct platform));
    pPlatform->pRenderer = pPlatformImage->pRenderer;
    pPlatform->pTexture = pPlatformImage->pTexture;
    pPlatform->window_width = window_width;
    pPlatform->window_height = window_height;
    SDL_QueryTexture(pPlatformImage->pTexture,NULL,NULL,&(pPlatform->rect.w),&(pPlatform->rect.h));
    pPlatform->rect.w=16;
    pPlatform->rect.h=16;

    return pPlatform;
}

SDL_Rect getRectPlatform(Platform *pPlatform){
    return pPlatform->rect;
}

void drawPlatform(Platform *pPlatform){
    SDL_RenderCopyEx(pPlatform->pRenderer,pPlatform->pTexture,NULL,&(pPlatform->rect),0,NULL,SDL_FLIP_NONE);
}

void destroyPlatform(Platform *pPlatform){
    free(pPlatform);
}

void destroyPlatformImage(PlatformImage *pPlatformImage){
    SDL_DestroyTexture(pPlatformImage->pTexture);
}
