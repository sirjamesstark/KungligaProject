#ifndef net_h
#define net_h

#include <stdbool.h>
#include <SDL.h>


typedef struct server Server;

Server *NET_INIT(bool is_server);
void bindPort(bool is_server, Server *pServer);
void setPacketSize(Server *pSrv);
void setSrvAdd_client(char **args, int argv, Server *pSrv);
void sendPaket(int x, int y, Server *pSrv, bool is_server);
void recivePaket(Server *pSrv, bool is_server, SDL_Rect *pDest, SDL_Rect *pSrvEco);
void NET_Quit(Server *pSrv);

#endif