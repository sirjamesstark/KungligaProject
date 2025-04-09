#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "../include/player.h"

struct frames {
    int nrOfFrames_idle;
    int nrOfFrames_sprint;
    int nrOfFrames_jump;
    int currentFrame_x, currentFrame_y; 
};

struct player {
    float x, y, vx, vy;
    int window_width,window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect srcRect;   // srcRect.w och srcRect.h lagrar den verkliga storleken av en frame i spritesheetet, srcRect.x och srcRect.y anger vilken frame i spritesheetet som väljs
    SDL_Rect dstRect;   // dstRect.w och dstRect.h är en nerskalad variant av srcRect.w och srcRect.h, srcRect.x och srcRect.y anger var i fönstret som den aktuella framen i srcRect.x och srcRect.y ska ritas upp
};

static float distance(int x1, int y1, int x2, int y2);

Player *createPlayer(SDL_Rect blockRect, SDL_Renderer *pRenderer, int window_width, int window_height){
    Player *pPlayer = malloc(sizeof(struct player));
    pPlayer->vx=pPlayer->vy=0;
    pPlayer->window_width = window_width;
    pPlayer->window_height = window_height;

    SDL_Surface* pSurface = NULL; 
    switch (player_ID) {
        case 0:
            pSurface = IMG_Load("resources/player_0.png");
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 11; 
            break; 
        case 1:
            pSurface = IMG_Load("resources/player_1.png");
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 8; 
            break; 
        case 2:
            pSurface = IMG_Load("resources/player_2.png");
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 7;  
            break;
        case 3:
            pSurface = IMG_Load("resources/player_3.png");
            pPlayer->frames.nrOfFrames_idle = 8;
            pPlayer->frames.nrOfFrames_sprint = 8; 
            pPlayer->frames.nrOfFrames_jump = 8;  
            break;
        default:
            destroyPlayer(pPlayer);
            return NULL;
            break; 
    }
    pPlayer->frames.currentFrame_x = pPlayer->frames.currentFrame_y = 0;

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
    SDL_QueryTexture(pPlayer->pTexture,NULL,NULL,&(pPlayer->playerRect.w),&(pPlayer->playerRect.h));
    pPlayer->playerRect.w = ((pPlayer->window_width)/BOX_COL)/2;
    pPlayer->playerRect.h = ((pPlayer->window_height)/BOX_ROW)/2;
    pPlayer->x = blockRect.w * 2;
    pPlayer->y = pPlayer->window_height - (pPlayer->window_height - (BOX_ROW * blockRect.h)) - blockRect.h*2 - pPlayer->playerRect.h ;
    return pPlayer;
}

void turnLeft(Player *pPlayer){
    // pPlayer->angle-=5;
}

void turnRight(Player *pPlayer){
    // pPlayer->angle+=5;
}

void setSpeed(bool up,bool down,bool left,bool right,bool *pGoUp,bool *pGoDown,bool *pGoLeft,bool *pGoRight
    ,int *pUpCounter, bool onGround,Player *pPlayer,int speedX,int speedY)
{
    pPlayer->vx = pPlayer->vy = 0;
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
    if(left && !right)
    {
        pPlayer->vx = -(speedX);
        (*pGoLeft) = 1;
    } 
    if(right && !left) 
    {
        pPlayer->vx = speedX;
        (*pGoRight) = 1;
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
        if (gameMap[(((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h][((int)pPlayer->x)/blockRect.w] == 1)  // Bottom edge blocked on left?
        {
            pPlayer->x -= (pPlayer->vx * 5 * deltaTime);         //Dont move
        }
        else if (gameMap[((int)pPlayer->y - 2)/blockRect.h][((int)pPlayer->x)/blockRect.w] == 1)     // Top edge blocked on left?
        {
            pPlayer->x -= (pPlayer->vx * 5 * deltaTime);         //Dont move
        }
    }
    if ((*pGoRight) == 1)
    {
        if(gameMap[(((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h][(((int)pPlayer->x) + pPlayer->playerRect.w) / blockRect.w] == 1 || // Bottom edge blocked on right?
        gameMap[((int)pPlayer->y)/blockRect.h][(((int)pPlayer->x) + pPlayer->playerRect.w) / blockRect.w] == 1)  // Top edge blocked on right?
        {
            pPlayer->x -= (pPlayer->vx * 5 * deltaTime);         //Dont move
        }
    }
    if ((*pGoUp) == 1)
    {
        if (gameMap[((int)pPlayer->y)/blockRect.h][((int)pPlayer->x)/blockRect.w] == 1 ||   // Left edge blocked on top?
        gameMap[((int)pPlayer->y)/blockRect.h][((int)pPlayer->x + pPlayer->playerRect.w)/blockRect.w] == 1) // Right edge blocked on top?
        {
            pPlayer->y -= (pPlayer->vy * deltaTime);             //Dont move
            (*pUpCounter) = 0;
        }
    }
    if ((*pGoDown) == 1)
    {
        if (gameMap[(((int)pPlayer->y - 3) + pPlayer->playerRect.h)/blockRect.h][((int)pPlayer->x)/blockRect.w] == 1 || // Left edge blocked on bottom?
        gameMap[(((int)pPlayer->y - 3) + pPlayer->playerRect.h)/blockRect.h][((int)pPlayer->x + pPlayer->playerRect.w)/blockRect.w] == 1)  // Right edge blocked on bottom?
        {
            pPlayer->y -= (pPlayer->vy * deltaTime);             //Dont move
            (*pOnGround) = true;

        }
    }
    if (gameMap[(((int)pPlayer->y + 3) + pPlayer->playerRect.h)/blockRect.h][((int)pPlayer->x )/blockRect.w] == 0 && // Left edge blocked on bottom?
        gameMap[(((int)pPlayer->y + 3) + pPlayer->playerRect.h)/blockRect.h][((int)pPlayer->x + pPlayer->playerRect.w)/blockRect.w] == 0)  // Right edge blocked on bottom?
    {
        (*pOnGround) = false;
    }
    pPlayer->playerRect.x = pPlayer->x;
    pPlayer->playerRect.y = pPlayer->y - 3;
}

void drawPlayer(Player *pPlayer){
    SDL_RenderCopy(pPlayer->pRenderer,pPlayer->pTexture,NULL,&pPlayer->playerRect);
}

void drawPlayer(Player *pPlayer) {
    //SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &pPlayer->dstRect, 0, NULL, SDL_FLIP_HORIZONTAL); <-- spegelvänd
    
    SDL_RenderCopy(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &pPlayer->dstRect);
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

int collidePlayer(Player *pPlayer, SDL_Rect rect) {
    //return SDL_HasIntersection(&(pPlayer->playerRect),&rect);
    return distance(pPlayer->dstRect.x+pPlayer->dstRect.w/2,pPlayer->dstRect.y+pPlayer->dstRect.h/2,rect.x+rect.w/2,rect.y+rect.h/2)<(pPlayer->dstRect.w+rect.w)/2;
}

static float distance(int x1, int y1, int x2, int y2) {
    return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}