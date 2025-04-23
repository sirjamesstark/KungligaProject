#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <math.h>
#include <stdbool.h>
#include "../include/player.h"

#define PLAYER_SCALEFACTOR 0.04

struct frames {
    int nrOfFrames_idle;
    int nrOfFrames_sprint;
    int nrOfFrames_jump;
    int currentFrame_x, currentFrame_y; 
    float character_w, character_h; 
    bool is_mirrored;
    int frameDelay;
    Uint32 lastFrameTime; 
};

struct player {
    float x, y, vx, vy;
    int window_width, window_height;
    Frames frames;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect srcRect;   // srcRect.w och srcRect.h lagrar den verkliga storleken av en frame i spritesheetet, srcRect.x och srcRect.y anger vilken frame i spritesheetet som väljs
    SDL_Rect dstRect;   // dstRect.w och dstRect.h är en nerskalad variant av srcRect.w och srcRect.h, srcRect.x och srcRect.y anger var i fönstret som den aktuella framen i srcRect.x och srcRect.y ska ritas upp
};

Player *createPlayer(SDL_Rect blockRect, SDL_Renderer *pRenderer, int window_width, int window_height) {
    int player_ID = 3; // har bara nu tills vidare under testing, denna variabel bestämmer vilken spelgubbe som ska laddas in

    Player *pPlayer = malloc(sizeof(struct player));
    if (pPlayer == NULL) return NULL;

    pPlayer->vx=pPlayer->vy = 0;
    pPlayer->window_width = window_width;
    pPlayer->window_height = window_height;

    SDL_Surface* pSurface = NULL; 
    switch (player_ID) {
        case 0:
            pSurface = IMG_Load("resources/player_0.png");
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 11;
            pPlayer->frames.character_w = 50; 
            pPlayer->frames.character_h = 75;  
            break; 
        case 1:
            pSurface = IMG_Load("resources/player_1.png");
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 8;
            pPlayer->frames.character_w = 60; 
            pPlayer->frames.character_h = 70;   
            break; 
        case 2:
            pSurface = IMG_Load("resources/player_2.png");
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 7;
            pPlayer->frames.character_w = 50; 
            pPlayer->frames.character_h = 70;  
            break;
        case 3:
            pSurface = IMG_Load("resources/player_3.png");
            pPlayer->frames.nrOfFrames_idle = 8;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 8;
            pPlayer->frames.character_w = 50;  
            pPlayer->frames.character_h = 71; 
            break;
        default:
            destroyPlayer(pPlayer);
            return NULL;
            break; 
    }

    pPlayer->frames.currentFrame_x = pPlayer->frames.currentFrame_y = 0;
    pPlayer->frames.is_mirrored = false;
    pPlayer->frames.frameDelay = 200; // 100 ms = 10 frames per sekund
    pPlayer->frames.lastFrameTime = SDL_GetTicks();

    if (!pSurface) {
        destroyPlayer(pPlayer); 
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }

    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);

    if (!pPlayer->pTexture) {
        destroyPlayer(pPlayer); 
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }

    int spritesheet_w, spritesheet_h; 
    SDL_QueryTexture(pPlayer->pTexture, NULL, NULL, &spritesheet_w, &spritesheet_h);
    pPlayer->srcRect.w = pPlayer->srcRect.h = spritesheet_h/3;
    pPlayer->srcRect.x = (pPlayer->frames.currentFrame_x) * pPlayer->srcRect.w;
    pPlayer->srcRect.y =  (pPlayer->frames.currentFrame_y) * pPlayer->srcRect.h;

    float scaleFactor = (float)pPlayer->window_width/(pPlayer->srcRect.w) * PLAYER_SCALEFACTOR;
    pPlayer->dstRect.w = (int)(pPlayer->srcRect.w * scaleFactor);
    pPlayer->dstRect.h = (int)((pPlayer->srcRect.h * scaleFactor));
    pPlayer->frames.character_w *= scaleFactor; 
    pPlayer->frames.character_h *= scaleFactor;

    //pPlayer->dstRect.w = ((pPlayer->window_width)/BOX_COL);
    //pPlayer->dstRect.h = ((pPlayer->window_height)/BOX_ROW);
    
    pPlayer->x = blockRect.w * 2;
    pPlayer->y = pPlayer->window_height - (pPlayer->window_height - (BOX_ROW * blockRect.h)) - blockRect.h*2 - pPlayer->dstRect.h;

    //pPlayer->dstRect.w = (pPlayer->srcRect.w) <-- multiplicera med skalfaktor
    //pPlayer->dstRect.h = (pPlayer->srcRect.h) <-- multiplicera med skalfaktor
    //pPlayer->dstRect.w = pPlayer->window_width/pPlayer->srcRect.w; 
    //pPlayer->dstRect.x = pPlayer->x - (pPlayer->dstRect.w/2); // Anger koordinaterna för gubbens placering på skärmen vänster i övre hörn
    //pPlayer->dstRect.y = pPlayer->y - (pPlayer->dstRect.h/2);
    return pPlayer;
}

