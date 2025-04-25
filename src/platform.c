#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "../include/platform.h"

struct blockImage
{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
};

struct blocks
{
    float x, y;
    int window_width, window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
};

BlockImage *createBlockImage(SDL_Renderer *pRenderer)
{
    BlockImage *pBlockImage = NULL;
    if (pBlockImage == NULL)
    {
        pBlockImage = malloc(sizeof(struct blockImage));
        if (pBlockImage == NULL)
            return NULL;
        SDL_Surface *surface = IMG_Load("resources/box8.png");
        if (!surface)
        {
            printf("Error: %s\n", SDL_GetError());
            return NULL;
        }
        pBlockImage->pRenderer = pRenderer;
        pBlockImage->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
        SDL_FreeSurface(surface);
        if (!pBlockImage->pTexture)
        {
            printf("Error: %s\n", SDL_GetError());
            return NULL;
        }
    }
    return pBlockImage;
}

Block *createBlock(BlockImage *pBlockImage, int window_width, int window_height)
{
    Block *pBlock = malloc(sizeof(struct blocks));
    if (pBlock == NULL)
        return NULL;
    pBlock->pRenderer = pBlockImage->pRenderer;
    pBlock->pTexture = pBlockImage->pTexture;
    pBlock->window_width = window_width;
    pBlock->window_height = window_height;
    SDL_QueryTexture(pBlockImage->pTexture, NULL, NULL, &(pBlock->rect.w), &(pBlock->rect.h));
    pBlock->rect.w = window_width / BOX_COL;
    pBlock->rect.h = window_height / BOX_ROW;

    return pBlock;
}

SDL_Rect getRectBlock(Block *pBlock)
{
    return pBlock->rect;
}

void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamX, int CamY)
{
    for (int row = 0; row < BOX_ROW; row++)
    {
        for (int col = 0; col < BOX_COL; col++)
        {
            if (gameMap[row][col] == 1)
            {
                pBlock->rect.x = col * pBlock->rect.w;
                pBlock->rect.y = row * pBlock->rect.h;
                pBlock->rect.y -= CamY;
                drawBlock(pBlock);
                pBlock->rect.y += CamY;
            }
        }
    }
}

void drawBlock(Block *pBlock)
{
    SDL_RenderCopy(pBlock->pRenderer, pBlock->pTexture, NULL, &(pBlock->rect));
}

void destroyBlock(Block *pBlock)
{
    free(pBlock);
}

void destroyBlockImage(BlockImage *pBlockImage)
{
    SDL_DestroyTexture(pBlockImage->pTexture);
}
