#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_net.h>
#include <math.h>
#include <stdbool.h>
#include "../include/player.h"

#define PLAYER_SCALEFACTOR 0.04

struct frames
{
    int nrOfFrames_idle, nrOfFrames_sprint, nrOfFrames_jump;
    int currentFrame_x, currentFrame_y;
    float character_w, character_h;
    bool is_mirrored;
    int frameDelay;
    Uint32 lastFrameTime;
};

struct player
{
    float x, y, vx, vy;
    // testing
    float oldX, oldY;
    // new variables
    int window_width, window_height;
    Frames frames;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect srcRect; // srcRect.w och srcRect.h lagrar den verkliga storleken av en frame i spritesheetet, srcRect.x och srcRect.y anger vilken frame i spritesheetet som väljs
    SDL_Rect dstRect; // dstRect.w och dstRect.h är en nerskalad variant av srcRect.w och srcRect.h, srcRect.x och srcRect.y anger var i fönstret som den aktuella framen i srcRect.x och srcRect.y ska ritas upp
    bool active;
};

SDL_Rect *getPlayerRect(Player *pPly)
{
    return &(pPly->srcRect);
}

int getPlyX(Player *pPlayer)
{
    return pPlayer->dstRect.x;
}

int getPlyY(Player *pPlayer)
{
    return pPlayer->dstRect.y;
}

