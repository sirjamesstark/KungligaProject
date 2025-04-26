#ifndef player_h
#define player_h

#include <SDL_net.h>

#define BOX_ROW 22
#define BOX_COL 26
#define COUNTER 20
#define MAX_NROFPLAYERS 2
#define PLAYER_SCALEFACTOR 0.04

typedef struct player Player;
typedef struct frames Frames;

Player *createPlayer(int player_ID, SDL_Rect blockRect, SDL_Renderer *pRenderer, int window_width, int window_height);
SDL_Surface *initPlayerFrames(Player *pPlayer, int player_ID);

SDL_Rect *getPlayerRect(Player *pPly);

int getPlyX(Player *pPlayer);
int getPlyY(Player *pPlayer);
// void updatePlayer(Player *pPlayer, SDL_Rect blockRect);

void updatePlayer(Player *pPlayer[MAX_NROFPLAYERS], float deltaTime, int gameMap[BOX_ROW][BOX_COL], SDL_Rect blockRect,
                  int *pUpCounter, bool *pOnGround, bool *pGoUp, bool *pGoDown, bool *pGoLeft, bool *pGoRight, UDPpacket *p,
                  UDPpacket *p2, int *pIs_server, IPaddress srvadd, UDPsocket *pSd);
void updatePlayerRect(Player *pPlayer);
void networkUDP (Player *pPlayer[MAX_NROFPLAYERS], UDPpacket *p, UDPpacket *p2, int *pIs_server, IPaddress srvadd, UDPsocket *pSd, float space);
void drawPlayer(Player *pPlayer, int CamX, int CamY, int window_width, int window_height);

void destroyPlayer(Player *pPlayer);
void setSpeed(bool up, bool down, bool left, bool right, bool *pGoUp, bool *pGoDown, bool *pGoLeft, bool *pGoRight,
                int *pUpCounter, bool onGround, Player *pPlayer);

#endif