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

#define COUNTER 20
#define NUM_MENU 2
#define BOX_ROW 22
#define BOX_COL 26

typedef struct
{
    int window_width;
    int window_height;
    int speed_x;
    int speed_y;
    bool continue_game;
} DisplayMode;

void initiateFullscreen(DisplayMode *pdM);
bool showMenu(SDL_Renderer *pRenderer, SDL_Window *pWindow, DisplayMode position);

int main(int argv, char **args)
{
    DisplayMode dM = {0};
    initiateFullscreen(&dM);

    if (!dM.continue_game)
    {
        return 1;
    }

    SDL_Window *pWindow = SDL_CreateWindow("Meny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dM.window_width, dM.window_height, SDL_WINDOW_FULLSCREEN_DESKTOP);
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

    bool startGame = showMenu(pRenderer, pWindow, dM);

    if (!startGame)
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 0;
    }

    // Här startar spelet om man väljer "Start"
    SDL_Surface *pPlayerSurface = IMG_Load("resources/Ship.png");
    SDL_Surface *pBlockSurface = IMG_Load("resources/boxPaint.png");
    if (!pPlayerSurface || !pBlockSurface)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *pPlayerTexture = SDL_CreateTextureFromSurface(pRenderer, pPlayerSurface);
    SDL_Texture *pBlockTexture = SDL_CreateTextureFromSurface(pRenderer, pBlockSurface);
    SDL_FreeSurface(pPlayerSurface);
    SDL_FreeSurface(pBlockSurface);
    if (!pPlayerTexture || !pBlockTexture)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Rect blockRect;
    blockRect.w = ((dM.window_width)/BOX_COL);
    blockRect.h = ((dM.window_height)/BOX_ROW);
    blockRect.x = (dM.window_width - blockRect.w)/2 + blockRect.w*5;
    blockRect.y = ((dM.window_height - blockRect.h)/2);

    SDL_Rect playerRect;
    SDL_QueryTexture(pPlayerTexture, NULL, NULL, &playerRect.w, &playerRect.h);
    playerRect.w = ((dM.window_width)/BOX_COL)/2;
    playerRect.h = ((dM.window_height)/BOX_ROW)/2;
    float shipX = blockRect.w * 2;
    float shipY = dM.window_height - (dM.window_height - (BOX_ROW * blockRect.h)) - blockRect.h - playerRect.h ;

    float shipVelocityX = 0; // unit: pixels/s
    float shipVelocityY = 0;

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
                    case SDL_SCANCODE_SPACE:
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
                    case SDL_SCANCODE_ESCAPE:
                        SDL_DestroyTexture(pPlayerTexture);
                        SDL_DestroyRenderer(pRenderer);
                        SDL_DestroyWindow(pWindow);
                        SDL_Quit();
                        return 0;
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
        shipVelocityX = shipVelocityY = 0;

        if (up && !down && onGround) 
        {
            upCounter = COUNTER;
        }
        if (upCounter > 0)
        {
            shipVelocityY = -(dM.speed_y * 5); 
            upCounter--;
        }
        else if (!onGround)
        {
            shipVelocityY = (dM.speed_y) * 5; 
        }

        if(left && !right) shipVelocityX = -dM.speed_x;
        if(right && !left) shipVelocityX = dM.speed_x;

        shipX += shipVelocityX * 5 * deltaTime;
        shipY += shipVelocityY * deltaTime;
        
        // Check Collision

        if (gameMap[(((int)shipY - 1) + playerRect.h)/blockRect.h][((int)shipX + 1)/blockRect.w] == 1 ||  // Bottom edge blocked on left?
            gameMap[((int)shipY + 1)/blockRect.h][((int)shipX + 1)/blockRect.w] == 1 ||     // Top edge blocked on left?
            gameMap[(((int)shipY - 1)+playerRect.h)/blockRect.h][(((int)shipX - 1)+playerRect.w) / blockRect.w] == 1 || // Bottom edge blocked on right?
            gameMap[((int)shipY + 1)/blockRect.h][(((int)shipX - 1)+playerRect.w) / blockRect.w] == 1)  // Top edge blocked on right?
        {
            shipX -= shipVelocityX * 5 * deltaTime;         //Dont move
            printf("blocked on side\n");
        }
        if (gameMap[((int)shipY + 1)/blockRect.h][((int)shipX + 1)/blockRect.w] == 1 ||   // Left edge blocked on top?
            gameMap[((int)shipY + 1)/blockRect.h][((int)shipX - 1 + playerRect.w)/blockRect.w] == 1) // Right edge blocked on top?
        {
            shipY -= shipVelocityY * deltaTime;
            upCounter = 0;
        }
        if (gameMap[(((int)shipY - 1) + playerRect.h)/blockRect.h][((int)shipX + 1)/blockRect.w] == 1 || // Left edge blocked on bottom?
            gameMap[(((int)shipY - 1) + playerRect.h)/blockRect.h][((int)shipX - 1 + playerRect.w)/blockRect.w] == 1)  // Right edge blocked on bottom?
        {
            shipY -= shipVelocityY * deltaTime;
            onGround = true;
        }
        else
        {
            onGround = false;
        }
        
        
        if(shipX<0) shipX=0;
        if(shipY<0) shipY=0;
        if(shipX>dM.window_width-playerRect.w) shipX = dM.window_width-playerRect.w;
        if(shipY>dM.window_height-playerRect.h) shipY = dM.window_height-playerRect.h;
        playerRect.x = shipX;
        playerRect.y = shipY;

        SDL_RenderClear(pRenderer);
        SDL_RenderCopy(pRenderer,pPlayerTexture,NULL,&playerRect);

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

                    SDL_RenderCopy(pRenderer, pBlockTexture, NULL, &blockRect);
                }
            }
        }

        SDL_RenderPresent(pRenderer);
        SDL_Delay(1); // Undvik 100% CPU-användning men låt SDL hantera FPS
    }

    SDL_DestroyTexture(pPlayerTexture);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return 0;
}

