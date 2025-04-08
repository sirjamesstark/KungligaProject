#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "../include/player.h"

struct player
{
    float x, y, vx, vy;
    int angle;
    int window_width, window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect playerRect;
};

static float distance(int x1, int y1, int x2, int y2);

Player *createPlayer(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height)
{
    Player *pPlayer = malloc(sizeof(struct player));
    pPlayer->vx = pPlayer->vy = 0;
    pPlayer->angle = 0;
    pPlayer->window_width = window_width;
    pPlayer->window_height = window_height;
    SDL_Surface *pSurface = IMG_Load("resources/Ship.png");
    if (!pSurface)
    {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pPlayer->pTexture)
    {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_QueryTexture(pPlayer->pTexture, NULL, NULL, &(pPlayer->playerRect.w), &(pPlayer->playerRect.h));
    pPlayer->playerRect.w /= 4;
    pPlayer->playerRect.h /= 4;
    pPlayer->x = x - pPlayer->playerRect.w / 2;
    pPlayer->y = y - pPlayer->playerRect.h / 2;
    return pPlayer;
}

void turnLeft(Player *pPlayer)
{
    pPlayer->angle -= 5;
}

void turnRight(Player *pPlayer)
{
    pPlayer->angle += 5;
}

void updatePlayer(Player *pPlayer)
{
    pPlayer->x += pPlayer->vx;
    pPlayer->y += pPlayer->vy;
    if (pPlayer->x < 0)
        pPlayer->x += pPlayer->window_width;
    else if (pPlayer->x > pPlayer->window_width)
        pPlayer->x -= pPlayer->window_width;
    if (pPlayer->y < 0)
        pPlayer->y += pPlayer->window_height;
    else if (pPlayer->y > pPlayer->window_height)
        pPlayer->y -= pPlayer->window_height;
    pPlayer->playerRect.x = pPlayer->x;
    pPlayer->playerRect.y = pPlayer->y;
}

void drawPlayer(Player *pPlayer)
{
    SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, NULL, &(pPlayer->playerRect), pPlayer->angle, NULL, SDL_FLIP_NONE);
}

void destroyPlayer(Player *pPlayer)
{
    SDL_DestroyTexture(pPlayer->pTexture);
    free(pPlayer);
}

int collidePlayer(Player *pPlayer, SDL_Rect rect)
{
    // return SDL_HasIntersection(&(pPlayer->playerRect),&rect);
    return distance(pPlayer->playerRect.x + pPlayer->playerRect.w / 2, pPlayer->playerRect.y + pPlayer->playerRect.h / 2, rect.x + rect.w / 2, rect.y + rect.h / 2) < (pPlayer->playerRect.w + rect.w) / 2;
}

static float distance(int x1, int y1, int x2, int y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}