void setSpeed(bool up,bool down,bool left,bool right,bool *pGoUp,bool *pGoDown,bool *pGoLeft,bool *pGoRight,
                int *pUpCounter, bool onGround,Player *pPlayer,int speedX,int speedY) 
{
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
        if (!onGround || (*pUpCounter)>0) pPlayer->frames.currentFrame_y = 2;
        else pPlayer->frames.currentFrame_y = 1;
    } 
    else if (right && !left) 
    {
        pPlayer->vx = speedX;
        (*pGoRight) = 1;
        pPlayer->frames.is_mirrored = false;
        if (!onGround || (*pUpCounter)>0) pPlayer->frames.currentFrame_y = 2;
        else pPlayer->frames.currentFrame_y = 1;
    }
}

void updatePlayer(Player *pPlayer,float deltaTime,int gameMap[BOX_ROW][BOX_COL],SDL_Rect blockRect,int *pUpCounter,bool *pOnGround,
                    bool *pGoUp,bool *pGoDown,bool *pGoLeft,bool *pGoRight)
{
    pPlayer->x += pPlayer->vx * 5 * deltaTime;
    pPlayer->y += pPlayer->vy * deltaTime;
    // Check Collision
    if ((*pGoLeft) == 1)
    {
        // printf("y: %d\n", (((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h);
        // printf("x: %d\n", ((int)pPlayer->x)/blockRect.w);
        if (gameMap[(int)(pPlayer->y + 22 + pPlayer->frames.character_h)/blockRect.h][((int)pPlayer->x + 25)/blockRect.w] == 1)  // Bottom edge blocked on left?
        {
            pPlayer->x -= (pPlayer->vx * 5 * deltaTime);         //Dont move
        }
        else if (gameMap[((int)pPlayer->y + 25)/blockRect.h][((int)pPlayer->x + 25)/blockRect.w] == 1)     // Top edge blocked on left?
        {
            pPlayer->x -= (pPlayer->vx * 5 * deltaTime);         //Dont move
        }
    }
    
    if ((*pGoRight) == 1)
    {
        // printf("y: %d\n", (((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h);
        // printf("x: %d\n", (((int)pPlayer->x) + pPlayer->playerRect.w) / blockRect.w);
        if (gameMap[(int)(pPlayer->y + 22 + pPlayer->frames.character_h)/blockRect.h][(int)(pPlayer->x + 10 + pPlayer->frames.character_w) / blockRect.w] == 1 || // Bottom edge blocked on right?
            gameMap[((int)pPlayer->y + 25)/blockRect.h][(int)(pPlayer->x + 10 + pPlayer->frames.character_w) / blockRect.w] == 1)  // Top edge blocked on right?
        {
            pPlayer->x -= (pPlayer->vx * 5 * deltaTime);         //Dont move
        }
    }

    if ((*pGoUp) == 1)
    {
        // printf("y: %d,\n", ((int)pPlayer->y + 1)/blockRect.h);
        // printf("x: %d,\n", ((int)pPlayer->x + 1)/blockRect.w);
        if (gameMap[((int)pPlayer->y + 25)/blockRect.h][((int)pPlayer->x + 25)/blockRect.w] == 1 ||   // Left edge blocked on top?
            gameMap[((int)pPlayer->y + 25)/blockRect.h][(int)(pPlayer->x + 10 + pPlayer->frames.character_w)/blockRect.w] == 1) // Right edge blocked on top?
        {

            pPlayer->y -= (pPlayer->vy * deltaTime);             //Dont move
            (*pUpCounter) = 0;
        }
    }

    if ((*pGoDown) == 1)
    {
        if (gameMap[(int)(pPlayer->y + 22 + pPlayer->frames.character_h)/blockRect.h][((int)pPlayer->x + 25)/blockRect.w] == 1 || // Left edge blocked on bottom?
            gameMap[(int)(pPlayer->y + 22 + pPlayer->frames.character_h)/blockRect.h][(int)(pPlayer->x + 10 + pPlayer->frames.character_w)/blockRect.w] == 1)  // Right edge blocked on bottom?
        {
            pPlayer->y -= (pPlayer->vy * deltaTime);             //Dont move
            (*pOnGround) = true;

        }
    }
    if (gameMap[(int)(pPlayer->y + 26 + pPlayer->frames.character_h)/blockRect.h][((int)pPlayer->x + 25 )/blockRect.w] == 0 && // Left edge blocked on bottom?
        gameMap[(int)(pPlayer->y + 26 + pPlayer->frames.character_h)/blockRect.h][(int)(pPlayer->x + 10 + pPlayer->frames.character_w)/blockRect.w] == 0)  // Right edge blocked on bottom?
    {
        (*pOnGround) = false;
    }
    pPlayer->dstRect.x = pPlayer->x;
    pPlayer->dstRect.y = pPlayer->y - 3;

    if (pPlayer->x < 0) 
    {
        pPlayer->x = 0;             //gör så att man inte kan falla ned i vänster hörnet
    }
       
}

