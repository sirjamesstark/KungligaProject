#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_net.h>
#include <math.h>
#include <stdbool.h>
#include "../include/player.h"
#include "../include/scaling.h"

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
    float x, y, vx, vy, oldX, oldY, targetX, targetY;
    Frames frames;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect; 
    SDL_Rect dstRect; 
    bool active;
    bool alive;
    IPaddress clientAddress;
};

void setIpAddress(Player *pPlayer, IPaddress address)
{
    pPlayer->clientAddress = address;
}

void setActive(Player *pPlayer, bool condition)
{
    pPlayer->active = condition;
}

int getPlayerActive(Player *pPlayer)
{
    return pPlayer->active;
}

int getAlive(Player *pPlayer)
{
    return pPlayer->alive;
}

void SetAlivefalse(Player *pPlayer)
{
    pPlayer->alive = false;
}

SDL_Rect *getPlayerRect(Player *pPly)
{
    return &(pPly->srcRect);
}

int getPlyRectX(Player *pPlayer)
{
    return pPlayer->dstRect.x;
}

float getPlyX(Player *pPlayer)
{
    return pPlayer->x;
}

float getPlyY(Player *pPlayer)
{
    return pPlayer->y;
}

Player *createPlayer(int player_ID, SDL_Renderer *pRenderer, SDL_Rect *pScreenRect)
{
    if (!pRenderer || !pScreenRect)
    {
        printf("Error in createPlayer: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Player *pPlayer = malloc(sizeof(struct player));
    if (!pPlayer)
    {
        printf("Error in createPlayer: Failed to allocate memory for pPlayer.\n");
        return NULL;
    }

    SDL_Surface *pSurface = initPlayerFrames(pPlayer, player_ID);
    if (!pSurface)
    {
        printf("Error in createPlayer: pSurface is NULL.\n");
        destroyPlayer(pPlayer);
        return NULL;
    }

    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    pPlayer->pScreenRect = pScreenRect;

    if (!pPlayer->pTexture)
    {
        printf("Error in createPlayer: pPlayer->pTexture is NULL.\n");
        destroyPlayer(pPlayer);
        return NULL;
    }
    pPlayer->alive = true;
    pPlayer->active = false;
    SDL_QueryTexture(pPlayer->pTexture, NULL, NULL, &pPlayer->srcRect.w, &pPlayer->srcRect.h);
    pPlayer->srcRect.w = pPlayer->srcRect.h = pPlayer->srcRect.h / 3;
    pPlayer->srcRect.x = pPlayer->srcRect.y = 0;
    printf("\nPlayer size:\n");
    pPlayer->dstRect = scaleRect(pPlayer->srcRect, *pPlayer->pScreenRect, PLAYER_SCALEFACTOR);
    pPlayer->frames.characterRect = scaleRect(pPlayer->frames.characterRect, *pPlayer->pScreenRect, PLAYER_SCALEFACTOR);
    printf("Player %d alive = %d\n", player_ID, pPlayer->alive);
    return pPlayer;
}

SDL_Surface *initPlayerFrames(Player *pPlayer, int player_ID)
{
    pPlayer->frames.currentFrame_x = pPlayer->frames.currentFrame_y = 0;
    pPlayer->frames.is_mirrored = false;
    pPlayer->frames.frameDelay = 200;
    pPlayer->frames.lastFrameTime = SDL_GetTicks();

    switch (player_ID)
    {
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

void initStartPosition(Player *pPlayer, SDL_Rect blockRect)
{
    pPlayer->oldX = pPlayer->x = blockRect.w * 5;
    //pPlayer->oldY = pPlayer->y = BOX_SCREEN_Y * blockRect.h - blockRect.h * 2 - pPlayer->dstRect.h;
        // För single player test...
    pPlayer->oldY = pPlayer->y = 0;
}

void setSpeed(Player *pPlayer, Movecheck *movecheck)
{
    int speedX = pPlayer->pScreenRect->w / 20;
    int speedY = pPlayer->pScreenRect->h / 20;

    pPlayer->vx = pPlayer->vy = 0;
    pPlayer->frames.currentFrame_y = 0;

    if (movecheck->up && !movecheck->down && movecheck->onGround)
    {
        (movecheck->pUpCounter) = COUNTER;
    }

    if ((movecheck->pUpCounter) > 0)
    {
        (movecheck->pGoUp) = 1;
        pPlayer->vy = -(speedY * 5);
        (movecheck->pUpCounter)--;
    }
    else if (!movecheck->onGround)
    {
        (movecheck->pGoDown) = 1;
        pPlayer->vy = (speedY) * 5;
    }

    if (movecheck->left && !movecheck->right)
    {
        pPlayer->vx = -(speedX);
        (movecheck->pGoLeft) = 1;
        pPlayer->frames.is_mirrored = true;
        if (!movecheck->onGround || (movecheck->pUpCounter) > 0)
            pPlayer->frames.currentFrame_y = 2;
        else
            pPlayer->frames.currentFrame_y = 1;
    }
    else if (movecheck->right && !movecheck->left)
    {
        pPlayer->vx = speedX;
        (movecheck->pGoRight) = 1;
        pPlayer->frames.is_mirrored = false;
        if (!movecheck->onGround || (movecheck->pUpCounter) > 0)
            pPlayer->frames.currentFrame_y = 2;
        else
            pPlayer->frames.currentFrame_y = 1;
    }
}

void setAnimation(Player *pPlayer)
{
    if (pPlayer->oldX > pPlayer->dstRect.x) // going left
    {
        pPlayer->frames.is_mirrored = true;

        if (pPlayer->oldY < pPlayer->dstRect.y) // and up
        {
            pPlayer->frames.currentFrame_y = 2;
        }
        else // not going up
        {
            pPlayer->frames.currentFrame_y = 1;
        }
    }
    else if (pPlayer->oldX < pPlayer->dstRect.x) // going right
    {
        pPlayer->frames.is_mirrored = false;

        if (pPlayer->oldY < pPlayer->dstRect.y) // and up
        {
            pPlayer->frames.currentFrame_y = 2;
        }
        else // not going up
        {
            pPlayer->frames.currentFrame_y = 1;
        }
    }
    else // idel
    {
        pPlayer->frames.currentFrame_y = 0;
    }

    pPlayer->oldX = pPlayer->dstRect.x;
    pPlayer->oldY = pPlayer->dstRect.y;
}

Offsets setOffsets(SDL_Rect screenRect, float shiftY, float shiftX)
{
    Offsets offset = {0};
    offset.top = (screenRect.h / TOP_OFFSETSCALER) - shiftY;
    offset.bot = (screenRect.h / BOT_OFFSETSCALER) - shiftY;
    offset.gravity = (screenRect.h / GRAVITY_OFFSETSCALER) - shiftY;
    offset.left = screenRect.w / LEFT_OFFSETSCALER - shiftX;
    offset.right = screenRect.w / RIGHT_OFFSETSCALER - shiftX;
    return offset;
}

void updatePlayer(Player *pPlayer[MAX_NROFPLAYERS], Offsets offset, int my_id, float deltaTime, int gameMap[BOX_ROW][BOX_COL], SDL_Rect blockRect,
                  Movecheck *movecheck)
{
    pPlayer[my_id]->x += pPlayer[my_id]->vx * 5 * deltaTime;
    float checkY = pPlayer[my_id]->y += pPlayer[my_id]->vy * deltaTime;
    while (checkY < 0)
    {
        checkY += blockRect.h * BOX_SCREEN_Y;
    }

    float lerpSpeed = 0.2f; 
    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        if (i != my_id)
        {
            pPlayer[i]->x += (pPlayer[i]->targetX - pPlayer[i]->x) * lerpSpeed;
            pPlayer[i]->y += (pPlayer[i]->targetY - pPlayer[i]->y) * lerpSpeed;
        }
    }

    // Check Collision
    if ((movecheck->pGoLeft) != 0)
    {
        if (gameMap[(int)(checkY + offset.bot + pPlayer[my_id]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[my_id]->x + offset.left) / blockRect.w] != 0) // Bottom edge blocked on left?
        {
            pPlayer[my_id]->x -= (pPlayer[my_id]->vx * 5 * deltaTime); // Dont move
        }
        else if (gameMap[(int)(checkY + offset.top) / blockRect.h][(int)(pPlayer[my_id]->x + offset.left) / blockRect.w] != 0) // Top edge blocked on left?
        {
            pPlayer[my_id]->x -= (pPlayer[my_id]->vx * 5 * deltaTime); // Dont move
        }
    }

    if ((movecheck->pGoRight) != 0)
    {
        if (gameMap[(int)(checkY + offset.bot + pPlayer[my_id]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[my_id]->x + offset.right + pPlayer[my_id]->frames.characterRect.w) / blockRect.w] != 0 || // Bottom edge blocked on right?
            gameMap[(int)(checkY + offset.top) / blockRect.h][(int)(pPlayer[my_id]->x + offset.right + pPlayer[my_id]->frames.characterRect.w) / blockRect.w] != 0)                                            // Top edge blocked on right?
        {
            pPlayer[my_id]->x -= (pPlayer[my_id]->vx * 5 * deltaTime); // Dont move
        }
    }

    if ((movecheck->pGoUp) != 0)
    {
        if (gameMap[(int)(checkY + offset.top) / blockRect.h][(int)(pPlayer[my_id]->x + offset.left) / blockRect.w] != 0 ||                                         // Left edge blocked on top?
            gameMap[(int)(checkY + offset.top) / blockRect.h][(int)(pPlayer[my_id]->x + offset.right + pPlayer[my_id]->frames.characterRect.w) / blockRect.w] != 0) // Right edge blocked on top?
        {

            pPlayer[my_id]->y -= (pPlayer[my_id]->vy * deltaTime); // Dont move
            (movecheck->pUpCounter) = 0;
        }
    }

    if ((movecheck->pGoDown) != 0)
    {
        if (gameMap[(int)(checkY + offset.bot + pPlayer[my_id]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[my_id]->x + offset.left) / blockRect.w] != 0 ||                                         // Left edge blocked on bottom?
            gameMap[(int)(checkY + offset.bot + pPlayer[my_id]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[my_id]->x + offset.right + pPlayer[my_id]->frames.characterRect.w) / blockRect.w] != 0) // Right edge blocked on bottom?
        {
            pPlayer[my_id]->y -= (pPlayer[my_id]->vy * deltaTime); // Dont move
            (movecheck->onGround) = true;
        }
    }
    if (gameMap[(int)(checkY + offset.gravity + pPlayer[my_id]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[my_id]->x + offset.left) / blockRect.w] == 0 &&                                         // Left edge blocked on bottom?
        gameMap[(int)(checkY + offset.gravity + pPlayer[my_id]->frames.characterRect.h) / blockRect.h][(int)(pPlayer[my_id]->x + offset.right + pPlayer[my_id]->frames.characterRect.w) / blockRect.w] == 0) // Right edge blocked on bottom?
    {
        (movecheck->onGround) = false;
    }
    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        pPlayer[i]->dstRect.x = pPlayer[i]->x;
        pPlayer[i]->dstRect.y = pPlayer[i]->y;
        syncCharacterRect(pPlayer[i]);
    }
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
        pPlayer->frames.currentFrame_x = (pPlayer->frames.currentFrame_x + 1) % pPlayer->frames.nrOfFrames_idle;
    }
    else if (pPlayer->frames.currentFrame_y == 1)
    {
        pPlayer->frames.currentFrame_x = (pPlayer->frames.currentFrame_x + 1) % pPlayer->frames.nrOfFrames_sprint;
    }
    else if (pPlayer->frames.currentFrame_y == 2)
    {
        pPlayer->frames.currentFrame_x = (pPlayer->frames.currentFrame_x + 1) % pPlayer->frames.nrOfFrames_jump;
    }

    pPlayer->srcRect.x = pPlayer->frames.currentFrame_x * pPlayer->srcRect.w;
    pPlayer->srcRect.y = pPlayer->frames.currentFrame_y * pPlayer->srcRect.h;
}

void networkUDP(Player *pPlayer[MAX_NROFPLAYERS], int my_id, UDPpacket *sendPacket, UDPpacket *receivePacket, int is_server,
                IPaddress srvadd, UDPsocket sd, int *pNrOfPlayers)
{
    static int lastSentTime = 0;
    int now = SDL_GetTicks();
    if ((now - lastSentTime) > 50 && (pPlayer[my_id]->oldX != pPlayer[my_id]->x || pPlayer[my_id]->oldY != pPlayer[my_id]->y))
    {
        snprintf((char *)sendPacket->data, 512, "%d %f %f", my_id, pPlayer[my_id]->x / pPlayer[my_id]->pScreenRect->w,
                 (pPlayer[my_id]->pScreenRect->h - pPlayer[my_id]->y) / pPlayer[my_id]->pScreenRect->h);
        sendPacket->len = strlen((char *)sendPacket->data) + 1;

        if (is_server)
        {
            for (int i = 1; i < *pNrOfPlayers; i++)
            {
                if (pPlayer[i]->active)
                {
                    sendPacket->address = pPlayer[i]->clientAddress;
                    SDLNet_UDP_Send(sd, -1, sendPacket);
                }
            }
        }
        else
        {
            sendPacket->address = pPlayer[0]->clientAddress;
            SDLNet_UDP_Send(sd, -1, sendPacket);
        }
        pPlayer[my_id]->oldX = pPlayer[my_id]->x;
        pPlayer[my_id]->oldY = pPlayer[my_id]->y;
    }
    if (SDLNet_UDP_Recv(sd, receivePacket))
    {
        if (is_server)
        {
            if (strncmp((char *)receivePacket->data, "JOIN", 4) == 0)
            {
                if (*pNrOfPlayers < MAX_NROFPLAYERS)
                {
                    int newID = *pNrOfPlayers;
                    pPlayer[newID]->active = true;
                    pPlayer[newID]->clientAddress = receivePacket->address;

                    sprintf((char *)sendPacket->data, "WELCOME %d", newID);
                    sendPacket->len = strlen((char *)sendPacket->data) + 1;
                    sendPacket->address = receivePacket->address;
                    SDLNet_UDP_Send(sd, -1, sendPacket);

                    printf("Server: Accepted JOIN, assigned ID %d\n", newID);

                    for (int i = 0; i < *pNrOfPlayers; i++) 
                    {
                        if (pPlayer[i]->active)
                        {
                            snprintf((char *)sendPacket->data, 512, "%d %f %f", i, pPlayer[i]->x / pPlayer[i]->pScreenRect->w,
                                     (pPlayer[i]->pScreenRect->h - pPlayer[i]->y) / pPlayer[i]->pScreenRect->h);
                            sendPacket->len = strlen((char *)sendPacket->data) + 1;
                            sendPacket->address = pPlayer[newID]->clientAddress;
                            SDLNet_UDP_Send(sd, -1, sendPacket);
                        }
                    }
                    (*pNrOfPlayers)++;
                }
                else
                {
                    printf("Server: JOIN refused, server full.\n");
                }
            }
            else
            {
                int id;
                float client_x, client_y;
                if (sscanf((char *)receivePacket->data, "%d %f %f", &id, &client_x, &client_y) == 3)
                {
                    if (id >= 0 && id < MAX_NROFPLAYERS && pPlayer[id]->active)
                    {
                        pPlayer[id]->oldX = pPlayer[id]->x;
                        pPlayer[id]->oldY = pPlayer[id]->y;
                        pPlayer[id]->targetX = client_x * pPlayer[id]->pScreenRect->w;
                        pPlayer[id]->targetY = pPlayer[id]->pScreenRect->h - client_y * pPlayer[id]->pScreenRect->h;
                    }
                    for (int i = 1; i < (*pNrOfPlayers); i++)
                    {
                        if (pPlayer[i]->active && i != id)
                        {
                            snprintf((char *)sendPacket->data, 512, "%d %f %f", id, client_x, client_y);
                            sendPacket->len = strlen((char *)sendPacket->data) + 1;
                            sendPacket->address = pPlayer[i]->clientAddress;
                            SDLNet_UDP_Send(sd, -1, sendPacket);
                        }
                    }
                }
            }
        }
        else
        {
            int id;
            float x, y;
            if (sscanf((char *)receivePacket->data, "%d %f %f", &id, &x, &y) == 3)
            {
                if (id >= 0 && id < MAX_NROFPLAYERS && id != my_id)
                {
                    pPlayer[id]->targetX = x * pPlayer[id]->pScreenRect->w;
                    pPlayer[id]->targetY = pPlayer[id]->pScreenRect->h - y * pPlayer[id]->pScreenRect->h;
                    pPlayer[id]->active = true;
                }
            }
        }
    }
}

void drawPlayer(Player *pPlayer, int CamX, int CamY)
{
    if (pPlayer->active)
    {
        updatePlayerFrame(pPlayer);

        pPlayer->dstRect.x = (pPlayer->x - CamX);
        pPlayer->dstRect.y = (pPlayer->y - CamY + pPlayer->pScreenRect->y * 2);

        if (pPlayer->frames.is_mirrored == true)
        {
            SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &pPlayer->dstRect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        else
        {
            SDL_RenderCopy(pPlayer->pRenderer, pPlayer->pTexture, &pPlayer->srcRect, &pPlayer->dstRect);
        }
    }
}

void destroyPlayer(Player *pPlayer)
{
    if (!pPlayer)
        return;
    if (pPlayer->pTexture)
    {
        SDL_DestroyTexture(pPlayer->pTexture);
        pPlayer->pTexture = NULL;
    }
    free(pPlayer);
}
