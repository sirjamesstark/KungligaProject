#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_net.h>
#include <math.h>
#include <stdbool.h>
#include "../include/player.h"
#include "../include/net.h"

int initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **sendPacket, UDPpacket **receivePacket,
                int *is_server, int argc, char *argv[], int *pMyId)
{
    *is_server = 0;

    if (argc > 1 && strcmp(argv[1], "server") == 0)
    {
        *is_server = 1;
        *pMyId = 0;

        // Sätt upp serveradress (lokal maskin, port 2000)
        if (SDLNet_ResolveHost(srvadd, NULL, 2000) < 0)
        {
            fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            return -1;
        }
    }

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    *sd = NULL;
    *sendPacket = NULL;
    *receivePacket = NULL;

    if (!(*sd = SDLNet_UDP_Open(*is_server ? 2000 : 0)))
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    if (!(*is_server))
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: %s client <server_ip>\n", argv[0]);
            return 0;
        }

        if (SDLNet_ResolveHost(srvadd, argv[2], 2000) == -1)
        {
            fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            return 0;
        }
    }

    if (!((*sendPacket = SDLNet_AllocPacket(512)) && (*receivePacket = SDLNet_AllocPacket(512))))
    {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    return 1;
}

void cleanUpNetwork(UDPsocket *sd, UDPpacket **sendPacket, UDPpacket **receivePacket)
{
    if (*sendPacket != NULL)
    {
        SDLNet_FreePacket(*sendPacket);
        *sendPacket = NULL;
    }
    if (*receivePacket != NULL)
    {
        SDLNet_FreePacket(*receivePacket);
        *receivePacket = NULL;
    }
    if (*sd != NULL)
    {
        SDLNet_UDP_Close(*sd);
        *sd = NULL;
    }
    SDLNet_Quit();
}

void connectToServer(Player *pPlayer[MAX_NROFPLAYERS], bool is_server, int *pMy_id, IPaddress srvadd, int *pNrOfPlayers,
                        UDPpacket *pSendPacket, UDPpacket *pReceivePacket, UDPsocket sd)
{
    bool joined = false;
    if (is_server == 1)
    {
        setActive(pPlayer[*pMy_id], true);
        setIpAddress(pPlayer[*pMy_id], srvadd);
        (*pNrOfPlayers)++;
    }
    else
    {
        sprintf((char *)pSendPacket->data, "JOIN");
        pSendPacket->len = strlen((char *)pSendPacket->data) + 1;
        setIpAddress(pPlayer[0], srvadd);
        pSendPacket->address = srvadd;
        SDLNet_UDP_Send(sd, -1, pSendPacket);

        // ⬇️ Timeout-loop för att vänta på svar
        Uint32 join_start = SDL_GetTicks();
        while (!joined && SDL_GetTicks() - join_start < 3000)
        {
            if (SDLNet_UDP_Recv(sd, pReceivePacket))
            {
                int assigned;
                if (sscanf((char *)pReceivePacket->data, "WELCOME %d", &assigned) == 1)
                {
                    *pMy_id = assigned;
                    joined = true;
                    setActive(pPlayer[*pMy_id], true);
                    printf("Client: My ID is %d\n", *pMy_id);
                }
            }
            SDL_Delay(10); // undvik att stressa CPU
        }
        if (!joined)
        {
            printf("Client: Timeout - kunde inte ansluta till servern.\n");
            exit(EXIT_FAILURE);
        }
    }
}

