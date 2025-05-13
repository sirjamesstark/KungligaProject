#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL2/SDL_net.h>

#define WINDOW_WIDTH (640)
#define WINDOW_HEIGHT (480)
#define SPEED (300)
#define MAX_PLAYERS 4

typedef struct
{
    SDL_Texture *pTexture;
    SDL_Rect *pRect;
    IPaddress addr;                 // client IP+port
    float x, y, oldX, oldY, vx, vy; // last known position
    bool active;                    // slot in use?
} Player;

int main(int argc, char *argv[])
{
    UDPsocket sd;
    IPaddress srvadd;
    UDPpacket *sendPacket, *receivePacket;
    Player players[MAX_PLAYERS] = {0};

    int is_server = 0, my_id = -1, nrOfPlayers = 0;
    bool joined = false;
    if (argc > 1 && strcmp(argv[1], "server") == 0)
    {
        is_server = 1;
        my_id = 0;
        players[my_id].active = true;
        nrOfPlayers++;
    }

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (!(sd = SDLNet_UDP_Open(is_server ? 2000 : 0)))
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (!is_server)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: %s client <server_ip>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        if (SDLNet_ResolveHost(&srvadd, argv[2], 2000) == -1)
        {
            fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            exit(EXIT_FAILURE);
        }
    }

    if (!((sendPacket = SDLNet_AllocPacket(512)) && (receivePacket = SDLNet_AllocPacket(512))))
    {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Testing Multiplayer",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!win)
    {
        printf("error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, render_flags);
    if (!rend)
    {
        printf("error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surface = IMG_Load("resources/drawing.png");
    if (!surface)
    {
        printf("error loading image\n");
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *sharedTexture = SDL_CreateTextureFromSurface(rend, surface);
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        players[i].pTexture = sharedTexture;
    }
    SDL_FreeSurface(surface);
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        players[i].pRect = malloc(sizeof(SDL_Rect));
    }
    SDL_QueryTexture(players[0].pTexture, NULL, NULL, &players[0].pRect->w, &players[0].pRect->h);
    players[0].pRect->w /= 4;
    players[0].pRect->h /= 4;
    players[3].pRect->w = players[1].pRect->w = players[2].pRect->w = players[0].pRect->w;
    players[1].pRect->h = players[2].pRect->h = players[3].pRect->h = players[0].pRect->h;

    players[0].x = (WINDOW_WIDTH - players[0].pRect->w) / 2;
    players[0].y = (WINDOW_HEIGHT - players[0].pRect->h) / 2;
    players[0].oldX = players[0].x;
    players[0].oldY = players[0].y;
    players[0].vx = players[0].vy = 0;

    int up = 0, down = 0, left = 0, right = 0;
    int close_requested = 0;

    if (!is_server)
    {
        sprintf((char *)sendPacket->data, "JOIN");
        sendPacket->len = strlen((char *)sendPacket->data) + 1;
        sendPacket->address = srvadd;
        SDLNet_UDP_Send(sd, -1, sendPacket);

        // ⬇️ Timeout-loop för att vänta på svar
        Uint32 join_start = SDL_GetTicks();
        while (!joined && SDL_GetTicks() - join_start < 3000)
        {
            if (SDLNet_UDP_Recv(sd, receivePacket))
            {
                int assigned;
                if (sscanf((char *)receivePacket->data, "GET THE FUCK IN %d", &assigned) == 1)
                {
                    my_id = assigned;
                    joined = true;
                    players[my_id].active = true;
                    printf("Client: My ID is %d\n", my_id);
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

    while (!close_requested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                close_requested = 1;
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                int is_down = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up = is_down;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left = is_down;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    down = is_down;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right = is_down;
                    break;
                default:
                    break;
                }
            }
        }

        players[my_id].vx = players[my_id].vy = 0;
        if (up && !down)
            players[my_id].vy = -SPEED;
        if (down && !up)
            players[my_id].vy = SPEED;
        if (left && !right)
            players[my_id].vx = -SPEED;
        if (right && !left)
            players[my_id].vx = SPEED;

        players[my_id].x += players[my_id].vx / 60;
        players[my_id].y += players[my_id].vy / 60;

        if (players[my_id].oldX != players[my_id].x || players[my_id].oldY != players[my_id].y)
        {
            snprintf((char *)sendPacket->data, 512, "%d %d %d", my_id, (int)players[my_id].x, (int)players[my_id].y);
            sendPacket->len = strlen((char *)sendPacket->data) + 1;

            if (is_server)
            {
                for (int i = 1; i < nrOfPlayers; i++)
                {
                    if (players[i].active)
                    {
                        sendPacket->address = players[i].addr;
                        SDLNet_UDP_Send(sd, -1, sendPacket);
                    }
                }
            }
            else // Client sends to server
            {
                sendPacket->address.host = srvadd.host;
                sendPacket->address.port = srvadd.port;
                SDLNet_UDP_Send(sd, -1, sendPacket);
            }
            players[my_id].oldX = players[my_id].x;
            players[my_id].oldY = players[my_id].y;
        }

        if (SDLNet_UDP_Recv(sd, receivePacket))
        {
            if (is_server)
            {
                if (strncmp((char *)receivePacket->data, "JOIN", 4) == 0)
                {
                    if (nrOfPlayers < MAX_PLAYERS)
                    {
                        int newID = nrOfPlayers;
                        players[newID].active = true;
                        players[newID].x = 50 * newID + 50;
                        players[newID].y = 50;
                        players[newID].oldX = players[newID].x;
                        players[newID].oldY = players[newID].y;
                        players[newID].vx = players[newID].vy = 0;
                        players[newID].pRect->x = players[newID].x;
                        players[newID].pRect->y = players[newID].y;
                        players[newID].pRect->w = players[0].pRect->w;
                        players[newID].pRect->h = players[0].pRect->h;
                        players[newID].pTexture = players[0].pTexture; // Dela samma textur
                        players[newID].addr = receivePacket->address;

                        sprintf((char *)sendPacket->data, "GET THE FUCK IN %d", newID);
                        sendPacket->len = strlen((char *)sendPacket->data) + 1;
                        sendPacket->address = receivePacket->address;
                        SDLNet_UDP_Send(sd, -1, sendPacket);

                        printf("Server: Accepted JOIN, assigned ID %d\n", newID);

                        for (int i = 0; i < nrOfPlayers; i++) // Include server (i=0)
                        {
                            if (players[i].active)
                            {
                                snprintf((char *)sendPacket->data, 512, "%d %d %d",
                                         i, (int)players[i].x, (int)players[i].y);
                                sendPacket->len = strlen((char *)sendPacket->data) + 1;
                                sendPacket->address = players[newID].addr;
                                SDLNet_UDP_Send(sd, -1, sendPacket);
                            }
                        }

                        nrOfPlayers++;
                    }
                    else
                    {
                        printf("Server: JOIN refused, server full.\n");
                    }
                }
                else
                {
                    // Positionsuppdatering: "123 456"
                    int id, client_x, client_y;
                    if (sscanf((char *)receivePacket->data, "%d %d %d", &id, &client_x, &client_y) == 3)
                    {
                        if (id >= 0 && id < MAX_PLAYERS && players[id].active)
                        {
                            players[id].x = players[id].pRect->x = client_x;
                            players[id].y = players[id].pRect->y = client_y;
                        }
                        for (int i = 0; i < nrOfPlayers; i++)
                        {
                            if (players[i].active && i != id) // Don't send back to sender
                            {
                                snprintf((char *)sendPacket->data, 512, "%d %d %d", id, client_x, client_y);
                                sendPacket->len = strlen((char *)sendPacket->data) + 1;
                                sendPacket->address = players[i].addr;
                                SDLNet_UDP_Send(sd, -1, sendPacket);
                            }
                        }
                    }
                }
            }
            else
            {
                // Klient: mottar data i formatet "id x y"
                int id, x, y;
                if (sscanf((char *)receivePacket->data, "%d %d %d", &id, &x, &y) == 3)
                {
                    if (id >= 0 && id < MAX_PLAYERS && id != my_id)
                    {
                        players[id].x = x;
                        players[id].y = y;
                        players[id].pRect->x = x;
                        players[id].pRect->y = y;
                        players[id].active = true;
                    }
                }
            }
        }

        if (players[my_id].x < 0)
            players[my_id].x = 0;
        if (players[my_id].y < 0)
            players[my_id].y = 0;
        if (players[my_id].x > WINDOW_WIDTH - players[my_id].pRect->w)
            players[my_id].x = WINDOW_WIDTH - players[my_id].pRect->w;
        if (players[my_id].y > WINDOW_HEIGHT - players[my_id].pRect->h)
            players[my_id].y = WINDOW_HEIGHT - players[my_id].pRect->h;

        players[my_id].pRect->x = (int)players[my_id].x;
        players[my_id].pRect->y = (int)players[my_id].y;

        SDL_RenderClear(rend);
        printf("id: %d, %f %f active: %d\n", my_id, players[my_id].x, players[my_id].y, players[my_id].active);
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (players[i].active)
            {
                SDL_RenderCopy(rend, players[i].pTexture, NULL, players[i].pRect);
            }
        }
        SDL_RenderPresent(rend);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyTexture(sharedTexture); // en gång
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        free(players[i].pRect);
    }
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDLNet_FreePacket(sendPacket);
    SDLNet_FreePacket(receivePacket);
    SDLNet_Quit();
    SDL_Quit();
}