Player *createPlayer(int player_ID, SDL_Rect blockRect, SDL_Renderer *pRenderer, int window_width, int window_height) 
{
    Player *pPlayer = malloc(sizeof(struct player));
    if (pPlayer == NULL) return NULL;
    SDL_Surface *pSurface = initPlayerFrames(pPlayer, player_ID);

    if (pSurface == NULL)
    {
        destroyPlayer(pPlayer);
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pPlayer->pTexture)
    {
        destroyPlayer(pPlayer);
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    pPlayer->frames.currentFrame_x = pPlayer->frames.currentFrame_y = 0;
    pPlayer->frames.is_mirrored = false;
    pPlayer->frames.frameDelay = 200; // 100 ms = 10 frames per sekund
    pPlayer->frames.lastFrameTime = SDL_GetTicks();

    pPlayer->vx = pPlayer->vy = 0;
    pPlayer->window_width = window_width;
    pPlayer->window_height = window_height;

    int spritesheet_w, spritesheet_h;
    SDL_QueryTexture(pPlayer->pTexture, NULL, NULL, &spritesheet_w, &spritesheet_h);
    pPlayer->srcRect.w = pPlayer->srcRect.h = spritesheet_h / 3;
    pPlayer->srcRect.x = (pPlayer->frames.currentFrame_x) * pPlayer->srcRect.w;
    pPlayer->srcRect.y = (pPlayer->frames.currentFrame_y) * pPlayer->srcRect.h;

    float scaleFactor = (float)pPlayer->window_width / (pPlayer->srcRect.w) * PLAYER_SCALEFACTOR;
    pPlayer->dstRect.w = (int)(pPlayer->srcRect.w * scaleFactor);
    pPlayer->dstRect.h = (int)((pPlayer->srcRect.h * scaleFactor));
    pPlayer->frames.character_w *= scaleFactor;
    pPlayer->frames.character_h *= scaleFactor;

    // pPlayer->dstRect.w = ((pPlayer->window_width)/BOX_COL);
    // pPlayer->dstRect.h = ((pPlayer->window_height)/BOX_ROW);

    pPlayer->oldX = pPlayer->x = blockRect.w * 2;
    pPlayer->oldX = pPlayer->y = pPlayer->window_height - (pPlayer->window_height - (BOX_ROW * blockRect.h)) - blockRect.h * 2 - pPlayer->dstRect.h;

    // pPlayer->dstRect.w = (pPlayer->srcRect.w) <-- multiplicera med skalfaktor
    // pPlayer->dstRect.h = (pPlayer->srcRect.h) <-- multiplicera med skalfaktor
    // pPlayer->dstRect.w = pPlayer->window_width/pPlayer->srcRect.w;
    // pPlayer->dstRect.x = pPlayer->x - (pPlayer->dstRect.w/2); // Anger koordinaterna för gubbens placering på skärmen vänster i övre hörn
    // pPlayer->dstRect.y = pPlayer->y - (pPlayer->dstRect.h/2);
    return pPlayer;
}

SDL_Surface *initPlayerFrames(Player *pPlayer, int player_ID) {
    switch (player_ID) {
        case 0:
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 11;
            pPlayer->frames.character_w = 50;
            pPlayer->frames.character_h = 75;
            return IMG_Load("resources/player_0.png");
        case 1:
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 8;
            pPlayer->frames.character_w = 60;
            pPlayer->frames.character_h = 70;
            return IMG_Load("resources/player_1.png");
        case 2:
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 7;
            pPlayer->frames.character_w = 50;
            pPlayer->frames.character_h = 70;
            return IMG_Load("resources/player_2.png");
        case 3:
            pPlayer->frames.nrOfFrames_idle = 8;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 8;
            pPlayer->frames.character_w = 50;
            pPlayer->frames.character_h = 71;
            return IMG_Load("resources/player_3.png");
        default:
            return NULL;
    }
}

// bool isSolidTile(Player pPlayer, int row, int col)
// {
//     int gameMap[row][col];
//     if (row < 0 || row >= pPlayer.window_height || col < 0 || col >= pPlayer.window_width)
//     {
//         return true; // Treat out-of-bounds as solid
//     }
//     return gameMap[row][col] != 0;
// }

void setSpeed(bool up, bool down, bool left, bool right, bool *pGoUp, bool *pGoDown, bool *pGoLeft, bool *pGoRight,
              int *pUpCounter, bool onGround, Player *pPlayer)
{
    int speedX = pPlayer->window_width / 20; 
    int speedY = pPlayer->window_height / 20;

    pPlayer->vx = pPlayer->vy = 0;
    pPlayer->frames.currentFrame_y = 0;

    if (up && !down && onGround)
    {
        (*pUpCounter) = COUNTER;
    }

    if ((*pUpCounter) > 0)
    {
        (*pGoUp) = 1;
        pPlayer->vy = -(speedY * 5);
        (*pUpCounter)--;
    }
    else if (!onGround)
    {
        (*pGoDown) = 1;
        pPlayer->vy = (speedY) * 5;
    }

    if (left && !right)
    {
        pPlayer->vx = -(speedX);
        (*pGoLeft) = 1;
        pPlayer->frames.is_mirrored = true;
        if (!onGround || (*pUpCounter) > 0)
            pPlayer->frames.currentFrame_y = 2;
        else
            pPlayer->frames.currentFrame_y = 1;
    }
    else if (right && !left)
    {
        pPlayer->vx = speedX;
        (*pGoRight) = 1;
        pPlayer->frames.is_mirrored = false;
        if (!onGround || (*pUpCounter) > 0)
            pPlayer->frames.currentFrame_y = 2;
        else
            pPlayer->frames.currentFrame_y = 1;
    }
}

void updatePlayer(Player *pPlayer[MAX_NROFPLAYERS], float deltaTime, int gameMap[BOX_ROW][BOX_COL], SDL_Rect blockRect,
                  int *pUpCounter, bool *pOnGround, bool *pGoUp, bool *pGoDown, bool *pGoLeft, bool *pGoRight, UDPpacket *p,
                  UDPpacket *p2, int *pIs_server, IPaddress srvadd, UDPsocket *pSd)
{
    float space = pPlayer[0]->window_height - blockRect.h * (BOX_ROW - 1);
    pPlayer[0]->active = true;
    pPlayer[0]->x += pPlayer[0]->vx * 5 * deltaTime;
    pPlayer[0]->y += pPlayer[0]->vy * deltaTime;

    networkUDP(pPlayer, p, p2, pIs_server, srvadd, pSd, space);

    // Check Collision
    if ((*pGoLeft) != 0)
    {
        // printf("y: %d\n", (((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h);
        // printf("x: %d\n", ((int)pPlayer->x)/blockRect.w);
        if (gameMap[(int)(pPlayer[0]->y + 22 + pPlayer[0]->frames.character_h) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0) // Bottom edge blocked on left?
        {
            pPlayer[0]->x -= (pPlayer[0]->vx * 5 * deltaTime); // Dont move
        }
        else if (gameMap[((int)pPlayer[0]->y + 25) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0) // Top edge blocked on left?
        {
            pPlayer[0]->x -= (pPlayer[0]->vx * 5 * deltaTime); // Dont move
        }
    }

    if ((*pGoRight) != 0)
    {
        // printf("y: %d\n", (((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h);
        // printf("x: %d\n", (((int)pPlayer->x) + pPlayer->playerRect.w) / blockRect.w);
        if (gameMap[(int)(pPlayer[0]->y + 22 + pPlayer[0]->frames.character_h) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.character_w) / blockRect.w] != 0 || // Bottom edge blocked on right?
            gameMap[((int)pPlayer[0]->y + 25) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.character_w) / blockRect.w] != 0)                                    // Top edge blocked on right?
        {
            pPlayer[0]->x -= (pPlayer[0]->vx * 5 * deltaTime); // Dont move
        }
    }

    if ((*pGoUp) != 0)
    {
        // printf("y: %d,\n", ((int)pPlayer->y + 1)/blockRect.h);
        // printf("x: %d,\n", ((int)pPlayer->x + 1)/blockRect.w);
        if (gameMap[((int)pPlayer[0]->y + 25) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0 ||                                // Left edge blocked on top?
            gameMap[((int)pPlayer[0]->y + 25) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.character_w) / blockRect.w] != 0) // Right edge blocked on top?
        {

            pPlayer[0]->y -= (pPlayer[0]->vy * deltaTime); // Dont move
            (*pUpCounter) = 0;
        }
    }

    if ((*pGoDown) != 0)
    {
        if (gameMap[(int)(pPlayer[0]->y + 22 + pPlayer[0]->frames.character_h) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0 ||                                // Left edge blocked on bottom?
            gameMap[(int)(pPlayer[0]->y + 22 + pPlayer[0]->frames.character_h) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.character_w) / blockRect.w] != 0) // Right edge blocked on bottom?
        {
            pPlayer[0]->y -= (pPlayer[0]->vy * deltaTime); // Dont move
            (*pOnGround) = true;
        }
    }
    if (gameMap[(int)(pPlayer[0]->y + 26 + pPlayer[0]->frames.character_h) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] == 0 &&                                // Left edge blocked on bottom?
        gameMap[(int)(pPlayer[0]->y + 26 + pPlayer[0]->frames.character_h) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.character_w) / blockRect.w] == 0) // Right edge blocked on bottom?
    {
        (*pOnGround) = false;
    }
    pPlayer[0]->dstRect.x = pPlayer[0]->x;
    pPlayer[0]->dstRect.y = pPlayer[0]->y - 3;
    pPlayer[1]->dstRect.x = pPlayer[1]->x;
    pPlayer[1]->dstRect.y = pPlayer[1]->y - 3;
    if (pPlayer[0]->x < 0)
    {
        pPlayer[0]->x = 0; // gör så att man inte kan falla ned i vänster hörnet
    }
}

