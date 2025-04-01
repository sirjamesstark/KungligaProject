#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>

#include "../include/platform.h"
#include "../include/player.h"

#define PLATTFORM_COUNT

#define COUNTER 20

typedef struct
{
    int window_width;
    int window_height;
    int speed_x;
    int speed_y;
} displayMode;

typedef struct
{
    SDL_Rect rect;
    bool isOnPlatform;
    float fallFromHeight;
} platform;

int main(int argv, char **args)
{

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    // Skärminformation för olika typer av skärmar
    SDL_DisplayMode windowMode;
    SDL_GetCurrentDisplayMode(0, &windowMode);

    // Initierarr struct
    displayMode DM;
    DM.window_width = windowMode.w;
    DM.window_height = windowMode.h;
    DM.speed_x = DM.window_width / 20;
    DM.speed_y = DM.window_height / 20;

    SDL_Window *pWindow = SDL_CreateWindow("Enkelt exempel 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DM.window_width, DM.window_height, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pWindow)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pRenderer)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *pSurface = IMG_Load("resources/Ship.png");
    if (!pSurface)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pTexture)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Rect playerRect;
    SDL_QueryTexture(pTexture, NULL, NULL, &playerRect.w, &playerRect.h);
    playerRect.w /= 6;
    playerRect.h /= 6;
    float playerX = (DM.window_width - playerRect.w) / 2;  // left side
    float playerY = (DM.window_height - playerRect.h) / 2; // upper side

    float playerVelocityX = 0; // unit: pixels/s
    float playerVelocityY = 0;

    bool closeWindow = false;
    bool up, down, left, right;
    bool onAir = 0;
    up = down = left = right = false;
    int upCounter = COUNTER, downCounter = COUNTER;

    while (!closeWindow)
    {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                closeWindow = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up = true;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left = true;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    down = true;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right = true;
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up = false;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left = false;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    down = false;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right = false;
                    break;
                }
                break;
            }
        }

        playerVelocityX = playerVelocityY = 0;

        if (up && !down && !onAir) //    Om man är på golvet och trycker bara upp
        {
            onAir = true; // Sätter vi på hoppande boolsk variabel på True (Nu hoppar vi)
        }
        if (onAir == true) // Om vi hoppar just nu
        {
            if (upCounter > 0) // Om räknaren för hopp är mer än 0
            {
                playerVelocityY = -(DM.speed_y * 5); // Rörelse upp
                upCounter--;                         // Minska räknaren för hopp
            }
            else // Om räknaren för hopp är 0
            {
                if ((downCounter > 0) && (playerY < (DM.window_height - playerRect.h))) // Om räknaren för gravitation är mer än 0 och karaktären är i luften
                {
                    playerVelocityY = (DM.speed_y) * 5; // Rörelse ner
                    downCounter--;                      // Minska räknare för gravitation
                }
                else // Om man har landat
                {
                    upCounter = COUNTER;   // Resetta räknare för hpop
                    downCounter = COUNTER; // Resetta räknare för gravitation
                    onAir = false;         // Vi hoppar inte längre
                }
            }
        }

        if (left && !right)
            playerVelocityX = -DM.speed_x;
        if (right && !left)
            playerVelocityX = DM.speed_x;
        playerX += playerVelocityX / 60; // 60 frames/s
        playerY += playerVelocityY / 60;
        if (playerX < 0)
            playerX = 0;
        if (playerY < 0)
            playerY = 0;
        if (playerX > DM.window_width - playerRect.w)
            playerX = DM.window_width - playerRect.w;
        if (playerY > DM.window_height - playerRect.h)
            playerY = DM.window_height - playerRect.h;
        playerRect.x = playerX;
        playerRect.y = playerY;

        SDL_RenderClear(pRenderer);
        SDL_RenderCopy(pRenderer, pTexture, NULL, &playerRect);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(1000 / 60); // 60 frames/s
    }

    SDL_DestroyTexture(pTexture);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);

    SDL_Quit();
    return 0;
}