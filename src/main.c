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
#include "../include/theme.h"
#include "../include/maps.h"
#include "../include/net.h"

#define NUM_MENU 2
#define MAX_NROFPLAYERS 2

typedef struct 
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer[MAX_NROFPLAYERS];
    Mix_Chunk *pJumpSound;
    Mix_Music *pGameMusic;
    BlockImage *pBlockImage;
    Block *pBlock;
    Maps *pMaps[NROFMAPS];
    Background *pBackground;

    bool is_server;
    Server *pServer;
    int pPlayerID;
} Game;

typedef struct 
{
    int window_width;
    int window_height;
    int speed_x;
    int speed_y;
    bool continue_game;
} DisplayMode;

int initiate(DisplayMode *pdM,Game *pGame, int argv, char **args);
void handleInput(Game *pGame,SDL_Event *pEvent,bool *pCloseWindow, bool *pUp,bool *pDown,bool *pLeft,bool *pRight);
void cleanUp(Game *pGame);

int main(int argc, char *argv[])
{
    Game game = {0};
    DisplayMode dM = {0};

    bool startGame = false;

    if (!initiate(&dM,&game,argc,argv))
    {
        return 1;
    } 
    if (!game.is_server && !game.pServer)
    {
        fprintf(stderr, "Fel: Klienten kunde inte skapa eller ansluta till server-socket.\n");
        cleanUp(&game);
        return 1;
    }
    if (!showMenu(game.pRenderer,dM.window_width,dM.window_height))
    {
        cleanUp(&game);
        return 1;
    }
    
    game.pGameMusic = initiateMusic(game.pGameMusic);
    game.pBackground = createBackground(game.pRenderer, dM.window_width, dM.window_height);
    game.pBlockImage = createBlockImage(game.pRenderer);
    game.pBlock = createBlock(game.pBlockImage,dM.window_width,dM.window_height);
    SDL_Rect blockRect = getRectBlock(game.pBlock);

    if (game.is_server)
    {
        game.pPlayerID = 0;
        (&game)->pPlayer[0] = createPlayer(blockRect,(&game)->pRenderer,dM.window_width,dM.window_height, game.pPlayerID);
        (&game)->pPlayer[1] = createPlayer(blockRect,(&game)->pRenderer,dM.window_width,dM.window_height, 1);
    }
    else 
    {
        game.pPlayerID = 1;
        (&game)->pPlayer[0] = createPlayer(blockRect,(&game)->pRenderer,dM.window_width,dM.window_height, 0);
        (&game)->pPlayer[1] = createPlayer(blockRect,(&game)->pRenderer,dM.window_width,dM.window_height, game.pPlayerID);
    }
    
    if (!game.pGameMusic || !game.pBackground || !game.pBlockImage || !game.pPlayer[0] || !game.pPlayer[1])
    {
        cleanUp(&game);
        return 1;
    }

    bool closeWindow = false;
    bool up, down, left, right, goUp, goDown, goLeft, goRight;
    bool onGround = true;
    up = down = left = right = false;
    int upCounter = 0,chosenMap = 0, gameMap[BOX_ROW][BOX_COL] = {0};

    Uint32 lastTime = SDL_GetTicks(); // Tidpunkt för senaste uppdateringen
    Uint32 currentTime;
    float deltaTime;
    drawBlueprints(game.pMaps);
    chooseMap(gameMap,game.pMaps[chosenMap]);
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
        goDown = goLeft = goRight = goUp = 0;
        setSpeed(up,down,left,right,&goUp,&goDown,&goLeft,&goRight,&upCounter,onGround,game.pPlayer[game.pPlayerID],dM.speed_x,dM.speed_y);
        updatePlayer(game.pPlayer[game.pPlayerID],deltaTime,gameMap,blockRect,&upCounter,&onGround,&goUp,&goDown,&goLeft,&goRight);

        if (!game.is_server)
        {
            if (game.pPlayer[0] && game.pPlayer[1])
            {
                sendPaket(getPlayerRect(game.pPlayer[1]), game.pServer, game.is_server);
                recivePaket(game.pServer, game.is_server, getPlayerRect(game.pPlayer[0]), getPlayerRect(game.pPlayer[1]));
            }
        } 
        else
        {
            recivePaket(game.pServer, game.is_server, getPlayerRect(game.pPlayer[1]), getPlayerRect(game.pPlayer[0]));
        }

        SDL_RenderClear(game.pRenderer);
        drawBackground(game.pBackground);
        buildTheMap(gameMap,game.pBlock);
        for (int i = 0; i < MAX_NROFPLAYERS; i++)
        {
            drawPlayer(game.pPlayer[i]);
        }

        SDL_RenderPresent(game.pRenderer);
        SDL_Delay(1); // Undvik 100% CPU-användning men låt SDL hantera FPS
    }
    NET_Quit(game.pServer);
    cleanUp(&game);
    return 0;
}

