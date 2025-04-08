#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>

#include "../include/game.h"
#include "../include/menu.h"
#include "../include/platform.h"
#include "../include/player.h"
#include "../include/renderer.h"

#define NUM_MENU 2

struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
    // AsteroidImage *pAsteroidImage;
    // Asteroid *pAsteroids[MAX_ASTEROIDS];
};
typedef struct game Game;

typedef struct {
    int window_width;
    int window_height;
    int speed_x;
    int speed_y;
    bool continue_game;
} DisplayMode;

int initiate(DisplayMode *pdM,Game *pGame);
bool showMenu(Game *pGame, DisplayMode position);
void handleInput(Game *pGame,SDL_Event *pEvent,bool *pCloseWindow,
                bool*pUp,bool *pDown,bool *pLeft,bool *pRight);

int main(int argv, char **args)
{
    DisplayMode dM = {0};
    Game game={0}; 
    if(!initiate(&dM,&game)) return 1;

    if (!showMenu(&game, dM))
    {
        SDL_DestroyRenderer(game.pRenderer);
        SDL_DestroyWindow(game.pWindow);
        SDL_Quit();
        return 1;
    }

    // Här startar spelet om man väljer "Start"
    SDL_Surface *pBlockSurface = IMG_Load("resources/boxPaint.png");
    if (!pBlockSurface)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(game.pRenderer);
        SDL_DestroyWindow(game.pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *pBlockTexture = SDL_CreateTextureFromSurface(game.pRenderer, pBlockSurface);
    SDL_FreeSurface(pBlockSurface);
    if (!pBlockTexture)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(game.pRenderer);
        SDL_DestroyWindow(game.pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Rect blockRect;
    blockRect.w = ((dM.window_width)/BOX_COL);
    blockRect.h = ((dM.window_height)/BOX_ROW);
    blockRect.x = (dM.window_width - blockRect.w)/2 + blockRect.w*5;
    blockRect.y = ((dM.window_height - blockRect.h)/2);

    (&game)->pPlayer = createPlayer(blockRect,(&game)->pRenderer,dM.window_width,dM.window_height);

    bool closeWindow = false;
    bool up, down, left, right;
    bool onGround = true;
    up = down = left = right = false;
    int upCounter = 0;

    Uint32 lastTime = SDL_GetTicks(); // Tidpunkt för senaste uppdateringen
    Uint32 currentTime;
    float deltaTime;

    int gameMap[BOX_ROW][BOX_COL] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
                                     1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,
                                     0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,
                                     0,1,0,0,0,0,0,1,1,0,0,1,1,1,1,0,0,0,0,1,0,0,0,1,1,0,
                                     0,1,0,0,1,1,0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,
                                     0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,
                                     0,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,0,
                                     0,1,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,0,
                                     0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                                     0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
                                     0,1,0,1,1,1,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,1,0,
                                     0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,1,0,0,1,1,0,
                                     0,1,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,0,
                                     0,1,1,0,0,1,1,0,1,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,1,0,
                                     0,1,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,0,1,0,0,0,0,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,
                                     0,1,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,1,1,0,
                                     0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};
    while (!closeWindow)
    {
        // Beräkna tid sedan senaste frame
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; // Omvandla till sekunder
        lastTime = currentTime;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if(event.type==SDL_QUIT) closeWindow = true;
            else handleInput(&game,&event,&closeWindow,&up,&down,&left,&right);
        }
        setSpeed(up,down,left,right,&upCounter,onGround,game.pPlayer,dM.speed_x,dM.speed_y);
        updatePlayer(game.pPlayer,deltaTime,gameMap,blockRect,&upCounter,&onGround);
        SDL_RenderClear(game.pRenderer);
        drawPlayer(game.pPlayer);

        int numBlocksX = dM.window_width / blockRect.w;  // Antal lådor per rad
        int numBlocksY = (dM.window_height / blockRect.h);  // Antal rader 

        // Loopa över rader (Y-led)
        for (int row = 0; row < numBlocksY; row++) {
            // Loopa över kolumner (X-led)
            for (int col = 0; col < numBlocksX; col++) {
                if (gameMap[row][col] == 1)
                {
                    blockRect.x = col * blockRect.w;  // Placera lådan horisontellt
                    blockRect.y = row * blockRect.h;  // Placera lådan vertikalt (från botten uppåt)

                    SDL_RenderCopy(game.pRenderer, pBlockTexture, NULL, &blockRect);
                }
            }
        }

        SDL_RenderPresent(game.pRenderer);
        SDL_Delay(1); // Undvik 100% CPU-användning men låt SDL hantera FPS
    }

    destroyPlayer(game.pPlayer);
    SDL_DestroyRenderer(game.pRenderer);
    SDL_DestroyWindow(game.pWindow);
    SDL_Quit();

    return 0;
}