bool showMenu(SDL_Renderer *pRenderer, SDL_Window *pWindow, DisplayMode position)
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

    SDL_Texture *pStartTexture = SDL_CreateTextureFromSurface(pRenderer, pStartSurface);
    SDL_Texture *pExitTexture = SDL_CreateTextureFromSurface(pRenderer, pExitSurface);
    SDL_Texture *pStartBlueTexture = SDL_CreateTextureFromSurface(pRenderer, pStartBlueSurface);
    SDL_Texture *pExitBlueTexture = SDL_CreateTextureFromSurface(pRenderer, pExitBlueSurface);
    SDL_Texture *pSoundOnTexture = SDL_CreateTextureFromSurface(pRenderer, pSoundOnSurface);
    SDL_Texture *pSoundOffTexture = SDL_CreateTextureFromSurface(pRenderer, pSoundOffSurface);

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

        SDL_RenderClear(pRenderer);
        if (soundgame)
        {
            SDL_RenderCopy(pRenderer, pSoundOnTexture, NULL, &soundRect);
        }
        else
        {
            SDL_RenderCopy(pRenderer, pSoundOffTexture, NULL, &soundRect);
        }

        if (menuChoice == 0)
        {
            SDL_RenderCopy(pRenderer, pStartTexture, NULL, &startRect);
            SDL_RenderCopy(pRenderer, pExitTexture, NULL, &exitRect);
        }
        if (menuChoice == 1)
        {
            SDL_RenderCopy(pRenderer, pStartBlueTexture, NULL, &startRect);
            SDL_RenderCopy(pRenderer, pExitTexture, NULL, &exitRect);
        }
        else if (menuChoice == 2)
        {
            SDL_RenderCopy(pRenderer, pStartTexture, NULL, &startRect);
            SDL_RenderCopy(pRenderer, pExitBlueTexture, NULL, &exitRect);
        }
        SDL_RenderPresent(pRenderer);
    }

    SDL_DestroyTexture(pStartTexture);
    SDL_DestroyTexture(pExitTexture);
    SDL_DestroyTexture(pStartBlueTexture);
    SDL_DestroyTexture(pExitBlueTexture);
    SDL_DestroyTexture(pSoundOnTexture);
    SDL_DestroyTexture(pSoundOffTexture);

    return startGame;
}

void initiateFullscreen(DisplayMode *pdM)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        pdM->continue_game = false;
        return;
    }

    SDL_DisplayMode windowMode;
    if (SDL_GetCurrentDisplayMode(0, &windowMode) != 0)
    {
        printf("Failed to get display mode: %s\n", SDL_GetError());
        pdM->continue_game = false;
    }
    else
    {
        pdM->window_width = windowMode.w;
        pdM->window_height = windowMode.h;
        pdM->speed_x = pdM->window_width / 20;
        pdM->speed_y = pdM->window_height / 20;
        pdM->continue_game = true;
    }
}