int initiate(DisplayMode *pdM,Game *pGame, int argv, char **args)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }

    if (argv > 1 && strcmp(args[1], "server") == 0) { // UDP prototyp
        pGame->is_server = true; 
    }

    pGame->pServer = NET_INIT(pGame->is_server);

    bindPort(pGame->is_server, pGame->pServer);

    if (!pGame->is_server) setSrvAdd_client(args, argv, pGame->pServer);

    // Initialize SDL_image for PNG loading
    int iconImage = IMG_INIT_PNG;
    if (!(IMG_Init(iconImage) & iconImage)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        cleanUp(pGame);
        return 0;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        cleanUp(pGame);
        return 0;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        cleanUp(pGame);
        return 0;
    }

    SDL_DisplayMode currentDisplay;
    if (SDL_GetCurrentDisplayMode(0, &currentDisplay) != 0) {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }

    pdM->window_width = currentDisplay.w;
    pdM->window_height = currentDisplay.h;
    
    /*
    float aspect_ratio = (float)(pdM->window_width/pdM->window_height);
    if (aspect_ratio != (16.0f / 9.0f)) {
        // Här tvingar vi till en 16:9 aspect ratio genom att justera höjd eller bredd
        if (aspect_ratio > (16.0f / 9.0f)) {
            // Om bredden är för stor (mer än 16:9)
            pdM->window_width = pdM->window_height * 16 / 9;
        } else {
            // Om höjden är för stor (mer än 9:16)
            pdM->window_height = pdM->window_width * 9 / 16;
        }
    }
    */

    pdM->speed_x = pdM->window_width / 20;
    pdM->speed_y = pdM->window_height / 20;

    // Create window with explicit cursor support
    pGame->pWindow = SDL_CreateWindow("Lavan?", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pdM->window_width, pdM->window_height, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pGame->pWindow)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }
    
    pGame->pJumpSound = NULL;  // Initialize jump sound to NULL

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }

    // Initialize SDL_image for cursor loading
    int cursor = IMG_INIT_PNG;
    if (!(IMG_Init(cursor) & cursor)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        cleanUp(pGame);
        return 0;
    }

    // Force cursor to be visible first
    SDL_ShowCursor(SDL_ENABLE);

    // Load and set custom cursor
    SDL_Surface* cursorSurface = IMG_Load("resources/cursor.png");
    if (!cursorSurface)
    {
        printf("Failed to load cursor image! SDL_image Error: %s\n", IMG_GetError());
    }
    else
    {
        // Create cursor with hotspot at top-left for better precision
        SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 0, 0);
        if (!cursor)
        {
            printf("Failed to create cursor! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            SDL_SetCursor(cursor);
            // Make sure cursor stays visible
            SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
        }
        SDL_FreeSurface(cursorSurface);
    }

    // Double check cursor visibility
    if (SDL_ShowCursor(-1) != SDL_ENABLE) {
        SDL_ShowCursor(SDL_ENABLE);
    }

    return 1;
}

void handleInput(Game *pGame,SDL_Event *pEvent,bool *pCloseWindow,
    bool*pUp,bool *pDown,bool *pLeft,bool *pRight)
{
    // First time jumping? I'll load the sound - then it's ready for next time
    if (!pGame->pJumpSound) {
        pGame->pJumpSound = Mix_LoadWAV("resources/jump_sound.wav");
        if (!pGame->pJumpSound) {
            printf("Failed to load jump sound! SDL_mixer Error: %s\n", Mix_GetError());
        }
    }

    if(pEvent->type == SDL_KEYDOWN)
    {
        switch (pEvent->key.keysym.scancode)
        {
            default: // Handle any unspecified keys
                break;

            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_SPACE:
                (*pUp) = true;
                if (pGame->pJumpSound) {
                    Mix_PlayChannel(-1, pGame->pJumpSound, 0);  // Play jump sound
                }
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
                if (pGame->pJumpSound) {
                    Mix_FreeChunk(pGame->pJumpSound);  // Clean up jump sound
                    pGame->pJumpSound = NULL;
                }
                (*pCloseWindow) = true;
                break;
        }
    }
    else if (pEvent->type == SDL_KEYUP)
    {
        switch (pEvent->key.keysym.scancode)
        {
            default: // Handle any unspecified keys
                break;
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

void cleanUp(Game *pGame) 
{
    if (pGame == NULL) return;
    /*
    if (pGame->pTexture != NULL) {
        SDL_DestroyTexture(pGame->pTexture);
        pGame->pTexture = NULL; 
    }
    */
    destroyBackground(pGame->pBackground);
    // Stop and free game music
    Mix_HaltMusic();
    Mix_FreeMusic(pGame->pGameMusic);
    if (pGame->pRenderer != NULL) {
        SDL_DestroyRenderer(pGame->pRenderer);
        pGame->pRenderer = NULL;
    }

    if (pGame->pWindow != NULL) {
        SDL_DestroyWindow(pGame->pWindow);
        pGame->pWindow = NULL;
    }

    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        if (pGame->pPlayer[i] != NULL) {
            destroyPlayer(pGame->pPlayer[i]);
            pGame->pPlayer[i] = NULL; 
        }
        
    }
    /*
    for (int i=0; i<MAX_NROFPLAYERS; i++) {
        if (pGame->pPlayers[i] != NULL) {
            destroyPlayer(pGame->pPlayer[i]);
            pGame->pPlayer[i] = NULL; 
        }
    }
    */

    // Här lägger vi till mer kod som frigör tidigare allokerat minne ifall det behövs (t.ex. för platforms sen)
    if (pGame->pBlock != NULL)
    {
        destroyBlock(pGame->pBlock);
        pGame->pBlock = NULL;
    }
    if (pGame->pBlockImage)
    {
        destroyBlockImage(pGame->pBlockImage);
        pGame->pBlockImage = NULL;
    }
    for (int i = 0; i < NROFMAPS; i++)
    {
        if (pGame->pMaps[i] != NULL)
        {
            destroyMap(pGame->pMaps[i]);
            pGame->pMaps[i] = NULL; // skyddar mot dubbel-free
        }
    }
    
    // Nu har jag lagt in blocks
    SDL_Quit();
}