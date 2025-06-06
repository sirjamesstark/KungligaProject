#ifndef player_h
#define player_h

#include "../include/scaling.h"
#include <SDL_net.h>

#define BOX_ROW 140
#define BOX_COL 26
#define BOX_SCREEN_Y 14
#define COUNTER 20
#define MAX_NROFPLAYERS 4
#define PLAYER_SCALEFACTOR 0.1f
#define TOP_OFFSETSCALER 23.35f
#define BOT_OFFSETSCALER -432.0f
#define LEFT_OFFSETSCALER 41.5f
#define RIGHT_OFFSETSCALER -153.6f
#define GRAVITY_OFFSETSCALER 432.0f

struct movecheck
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool pGoUp;
    bool pGoDown;
    bool pGoLeft;
    bool pGoRight;
    int pUpCounter;
    bool onGround;
};

struct offsets
{
    float top, bot, left, right, gravity;
};
typedef struct offsets Offsets;
typedef struct player Player;
typedef struct frames Frames;
typedef struct movecheck Movecheck;

Player *createPlayer(int player_ID, SDL_Renderer *pRenderer, SDL_Rect *pScreenRect);
SDL_Surface *initPlayerFrames(Player *pPlayer, int player_ID);
void initStartPosition(Player *pPlayer, SDL_Rect blockRect);

SDL_Rect *getPlayerRect(Player *pPly);
int getPlayerActive(Player *pPlayer);
int getAlive(Player *pPlayer);
void SetAlivefalse(Player *pPlayer);
void setActive(Player *pPlayer, bool condition);
void setIpAddress(Player *pPlayer, IPaddress address);
Offsets setOffsets(SDL_Rect screenRect, float shiftY, float shiftX);
int getPlyRectX(Player *pPlayer);
float getPlyX(Player *pPlayer);
float getPlyY(Player *pPlayer);

void updatePlayer(Player *pPlayer[MAX_NROFPLAYERS], Offsets offset, int my_id, float deltaTime, int gameMap[BOX_ROW][BOX_COL], SDL_Rect blockRect,
                  Movecheck *movecheck);
void syncCharacterRect(Player *pPlayer);
void updatePlayerFrame(Player *pPlayer);
void networkUDP(Player *pPlayer[MAX_NROFPLAYERS], int my_id, UDPpacket *sendPacket, UDPpacket *receivePacket, int is_server,
                IPaddress srvadd, UDPsocket sd, int *pNrOfPlayers);
void drawPlayer(Player *pPlayer, int CamX, int CamY);
void setAnimation(Player *pPlayer);
void destroyPlayer(Player *pPlayer);
void setSpeed(Player *pPlayer, Movecheck *movecheck);

#endif