void updatePlayerRect(Player *pPlayer) 
{
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - pPlayer->frames.lastFrameTime < pPlayer->frames.frameDelay) return;
    pPlayer->frames.lastFrameTime = currentTime;

    if (pPlayer->frames.currentFrame_y == 0) 
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_idle-1) 
        {
            pPlayer->frames.currentFrame_x +=1;
        }
        else pPlayer->frames.currentFrame_x = 0;
    } 
    else if (pPlayer->frames.currentFrame_y == 1) 
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_sprint-1) 
        {
            pPlayer->frames.currentFrame_x +=1;
        }
        else pPlayer->frames.currentFrame_x = 0;
    }
    else if (pPlayer->frames.currentFrame_y == 2) 
    {
        if (pPlayer->frames.currentFrame_x < pPlayer->frames.nrOfFrames_jump-1) 
        {
            pPlayer->frames.currentFrame_x +=1;
        }
        else pPlayer->frames.currentFrame_x = 0; 
    }

    pPlayer->srcRect.x = pPlayer->frames.currentFrame_x * pPlayer->srcRect.w;
    pPlayer->srcRect.y = pPlayer->frames.currentFrame_y * pPlayer->srcRect.h;
}

void drawPlayer(Player *pPlayer) 
{
    updatePlayerRect(pPlayer);
    if (pPlayer->frames.is_mirrored == true) 
    {
        SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &pPlayer->dstRect, 0, NULL, SDL_FLIP_HORIZONTAL);
    }
    else {
        SDL_RenderCopy(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &pPlayer->dstRect);
    }
}

void destroyPlayer(Player *pPlayer) {
    if (pPlayer == NULL) return;
    if (pPlayer->pTexture != NULL) {
        SDL_DestroyTexture(pPlayer->pTexture);
        pPlayer->pTexture = NULL; 
    }
    free(pPlayer);
    pPlayer = NULL;
}