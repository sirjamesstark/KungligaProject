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
    SDL_Rect srcRect; // srcRect.w och srcRect.h lagrar den verkliga storleken av en frame i spritesheetet, srcRect.x och srcRect.y anger vilken frame i spritesheetet för blocks som väljs
    SDL_Rect dstRect; // dstRect.w och dstRect.h är en nerskalad variant av srcRect.w och srcRect.h, dstRect.x och dstRect.y anger var i fönstret som den aktuella framen i srcRect.x och srcRect.y ska ritas upp
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

float getShiftLength(Block *pBlock)
{
    return (float)(pBlock->pScreenRect->w - (pBlock->dstRect.w * BOX_COL)) / 2.0f;
}

void buildTheMap(int gameMap[BOX_ROW][BOX_COL], Block *pBlock, int CamY) {
    float shift_cols = (float)(pBlock->pScreenRect->w - (pBlock->dstRect.w * BOX_COL)) / 2.0f;
    int startX_leftBlock = pBlock->pScreenRect->x + (int)(shift_cols + 0.5f);
    int startY_bottomBlock = pBlock->pScreenRect->y + pBlock->pScreenRect->h - pBlock->dstRect.h;

    for (int row = BOX_ROW - 1; row >= 0; row--)
    {
        int blockScreenY = startY_bottomBlock - (BOX_ROW - 1 - row) * pBlock->dstRect.h;
        int blockYRelativeToCam = blockScreenY - CamY;

        // Hoppa över block utanför bus skärm #viktigt
        if (blockYRelativeToCam + pBlock->dstRect.h < 0 || blockYRelativeToCam > pBlock->pScreenRect->h)
            continue;

        for (int col = 0; col < BOX_COL; col++)
        {
            if (gameMap[row][col]) {
                pBlock->dstRect.x = startX_leftBlock + pBlock->dstRect.w * col;
                pBlock->dstRect.y = pBlock->pScreenRect->y + blockYRelativeToCam;
                drawBlock(pBlock, gameMap[row][col]);
            }
        }
    }
}

void drawBlock(Block *pBlock, int block_type) {
    if (block_type >= 1 && block_type <= 3) {
        pBlock->srcRect.x = pBlock->srcRect.w * (block_type - 1);
        SDL_RenderCopy(pBlock->pRenderer, pBlock->pTexture, &pBlock->srcRect, &pBlock->dstRect);
    }
}

void destroyBlock(Block *pBlock)
{
    if (!pBlock) return;
    if (pBlock->pTexture) {
        SDL_DestroyTexture(pBlock->pTexture);
        pBlock->pTexture = NULL;
    }
    free(pBlock);
}
