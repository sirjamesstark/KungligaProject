#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "../include/platform.h"

struct blockImage{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;    
};

struct blocks{
    float x, y;
    int window_width,window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
};

static void getStartValues(Blocks *a);

BlockImage *createBlockImage(SDL_Renderer *pRenderer){
    static BlockImage* pBlockImage = NULL;
    if(pBlockImage==NULL){
        pBlockImage = malloc(sizeof(struct blockImage));
        SDL_Surface *surface = IMG_Load("resources/boxPaint.png");
        if(!surface){
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
        pBlockImage->pRenderer = pRenderer;
        pBlockImage->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
        SDL_FreeSurface(surface);
        if(!pBlockImage->pTexture){
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
    }
    return pBlockImage;
}

Blocks *createBlock(BlockImage *pBlockImage, int window_width, int window_height){
    Blocks *pBlock = malloc(sizeof(struct blocks));
    pBlock->pRenderer = pBlockImage->pRenderer;
    pBlock->pTexture = pBlockImage->pTexture;
    pBlock->window_width = window_width;
    pBlock->window_height = window_height;
    SDL_QueryTexture(pBlockImage->pTexture,NULL,NULL,&(pBlock->rect.w),&(pBlock->rect.h));
    pBlock->rect.w=16;
    pBlock->rect.h=16;

    return pBlock;
}

SDL_Rect getRectBlock(Blocks *pBlock){
    return pBlock->rect;
}

void drawBlock(Blocks *pBlock){
    SDL_RenderCopyEx(pBlock->pRenderer,pBlock->pTexture,NULL,&(pBlock->rect),0,NULL,SDL_FLIP_NONE);
}

void destroyBlock(Blocks *pBlock){
    free(pBlock);
}

void destroyBlockImage(BlockImage *pBlockImage){
    SDL_DestroyTexture(pBlockImage->pTexture);
}
