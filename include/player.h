#ifndef rocket_h
#define rocket_h

typedef struct player Player;

Player *createPlayer(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height);
void updatePlayer(Player *pRocket);
void drawPlayer(Player *pRocket);
void destroyPlayer(Player *pRocket);
void turnLeft(Player *pRocket);
void turnRight(Player *pRocket);
void accelerate(Player *pRocket);
int collidePlayer(Player *pRocket, SDL_Rect rect);

#endif