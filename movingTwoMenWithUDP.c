#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_net.h>

#define WINDOW_WIDTH (640)
#define WINDOW_HEIGHT (480)
#define SPEED (300)
#define MAX_PLAYERS 4

typedef struct {
  IPaddress addr;   // client IP+port
  int     x, y;     // last known position
  bool    active;   // slot in use?
} Player;
Player players[MAX_PLAYERS];

int main(int argc, char *argv[])
{
    UDPsocket sd;
    IPaddress srvadd;
    UDPpacket *p, *p2;

    int is_server = 0;
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        is_server = 1;
    }

    if (SDLNet_Init() < 0) {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (!(sd = SDLNet_UDP_Open(is_server ? 2000 : 0))) {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (!is_server) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s client <server_ip>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        if (SDLNet_ResolveHost(&srvadd, argv[2], 2000) == -1) {
            fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            exit(EXIT_FAILURE);
        }
    }

    if (!((p = SDLNet_AllocPacket(512)) && (p2 = SDLNet_AllocPacket(512)))) {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Two Player UDP Game",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!win) {
        printf("error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    if (!rend) {
        printf("error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* surface = IMG_Load("resources/drawing.png");
    if (!surface) {
        printf("error loading image\n");
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface);
    SDL_Texture* tex2 = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    SDL_Rect dest, secondDest;
    SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
    dest.w /= 4;
    dest.h /= 4;
    secondDest.w = dest.w;
    secondDest.h = dest.h;

    float x_pos = (WINDOW_WIDTH - dest.w) / 2;
    float y_pos = (WINDOW_HEIGHT - dest.h) / 2;
    float x_posOld = x_pos;
    float y_posOld = y_pos;
    float x_vel = 0, y_vel = 0;

    int up = 0, down = 0, left = 0, right = 0;
    int close_requested = 0;

    while (!close_requested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                close_requested = 1;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                int is_down = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP: up = is_down; break;
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT: left = is_down; break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN: down = is_down; break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT: right = is_down; break;
                    default: break;
                }
            }
        }

        x_vel = y_vel = 0;
        if (up && !down) y_vel = -SPEED;
        if (down && !up) y_vel = SPEED;
        if (left && !right) x_vel = -SPEED;
        if (right && !left) x_vel = SPEED;

        x_pos += x_vel / 60;
        y_pos += y_vel / 60;

        if (x_posOld != x_pos || y_posOld != y_pos) {
            sprintf((char*)p->data, "%d %d", (int)x_pos, (int)y_pos);
            p->len = strlen((char*)p->data) + 1;

            if (!is_server) {
                p->address.host = srvadd.host;
                p->address.port = srvadd.port;
            }

            SDLNet_UDP_Send(sd, -1, p);
            x_posOld = x_pos;
            y_posOld = y_pos;
        }

        if (SDLNet_UDP_Recv(sd, p2)) {
            int a, b;
            sscanf((char*)p2->data, "%d %d", &a, &b);
            secondDest.x = a;
            secondDest.y = b;

            if (is_server) {
                sprintf((char*)p->data, "%d %d", (int)x_pos, (int)y_pos);
                p->address = p2->address;
                p->len = strlen((char*)p->data) + 1;
                SDLNet_UDP_Send(sd, -1, p);
            }
        }

        if (x_pos < 0) x_pos = 0;
        if (y_pos < 0) y_pos = 0;
        if (x_pos > WINDOW_WIDTH - dest.w) x_pos = WINDOW_WIDTH - dest.w;
        if (y_pos > WINDOW_HEIGHT - dest.h) y_pos = WINDOW_HEIGHT - dest.h;

        dest.x = (int)x_pos;
        dest.y = (int)y_pos;

        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, tex, NULL, &dest);
        SDL_RenderCopy(rend, tex2, NULL, &secondDest);
        SDL_RenderPresent(rend);

        SDL_Delay(1000 / 60);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyTexture(tex2);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
