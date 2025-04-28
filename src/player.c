#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_net.h>
#include <math.h>
#include <stdbool.h>
#include "../include/player.h"

struct frames
{
    int nrOfFrames_idle, nrOfFrames_sprint, nrOfFrames_jump;
    int currentFrame_x, currentFrame_y;
    SDL_Rect characterRect;
    bool is_mirrored;
    int frameDelay;
    Uint32 lastFrameTime;
};

struct player
{
    float x, y, vx, vy, oldX, oldY,targetX,targetY;
    Frames frames;
    SDL_Renderer *pRenderer;
    SDL_Rect *pGameAreaRect;
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

Player *createPlayer(int player_ID, SDL_Renderer *pRenderer, SDL_Rect *pGameAreaRect)
{
    if (!pRenderer || !pGameAreaRect) {
        printf("Error: Invalid parameters. Renderer or GameAreaRect is NULL.\n");
        return NULL;
    }

    Player *pPlayer = malloc(sizeof(struct player));
    if (pPlayer == NULL) {
        printf("Error: Failed to allocate memory for player.\n");
        return NULL;
    }

    pPlayer->pRenderer = pRenderer;
    pPlayer->pGameAreaRect = pGameAreaRect;

    SDL_Surface *pSurface = initPlayerFrames(pPlayer, player_ID);
    if (!pSurface) {
        destroyPlayer(pPlayer);
        printf("Error: Failed to initialize player frames.\n");
        return NULL;
    }

    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pPlayer->pTexture) {
        destroyPlayer(pPlayer);
        printf("Error: Failed to create player texture.\n");
        return NULL;
    }

    SDL_QueryTexture(pPlayer->pTexture, NULL, NULL, &pPlayer->srcRect.w, &pPlayer->srcRect.h);
    pPlayer->srcRect.w = pPlayer->srcRect.h = pPlayer->srcRect.h/3; 
    pPlayer->srcRect.x = (pPlayer->frames.currentFrame_x) * pPlayer->srcRect.w;
    pPlayer->srcRect.y = (pPlayer->frames.currentFrame_y) * pPlayer->srcRect.h;
    printf("Player frame size (before scaling): w: %d, h: %d\n", pPlayer->srcRect.w, pPlayer->srcRect.h);

    float scaleFactor = (float)pPlayer->pGameAreaRect->w / (float)pPlayer->srcRect.w * PLAYER_SCALEFACTOR;
    pPlayer->dstRect.w = (int)(pPlayer->srcRect.w * scaleFactor + 0.5f);    
    pPlayer->dstRect.h = (int)(pPlayer->srcRect.h * scaleFactor + 0.5f);
    printf("Player frame size (after scaling): w: %d, h: %d\n", pPlayer->dstRect.w, pPlayer->dstRect.h);

    printf("Player character size (before scaling): w: %d, h: %d\n", pPlayer->frames.characterRect.w, pPlayer->frames.characterRect.h);
    pPlayer->frames.characterRect.w = (int)(pPlayer->frames.characterRect.w * scaleFactor + 0.5f);
    pPlayer->frames.characterRect.h = (int)(pPlayer->frames.characterRect.h * scaleFactor + 0.5f);
    printf("Player character size (after scaling): w: %d, h: %d\n", pPlayer->frames.characterRect.w, pPlayer->frames.characterRect.h);  

    // pPlayer->dstRect.w = ((pPlayer->window_width)/BOX_COL);
    // pPlayer->dstRect.h = ((pPlayer->window_height)/BOX_ROW);

    // pPlayer->dstRect.w = (pPlayer->srcRect.w) <-- multiplicera med skalfaktor
    // pPlayer->dstRect.h = (pPlayer->srcRect.h) <-- multiplicera med skalfaktor
    // pPlayer->dstRect.w = pPlayer->window_width/pPlayer->srcRect.w;
    // pPlayer->dstRect.x = pPlayer->x - (pPlayer->dstRect.w/2); // Anger koordinaterna för gubbens placering på skärmen vänster i övre hörn
    // pPlayer->dstRect.y = pPlayer->y - (pPlayer->dstRect.h/2);
    return pPlayer;
}

