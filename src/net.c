#include <SDL.h>
#include <SDL_net.h>
#include "../include/net.h"
#include <stdbool.h>

struct server
{
    UDPsocket sd;
    IPaddress serverAdd;
    UDPpacket *p, *p2;
}; typedef struct server Server;

Server *NET_INIT(bool is_server){
    Server *pServer = malloc(sizeof(Server));

    if (SDLNet_Init() < 0) {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (!(pServer->sd = SDLNet_UDP_Open(is_server ? 2000 : 0))) {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    return pServer;
}

void bindPort(bool is_server, Server *pServer){
    if (!(pServer->sd = SDLNet_UDP_Open(is_server ? 2000 : 0))) {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }
}

void setSrvAdd_client(char **args, int argv, Server *pSrv){
    if (argv < 3) {
        fprintf(stderr, "Usage: %s client <server_ip>\n", args[0]);
        exit(EXIT_FAILURE);
    }
    
    if (SDLNet_ResolveHost(&(pSrv->serverAdd), args[2], 2000) == -1) {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

}

void setPacketSize(Server *pSrv){
    if (!((pSrv->p = SDLNet_AllocPacket(512)) && (pSrv->p2 = SDLNet_AllocPacket(512)))) {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }
}

void sendPaket(int x, int y, Server *pSrv, bool is_server){
    sprintf((char*)pSrv->p->data, "%d %d", (int)x, (int)y);
    pSrv->p->len = strlen((char*)pSrv->p->data) + 1;

    if (!is_server) {
        pSrv->p->address.host = pSrv->serverAdd.host;
        pSrv->p->address.port = pSrv->serverAdd.port;
    }

    SDLNet_UDP_Send(pSrv->sd, -1, pSrv->p);
}

void recivePaket(Server *pSrv, bool is_server, SDL_Rect *pDest, SDL_Rect *pSrvEco){
    if (SDLNet_UDP_Recv(pSrv->sd, pSrv->p2)) {
        int a, b;
        sscanf((char*)pSrv->p2->data, "%d %d", &a, &b);
        pDest->x = a;
        pDest->y = b;

        if (is_server) {
            sprintf((char*)pSrv->p->data, "%d %d", pSrvEco->x, pSrvEco->y);
            pSrv->p->address = pSrv->p2->address;
            pSrv->p->len = strlen((char*)pSrv->p->data) + 1;
            SDLNet_UDP_Send(pSrv->sd, -1, pSrv->p);
        }
    }
}

IPaddress getServerAdd(Server *pSrv){
    return pSrv->serverAdd;
}

void NET_Quit(Server *pSrv){
    SDLNet_FreePacket(pSrv->p);
    SDLNet_FreePacket(pSrv->p2);
    SDLNet_UDP_Close(pSrv->sd);
    SDLNet_Quit();
}

/*
struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
    // AsteroidImage *pAsteroidImage;
    // Asteroid *pAsteroids[MAX_ASTEROIDS];

    bool is_server;
    Server *pServer;
};
typedef struct game Game;


    if (argv > 1 && strcmp(args[1], "server") == 0) { // UDP prototyp
        pGame->is_server = true; 
    }

    pGame->pServer = NET_INIT(pGame->is_server);

    bindPort(pGame->is_server, pGame->pServer);

    if (!pGame->is_server) setSrvAdd_client(args, argv, pGame->pServer);

*/