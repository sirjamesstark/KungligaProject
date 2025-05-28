#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "../include/platform.h"
#include "../include/scaling.h"

struct block
{
    int nrOfFrames_blocks;
    SDL_Renderer *pRenderer;
    SDL_Rect *pScreenRect;
    SDL_Texture *pTexture;
    SDL_Rect srcRect; 
    SDL_Rect dstRect; 
};

Block *createBlock(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect)
{
    if (!pRenderer || !pScreenRect)
    {
        printf("Error: Invalid parameters. pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Block *pBlock = malloc(sizeof(struct block));
    if (pBlock == NULL)
    {
        printf("Error: Failed to allocate memory for block.\n");
        return NULL;
    }

    SDL_Surface *pSurface = IMG_Load("resources/block_sprites_effects23.png");
    if (!pSurface)
    {
        destroyBlock(pBlock);
        printf("Error: Failed to initialize block frames.\n");
        return NULL;
    }

    pBlock->pRenderer = pRenderer;
    pBlock->pScreenRect = pScreenRect;
    pBlock->nrOfFrames_blocks = 3;

    pBlock->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pBlock->pTexture)
    {
        destroyBlock(pBlock);
        printf("Error: Failed to create block texture.\n");
        return NULL;
    }

    SDL_QueryTexture(pBlock->pTexture, NULL, NULL, &(pBlock->srcRect.w), &(pBlock->srcRect.h));
    pBlock->srcRect.w /= 3;
    printf("\nBlock size:\n");
    pBlock->dstRect = scaleRect(pBlock->srcRect, *pBlock->pScreenRect, BLOCK_SCALEFACTOR);

    return pBlock;
}

SDL_Rect getBlockRect(Block *pBlock)
{
    return pBlock->dstRect;
}

float getShiftY(Block *pBlock)
{
    return (float)(pBlock->pScreenRect->h - pBlock->dstRect.h * (BOX_SCREEN_Y));
}

float getShiftX(Block *pBlock)
{
    return (float)(pBlock->pScreenRect->w - (pBlock->dstRect.w * BOX_COL)) / 2.0f;
}

void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY, SDL_Rect *screenRect)
{
    float shift_col_0 = (float)(screenRect->w - (pBlock->dstRect.w * BOX_COL)) / 2.0f;

    for (int row = BOX_ROW - 1; row >= 0; row--)
    {
        int blockScreenY = (screenRect->h + screenRect->y) - (BOX_ROW - row) * pBlock->dstRect.h;
        int blockYRelativeToCam = blockScreenY - CamY;

        if (blockYRelativeToCam + pBlock->dstRect.h < 0 || blockYRelativeToCam > screenRect->h)
            continue;

        for (int col = 0; col <= BOX_COL; col++)
        {
            if (gameMap[row][col] != 0)
            {

                pBlock->dstRect.x = (int)(col * pBlock->dstRect.w + shift_col_0 + screenRect->x);
                pBlock->dstRect.y = blockYRelativeToCam + screenRect->y;
                pBlock->dstRect.w = pBlock->dstRect.w;
                pBlock->dstRect.h = pBlock->dstRect.h;

                drawBlock(pBlock, gameMap[row][col], &pBlock->dstRect);
            }
        }
    }
}

void drawBlock(Block *pBlock, int block_type, SDL_Rect *dstRect)
{
    if (block_type >= 1 && block_type <= 3)
    {
        SDL_Rect srcRect =
            {
                .x = (block_type - 1) * pBlock->srcRect.w, 
                .y = 0,
                .w = pBlock->srcRect.w,
                .h = pBlock->srcRect.h};
        SDL_RenderCopy(pBlock->pRenderer, pBlock->pTexture, &srcRect, dstRect);
    }
}

void destroyBlock(Block *pBlock)
{
    if (!pBlock)
        return;
    if (pBlock->pTexture)
    {
        SDL_DestroyTexture(pBlock->pTexture);
        pBlock->pTexture = NULL;
    }
    free(pBlock);
}