int initiate(DisplayMode *pdM,Game *pGame)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_DisplayMode windowMode;
    if (SDL_GetCurrentDisplayMode(0, &windowMode) != 0)
    {
        printf("Failed to get display mode: %s\n", SDL_GetError());
        return 0;
    }
    else
    {
        pdM->window_width = windowMode.w;
        pdM->window_height = windowMode.h;
        pdM->speed_x = pdM->window_width / 20;
        pdM->speed_y = pdM->window_height / 20;
    }
    pGame->pWindow = SDL_CreateWindow("Meny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pdM->window_width, pdM->window_height, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pGame->pWindow)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(pGame->pWindow);
        SDL_Quit();
        return 0;
    }
    else return 1;
}

bool showMenu(Game *pGame, DisplayMode position)
{
    int menuChoice = 1;
    SDL_Surface *pStartSurface = IMG_Load("resources/menu_start.png");
    SDL_Surface *pExitSurface = IMG_Load("resources/menu_exit.png");
    SDL_Surface *pStartBlueSurface = IMG_Load("resources/menu_start_blue.png");
    SDL_Surface *pExitBlueSurface = IMG_Load("resources/menu_exit_blue.png");

    SDL_Surface *pSoundOnSurface = IMG_Load("resources/soundon.png");
    SDL_Surface *pSoundOffSurface = IMG_Load("resources/soundoff.png");

    if (!pStartSurface || !pExitSurface || !pStartBlueSurface || !pExitBlueSurface ||
        !pSoundOnSurface || !pSoundOffSurface)
    {
        printf("Error loading menu images: %s\n", SDL_GetError());
        return false;
    }

    SDL_Texture *pStartTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pStartSurface);
    SDL_Texture *pExitTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pExitSurface);
    SDL_Texture *pStartBlueTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pStartBlueSurface);
    SDL_Texture *pExitBlueTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pExitBlueSurface);
    SDL_Texture *pSoundOnTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pSoundOnSurface);
    SDL_Texture *pSoundOffTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pSoundOffSurface);

    SDL_FreeSurface(pStartSurface);
    SDL_FreeSurface(pExitSurface);
    SDL_FreeSurface(pStartBlueSurface);
    SDL_FreeSurface(pExitBlueSurface);
    SDL_FreeSurface(pSoundOnSurface);
    SDL_FreeSurface(pSoundOffSurface);

    if (!pStartTexture || !pExitTexture)
    {
        printf("Error creating menu textures: %s\n", SDL_GetError());
        return false;
    }

    SDL_Rect startRect;
    startRect.w = ((position.window_width) / 6) * 2;
    startRect.h = ((position.window_height) / 10) * 2;
    startRect.x = (position.window_width - startRect.w) / 2;
    startRect.y = ((position.window_height - (startRect.h) * 4));
    SDL_Rect exitRect;
    exitRect.w = ((position.window_width) / 6) * 2;
    exitRect.h = ((position.window_height) / 10) * 2;
    exitRect.x = (position.window_width - exitRect.w) / 2;
    exitRect.y = ((position.window_height - (exitRect.h) * 2));
    SDL_Rect soundRect;
    soundRect.w = ((position.window_width) / 20);
    soundRect.h = ((position.window_height) / 9);
    soundRect.x = (position.window_width - soundRect.w - (position.window_width / 10));
    soundRect.y = ((position.window_height / 10));

    int mousex, mousey;
    bool menuRunning = true;
    bool startGame = false;
    bool soundgame = true;
    bool muteHover = false;

    while (menuRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
           if (event.type == SDL_QUIT)
            {
                menuRunning = false;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&mousex, &mousey);
                if ((mousex > exitRect.x && mousex < exitRect.x + exitRect.w) && (mousey > exitRect.y && mousey < exitRect.y + exitRect.h))
                {
                    menuChoice = 2;
                }
                else if ((mousex > startRect.x && mousex < startRect.x + startRect.w) && (mousey > startRect.y && mousey < startRect.y + startRect.h))
                {
                    menuChoice = 1;
                }
                else
                {
                    menuChoice = 0;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (SDL_BUTTON_LEFT == event.button.button)
                {
                    if ((mousex > soundRect.x && mousex < soundRect.x + soundRect.w) && (mousey > soundRect.y && mousey < soundRect.y + soundRect.h))
                    {
                        if (soundgame == true)
                        {
                            soundgame = false;
                        }
                        else
                        {
                            soundgame = true;
                        }
                    }
                    if (menuChoice == 1)
                    {
                        startGame = true;
                        menuRunning = false;
                    }
                    if (menuChoice == 2)
                    {
                        menuRunning = false;
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_UP:
                    if (menuChoice < 2)
                    {
                        menuChoice = 1;
                    }
                    else
                    {
                        menuChoice--;
                    }
                    break;
                case SDL_SCANCODE_DOWN:
                    if (menuChoice == NUM_MENU)
                    {
                        menuChoice = NUM_MENU;
                    }
                    else
                    {
                        menuChoice++;
                    }
                    break;
                case SDL_SCANCODE_RETURN:
                    if (menuChoice == 1)
                    {
                        startGame = true;
                        menuRunning = false;
                    }
                    else
                    {
                        menuRunning = false;
                    }
                }
            }
        }

        SDL_RenderClear(pGame->pRenderer);
        if (soundgame)
        {
            SDL_RenderCopy(pGame->pRenderer, pSoundOnTexture, NULL, &soundRect);
        }
        else
        {
            SDL_RenderCopy(pGame->pRenderer, pSoundOffTexture, NULL, &soundRect);
        }

        if (menuChoice == 0)
        {
            SDL_RenderCopy(pGame->pRenderer, pStartTexture, NULL, &startRect);
            SDL_RenderCopy(pGame->pRenderer, pExitTexture, NULL, &exitRect);
        }
        if (menuChoice == 1)
        {
            SDL_RenderCopy(pGame->pRenderer, pStartBlueTexture, NULL, &startRect);
            SDL_RenderCopy(pGame->pRenderer, pExitTexture, NULL, &exitRect);
        }
        else if (menuChoice == 2)
        {
            SDL_RenderCopy(pGame->pRenderer, pStartTexture, NULL, &startRect);
            SDL_RenderCopy(pGame->pRenderer, pExitBlueTexture, NULL, &exitRect);
        }
        SDL_RenderPresent(pGame->pRenderer);
    }

    SDL_DestroyTexture(pStartTexture);
    SDL_DestroyTexture(pExitTexture);
    SDL_DestroyTexture(pStartBlueTexture);
    SDL_DestroyTexture(pExitBlueTexture);
    SDL_DestroyTexture(pSoundOnTexture);
    SDL_DestroyTexture(pSoundOffTexture);

    return startGame;
}

void handleInput(Game *pGame,SDL_Event *pEvent,bool *pCloseWindow,
    bool*pUp,bool *pDown,bool *pLeft,bool *pRight)
{
    if(pEvent->type == SDL_KEYDOWN)
    {
        switch (pEvent->key.keysym.scancode)
        {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_SPACE:
                (*pUp) = true;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                (*pLeft) = true;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                (*pDown) = true;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                (*pRight) = true;
                break;
            case SDL_SCANCODE_ESCAPE:
                (*pCloseWindow) = true;
                return;
        }
    }
    else if (pEvent->type == SDL_KEYUP)
    {
        switch (pEvent->key.keysym.scancode)
        {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                (*pUp) = false;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                (*pLeft) = false;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                (*pDown) = false;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                (*pRight) = false;
                break;
        }
    }
    
}