SDL_Surface *initPlayerFrames(Player *pPlayer, int player_ID) 
{
    pPlayer->frames.currentFrame_x = pPlayer->frames.currentFrame_y = 0;
    pPlayer->frames.is_mirrored = false;
    pPlayer->frames.frameDelay = 200; // 100 ms = 10 frames per sekund
    pPlayer->frames.lastFrameTime = SDL_GetTicks();

    switch (player_ID) {
        case 0:
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 11;
            pPlayer->frames.characterRect.w = 50;
            pPlayer->frames.characterRect.h = 75;
            return IMG_Load("resources/player_0.png");
        case 1:
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 8;
            pPlayer->frames.characterRect.w = 60;
            pPlayer->frames.characterRect.h = 70;
            return IMG_Load("resources/player_1.png");
        case 2:
            pPlayer->frames.nrOfFrames_idle = 5;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 7;
            pPlayer->frames.characterRect.w = 50;
            pPlayer->frames.characterRect.h = 70;
            return IMG_Load("resources/player_2.png");
        case 3:
            pPlayer->frames.nrOfFrames_idle = 8;
            pPlayer->frames.nrOfFrames_sprint = 8;
            pPlayer->frames.nrOfFrames_jump = 8;
            pPlayer->frames.characterRect.w = 50;
            pPlayer->frames.characterRect.h = 71;
            return IMG_Load("resources/player_3.png");
        default:
            return NULL;
    }
}

