#ifndef player_h
#define player_h

#define BOX_ROW 22
#define BOX_COL 26
#define COUNTER 20

typedef struct player Player;
typedef struct frames Frames;

Player *createPlayer(SDL_Rect blockRect, SDL_Renderer *pRenderer, int window_width, int window_height);
SDL_Rect *getPlayerRect(Player *pPly);
bool isSolidTile(Player pPlayer, int row, int col);
int getPlyX(Player *pPlayer);
int getPlyY(Player *pPlayer);
// void updatePlayer(Player *pPlayer, SDL_Rect blockRect);
void updatePlayer(Player *pPlayer, float deltaTime, int gameMap[BOX_ROW][BOX_COL], SDL_Rect blockRect, int *pUpCounter, bool *pOnGround, bool *pGoUp, bool *pGoDown, bool *pGoLeft, bool *pGoRight);
void updatePlayerRect(Player *pPlayer);
void drawPlayer(Player *pPlayer, int CamX, int CamY, int window_width, int window_height);

void destroyPlayer(Player *pPlayer);
void setSpeed(bool up, bool down, bool left, bool right, bool *pGoUp, bool *pGoDown, bool *pGoLeft, bool *pGoRight,
              int *pUpCounter, bool onGround, Player *pPlayer, int speedX, int speedY);

#endif