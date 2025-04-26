#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "../include/platform.h"

/*
struct blockImage
{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
};
*/

struct block
{
    int nrOfFrames_blocks;
    SDL_Renderer *pRenderer;
    SDL_Rect *pGameAreaRect;
    SDL_Texture *pTexture;
    SDL_Rect srcRect;   // srcRect.w och srcRect.h lagrar den verkliga storleken av en frame i spritesheetet, srcRect.x och srcRect.y anger vilken frame i spritesheetet för blocks som väljs
    SDL_Rect dstRect;   // dstRect.w och dstRect.h är en nerskalad variant av srcRect.w och srcRect.h, dstRect.x och dstRect.y anger var i fönstret som den aktuella framen i srcRect.x och srcRect.y ska ritas upp
};

/*
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
*/

Block *createBlock(SDL_Renderer *pRenderer, SDL_Rect *pGameAreaRect)
{
    if (!pRenderer || !pGameAreaRect) {
        printf("Error: Invalid parameters. Renderer or GameAreaRect is NULL.\n");
        return NULL;
    }

    Block *pBlock = malloc(sizeof(struct block));
    if (pBlock == NULL) {
        printf("Error: Failed to allocate memory for block.\n");
        return NULL;
    }

    SDL_Surface *pSurface = IMG_Load("resources/blocks_spritesheet_test.png");
    if (!pSurface) {
        destroyBlock(pBlock);
        printf("Error: Failed to initialize block frames.\n");
        return NULL;
    }

    pBlock->pRenderer = pRenderer;
    pBlock->pGameAreaRect = pGameAreaRect;
    pBlock->nrOfFrames_blocks = 3;


    pBlock->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pBlock->pTexture) {
        destroyBlock(pBlock);
        printf("Error: Failed to create block texture.\n");
        return NULL;
    }

    SDL_QueryTexture(pBlock->pTexture, NULL, NULL, &(pBlock->srcRect.w), &(pBlock->srcRect.h));
    pBlock->srcRect.w /=3;

    /*
    printf("Block frame size (before scaling): w: %d, h: %d\n", pBlock->srcRect.w, pBlock->srcRect.h);
    float scaleFactor = (float)pBlock->pGameAreaRect->w / (float)pBlock->srcRect.w * BLOCK_SCALEFACTOR;
    pBlock->dstRect.w = (int)(pBlock->srcRect.w * scaleFactor + 0.5f);    
    pBlock->dstRect.h = (int)(pBlock->srcRect.h * scaleFactor + 0.5f);
    printf("Block frame size (after scaling): w: %d, h: %d\n", pBlock->dstRect.w, pBlock->dstRect.h);
    */

    // koden nedanför bör skrivas om, men används sålänge: 
    pBlock->dstRect.w = pBlock->srcRect.w;
    pBlock->dstRect.h = pBlock->srcRect.h;
    pBlock->dstRect.w = pBlock->pGameAreaRect->w / BOX_COL;
    pBlock->dstRect.h = pBlock->pGameAreaRect->h / BOX_ROW;

    return pBlock;
}

SDL_Rect getBlockRect(Block *pBlock) {
    return pBlock->dstRect;
}

void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY)
{
    for (int row = 0; row < BOX_ROW; row++)
    {
        for (int col = 0; col < BOX_COL; col++)
        {
            if (gameMap[row][col] != 0)
            {
                pBlock->dstRect.x = col * pBlock->dstRect.w;
                pBlock->dstRect.y = row * pBlock->dstRect.h;
                pBlock->dstRect.y -= CamY;
                drawBlock(pBlock, gameMap[row][col]);
                pBlock->dstRect.y += CamY;
            }
        }
    }
}

void drawBlock(Block *pBlock, int block_type)
{
    if (block_type >= 1 && block_type <= 3) {    // Det finns tre olika bilder i spritesheetet för blocktyp
        pBlock->srcRect.x = pBlock->srcRect.w * (block_type-1);    // Bestämmer om det är 1, 2 eller 3 block som ska ritas ut
        SDL_RenderCopy(pBlock->pRenderer, pBlock->pTexture, &pBlock->srcRect, &pBlock->dstRect);
    }
}

void destroyBlock(Block *pBlock) {
    if (pBlock == NULL) return;
    if (pBlock->pTexture != NULL) {
        SDL_DestroyTexture(pBlock->pTexture);
        pBlock->pTexture = NULL;      
    }
    free(pBlock);
    pBlock = NULL;
}

/*
void destroyBlockImage(BlockImage *pBlockImage)
{
    SDL_DestroyTexture(pBlockImage->pTexture);
}
*/