void initStartPosition(Player *pPlayer, SDL_Rect blockRect) {
    pPlayer->oldX = pPlayer->x = blockRect.w * 2;
    pPlayer->oldY = pPlayer->y = BOX_SCREEN_Y * blockRect.h - blockRect.h * 2 - pPlayer->dstRect.h;
    // pPlayer->oldY = pPlayer->y = 0;
    /*
    pPlayer->dstRect.x = (float)(pPlayer->pGameAreaRect->x + blockRect.w / 2);  // Börjar i mitten av ett block i x-led
    pPlayer->dstRect.y = (float)(pPlayer->pGameAreaRect->y + pPlayer->pGameAreaRect->h - blockRect.h - pPlayer->frames.characterRect.h);
    printf("pPlayer->dstRect: x=%d, y=%d\n", pPlayer->dstRect.x, pPlayer->dstRect.y);

    pPlayer->frames.characterRect.x = pPlayer->dstRect.x + (int)(((float)(pPlayer->dstRect.w - pPlayer->frames.characterRect.w) / 2.0f) + 0.5f);
    pPlayer->frames.characterRect.y = pPlayer->dstRect.y + (int)(float)(pPlayer->dstRect.h - pPlayer->frames.characterRect.h + 0.5f);
    printf("pPlayer->frames.characterRect: x=%d, y=%d\n", pPlayer->frames.characterRect.x, pPlayer->frames.characterRect.y);
    */
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
    int speedX = pPlayer->pGameAreaRect->w / 20; 
    int speedY = pPlayer->pGameAreaRect->h / 20;

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
                  UDPpacket *p2, int *pIs_server, IPaddress srvadd, UDPsocket *pSd, int window_height)
{
    float space = pPlayer[0]->pGameAreaRect->h - blockRect.h * (BOX_SCREEN_Y), checkY;
    Offsets offset = {(window_height/TOP_OFFSETSCALER) - space,window_height/BOT_OFFSETSCALER - space,window_height/GRAVITY_OFFSETSCALER - space};
    pPlayer[0]->active = true;
    pPlayer[0]->x += pPlayer[0]->vx * 5 * deltaTime;
    // pPlayer[0]->y += pPlayer[0]->vy * deltaTime;
    checkY = pPlayer[0]->y += pPlayer[0]->vy * deltaTime;
    while (checkY < 0)
    {
        checkY += blockRect.h * BOX_SCREEN_Y;
    }
    
    networkUDP(pPlayer, p, p2, pIs_server, srvadd, pSd, space);
    float lerpSpeed = 0.2f; // Testa mellan 0.1 och 0.2
    pPlayer[1]->x += (pPlayer[1]->targetX - pPlayer[1]->x) * lerpSpeed;
    pPlayer[1]->y += (pPlayer[1]->targetY - pPlayer[1]->y) * lerpSpeed;

    // Check Collision
    if ((*pGoLeft) != 0)
    {
        // printf("y: %d\n", (((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h);
        // printf("x: %d\n", ((int)pPlayer->x)/blockRect.w);
        if (gameMap[(int)(checkY + offset.bot + pPlayer[0]->frames.characterRect.h) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0) // Bottom edge blocked on left?
        {
            pPlayer[0]->x -= (pPlayer[0]->vx * 5 * deltaTime); // Dont move
        }
        else if (gameMap[(int)(checkY + offset.top) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0) // Top edge blocked on left?
        {
            pPlayer[0]->x -= (pPlayer[0]->vx * 5 * deltaTime); // Dont move
        }
    }

    if ((*pGoRight) != 0)
    {
        // printf("y: %d\n", (((int)pPlayer->y - 4) + pPlayer->playerRect.h)/blockRect.h);
        // printf("x: %d\n", (((int)pPlayer->x) + pPlayer->playerRect.w) / blockRect.w);
        if (gameMap[(int)(checkY + offset.bot + pPlayer[0]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.characterRect.w) / blockRect.w] != 0 || // Bottom edge blocked on right?
            gameMap[(int)(checkY + offset.top) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.characterRect.w) / blockRect.w] != 0)                                    // Top edge blocked on right?
        {
            pPlayer[0]->x -= (pPlayer[0]->vx * 5 * deltaTime); // Dont move
        }
    }

    if ((*pGoUp) != 0)
    {
        // printf("y: %d,\n", ((int)pPlayer->y + 1)/blockRect.h);
        // printf("x: %d,\n", ((int)pPlayer->x + 1)/blockRect.w);
        if (gameMap[(int)(checkY + offset.top) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0 ||                                // Left edge blocked on top?
            gameMap[(int)(checkY + offset.top) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.characterRect.w) / blockRect.w] != 0) // Right edge blocked on top?
        {

            pPlayer[0]->y -= (pPlayer[0]->vy * deltaTime); // Dont move
            (*pUpCounter) = 0;
        }
    }

    if ((*pGoDown) != 0)
    {
        if (gameMap[(int)(checkY + offset.bot + pPlayer[0]->frames.characterRect.h) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] != 0 ||                                // Left edge blocked on bottom?
            gameMap[(int)(checkY + offset.bot + pPlayer[0]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.characterRect.w) / blockRect.w] != 0) // Right edge blocked on bottom?
        {
            pPlayer[0]->y -= (pPlayer[0]->vy * deltaTime); // Dont move
            (*pOnGround) = true;
        }
    }
    if (gameMap[(int)(checkY + offset.gravity + pPlayer[0]->frames.characterRect.h) / blockRect.h][((int)pPlayer[0]->x + 25) / blockRect.w] == 0 &&                                // Left edge blocked on bottom?
        gameMap[(int)(checkY + offset.gravity + pPlayer[0]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[0]->x + 10 + pPlayer[0]->frames.characterRect.w) / blockRect.w] == 0) // Right edge blocked on bottom?
    {
        (*pOnGround) = false;   
    }
    pPlayer[0]->dstRect.x = pPlayer[0]->x;
    pPlayer[0]->dstRect.y = pPlayer[0]->y;
    syncCharacterRect(pPlayer[0]);
    pPlayer[1]->dstRect.x = pPlayer[1]->x;
    pPlayer[1]->dstRect.y = pPlayer[1]->y;
    syncCharacterRect(pPlayer[1]);
    if (pPlayer[0]->x < 0)
    {
        pPlayer[0]->x = 0; // gör så att man inte kan falla ned i vänster hörnet
    }


    /* BEHÖVER UNDERSÖKA SPELETS LOGIK INNAN NEDANSTÅENDE KOD IMPLEMENTERAS
    pPlayer[0]->frames.characterRect.x = pPlayer[0]->dstRect.x + (pPlayer[0]->dstRect.w - pPlayer[0]->frames.characterRect.w) / 2; 
    pPlayer[0]->frames.characterRect.y = pPlayer[0]->dstRect.y + (pPlayer[0]->dstRect.h - pPlayer[0]->frames.characterRect.h); 

    // Kontrollerar så att spelaren inte kan röra sig utanför "spelets bana"
    if (pPlayer[0]->frames.characterRect.x <= pPlayer[0]->pGameAreaRect->x) {
        pPlayer[0]->frames.characterRect.x = pPlayer[0]->pGameAreaRect->x;
    }
    else if (pPlayer[0]->frames.characterRect.x >= (pPlayer[0]->pGameAreaRect->x + pPlayer[0]->pGameAreaRect->w)) {
        pPlayer[0]->frames.characterRect.x = (pPlayer[0]->pGameAreaRect->x + pPlayer[0]->pGameAreaRect->w);
    }

    if (pPlayer[0]->frames.characterRect.y <= pPlayer[0]->pGameAreaRect->y) {
        pPlayer[0]->frames.characterRect.x = pPlayer[0]->pGameAreaRect->y;
    }
    else if (pPlayer[0]->frames.characterRect.y >= (pPlayer[0]->pGameAreaRect->y + pPlayer[0]->pGameAreaRect->h)) {
        pPlayer[0]->frames.characterRect.x = (pPlayer[0]->pGameAreaRect->y + pPlayer[0]->pGameAreaRect->h);
    }
    pPlayer[0]->dstRect.x = pPlayer[0]->frames.characterRect.x - (pPlayer[0]->dstRect.w - pPlayer[0]->frames.characterRect.w) / 2; 
    pPlayer[0]->dstRect.y = pPlayer[0]->frames.characterRect.y - (pPlayer[0]->dstRect.h - pPlayer[0]->frames.characterRect.h);
    */
}

void syncCharacterRect(Player *pPlayer) 
{
    pPlayer->frames.characterRect.x = pPlayer->dstRect.x + (pPlayer->dstRect.w - pPlayer->frames.characterRect.w) / 2;
    pPlayer->frames.characterRect.y = pPlayer->dstRect.y + (pPlayer->dstRect.h - pPlayer->frames.characterRect.h);
}

void updatePlayerFrame(Player *pPlayer)
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
    static int lastSentTime = 0;
    int now = SDL_GetTicks();
    if ((now - lastSentTime) > 50 && (pPlayer[0]->oldX != pPlayer[0]->x || pPlayer[0]->oldY != pPlayer[0]->y))
    {
        sprintf((char *)p->data, "%f %f", pPlayer[0]->x / pPlayer[0]->pGameAreaRect->w, (pPlayer[0]->y) / pPlayer[0]->pGameAreaRect->h);
        p->len = strlen((char *)p->data) + 1;

        if (!(*pIs_server))
        {
            p->address.host = srvadd.host;
            p->address.port = srvadd.port;
        }

        SDLNet_UDP_Send(*pSd, -1, p);
        pPlayer[0]->oldX = pPlayer[0]->x;
        pPlayer[0]->oldY = pPlayer[0]->y;
        lastSentTime = now;
    }
    if (SDLNet_UDP_Recv(*pSd, p2))
    {
        float a, b;
        sscanf((char *)p2->data, "%f %f", &a, &b);
        pPlayer[1]->targetX = a * pPlayer[1]->pGameAreaRect->w;
        pPlayer[1]->targetY = b * pPlayer[1]->pGameAreaRect->h;
        pPlayer[1]->active = true;
        if (*pIs_server)
        {
            sprintf((char *)p->data, "%f %f", pPlayer[0]->x / pPlayer[0]->pGameAreaRect->w, (pPlayer[0]->y) / pPlayer[0]->pGameAreaRect->h);
            p->address = p2->address;
            p->len = strlen((char *)p->data) + 1;
            SDLNet_UDP_Send(*pSd, -1, p);
        }
    }
}

void drawPlayer(Player *pPlayer, int CamX, int CamY)
{
    updatePlayerFrame(pPlayer);

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