void updatePlayerRect(Player *pPlayer)
{
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - pPlayer->frames.lastFrameTime < pPlayer->frames.frameDelay)
        return;
    pPlayer->frames.lastFrameTime = currentTime;

    if (pPlayer->frames.currentFrame_y == 0)
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_idle - 1)
        {
            pPlayer->frames.currentFrame_x += 1;
        }
        else
            pPlayer->frames.currentFrame_x = 0;
    }
    else if (pPlayer->frames.currentFrame_y == 1)
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_sprint - 1)
        {
            pPlayer->frames.currentFrame_x += 1;
        }
        else
            pPlayer->frames.currentFrame_x = 0;
    }
    else if (pPlayer->frames.currentFrame_y == 2)
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_jump - 1)
        {
            pPlayer->frames.currentFrame_x += 1;
        }
        else
            pPlayer->frames.currentFrame_x = 0;
    }
    else if (pPlayer->frames.currentFrame_y == 2)
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_jump - 1)
        {
            pPlayer->frames.currentFrame_x += 1;
        }
        else
            pPlayer->frames.currentFrame_x = 0;
    }

    pPlayer->srcRect.x = pPlayer->frames.currentFrame_x * pPlayer->srcRect.w;
    pPlayer->srcRect.y = pPlayer->frames.currentFrame_y * pPlayer->srcRect.h;
}

void networkUDP(Player *pPlayer[MAX_NROFPLAYERS], UDPpacket *p, UDPpacket *p2, int *pIs_server, IPaddress srvadd, UDPsocket *pSd, float space)
{
        if (pPlayer[0]->oldX != pPlayer[0]->x || pPlayer[0]->oldY != pPlayer[0]->y)
        {
            sprintf((char *)p->data, "%f %f", pPlayer[0]->x / pPlayer[0]->window_width, (pPlayer[0]->y + space) / pPlayer[0]->window_height);
            p->len = strlen((char *)p->data) + 1;
    
            if (!(*pIs_server))
            {
                p->address.host = srvadd.host;
                p->address.port = srvadd.port;
            }
    
            SDLNet_UDP_Send(*pSd, -1, p);
            pPlayer[0]->oldX = pPlayer[0]->x;
            pPlayer[0]->oldY = pPlayer[0]->y;
        }
        if (SDLNet_UDP_Recv(*pSd, p2))
        {
            float a, b;
            sscanf((char *)p2->data, "%f %f", &a, &b);
            pPlayer[1]->x = a * pPlayer[0]->window_width;
            pPlayer[1]->y = b * pPlayer[0]->window_height - space;
            pPlayer[1]->active = true;
            if (*pIs_server)
            {
                sprintf((char *)p->data, "%f %f", pPlayer[0]->x / pPlayer[0]->window_width, (pPlayer[0]->y + space) / pPlayer[0]->window_height);
                p->address = p2->address;
                p->len = strlen((char *)p->data) + 1;
                SDLNet_UDP_Send(*pSd, -1, p);
            }
        }
}

void drawPlayer(Player *pPlayer, int CamX, int CamY, int window_width, int window_height)
{
    updatePlayerRect(pPlayer);

    SDL_Rect screenRect = {
        (int)(pPlayer->x - CamX),
        (int)(pPlayer->y - CamY),
        pPlayer->dstRect.w,
        pPlayer->dstRect.h};

    if (pPlayer->frames.is_mirrored == true)
    {
        SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &screenRect, 0, NULL, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        SDL_RenderCopy(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &screenRect);
    }
}

void destroyPlayer(Player *pPlayer)
{
    if (pPlayer == NULL)
        return;
    if (pPlayer->pTexture != NULL)
    {
        SDL_DestroyTexture(pPlayer->pTexture);
        pPlayer->pTexture = NULL;
    }
    free(pPlayer);
    pPlayer = NULL;
}