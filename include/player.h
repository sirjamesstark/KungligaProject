#ifndef rocket_h
#define rocket_h

#include <stdbool.h>

#define BOX_ROW 22
#define BOX_COL 26
#define COUNTER 20

typedef struct player Player;
typedef struct frames Frames;

Player *createPlayer(SDL_Rect blockRect, SDL_Renderer *pRenderer, int window_width, int window_height);
void updatePlayer(Player *pPlayer,float deltaTime,int gameMap[BOX_ROW][BOX_COL],SDL_Rect blockRect,int *pUpCounter,bool *pOnGround);
void drawPlayer(Player *pRocket);
void destroyPlayer(Player *pRocket);
void setSpeed(bool up,bool down,bool left,bool right,int *pUpCounter,
    bool onGround,Player *pPlayer,int speedX,int speedY);


void turnLeft(Player *pRocket);
void turnRight(Player *pRocket);
int collidePlayer(Player *pRocket, SDL_Rect rect);

#endif