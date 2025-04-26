#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>

#include "../include/menu.h"
#include "../include/platform.h"
#include "../include/player.h"
#include "../include/theme.h"
#include "../include/maps.h"
#include "../include/camera.h"
#include <SDL_net.h>

#define NUM_MENU 2
#define TARGET_ASPECT_RATIO (16.0f / 9.0f)

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
    Camera *pCamera;
} Game;

typedef struct
{
    int window_width, window_height;
    int modified_width, modified_height;
    SDL_Rect topRect, bottomRect, leftRect, rightRect;
} DisplayMode;


void initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **p, UDPpacket **p2, int *is_server, int argc, char *argv[]);
void initGame(Game *pGame);
void initDisplayMode(Game *pGame, DisplayMode *pDisplay);

void drawScreenPadding(Game *pGame, DisplayMode *pDisplay);     // Anropas ej i nuläget - kan komma att användas senare 
void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow, bool *pUp, bool *pDown, bool *pLeft, bool *pRight);
void cleanUpNetwork(UDPsocket *sd, UDPpacket **p, UDPpacket **p2);
void cleanUpGame(Game *pGame);


int main(int argc, char *argv[])
{
    Game game = {0};
    DisplayMode display = {0};
    bool startGame = false;

    UDPsocket sd;
    IPaddress srvadd;
    UDPpacket *p, *p2;
    int is_server = 0;
    
    initNetwork(&sd, &srvadd, &p, &p2, &is_server, argc, argv);
    initGame(&game);
    initDisplayMode(&game, &display);

    if (!showMenu(game.pRenderer, display.window_width, display.window_height))
    {
        cleanUpGame(&game);
        return 1;
    }

    game.pGameMusic = initiateMusic(game.pGameMusic);
    game.pBackground = createBackground(game.pRenderer, display.window_width, display.window_height);
    game.pBlockImage = createBlockImage(game.pRenderer);
    game.pBlock = createBlock(game.pBlockImage, display.window_width, display.window_height);
    SDL_Rect blockRect = getRectBlock(game.pBlock);
    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        game.pPlayer[i] = createPlayer(i, blockRect, game.pRenderer, display.window_width, display.window_height);
    }
    game.pCamera = camera(display.window_width, display.window_height);
    if (!game.pGameMusic || !game.pBackground || !game.pBlockImage || !game.pPlayer)
    {
        cleanUpGame(&game);
        return 1;
    }
    bool closeWindow = false;
    bool up, down, left, right, goUp, goDown, goLeft, goRight;
    bool onGround = true;
    up = down = left = right = false;
    int upCounter = 0, chosenMap = 0, gameMap[BOX_ROW][BOX_COL] = {0};

    Uint32 lastTime = SDL_GetTicks(); // Tidpunkt för senaste uppdateringen
    Uint32 currentTime;
    float deltaTime;
    drawBlueprints(game.pMaps);
    chooseMap(gameMap, game.pMaps[chosenMap]);
    while (!closeWindow)
    {
        // Beräkna tid sedan senaste frame
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; // Omvandla till sekunder
        lastTime = currentTime;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                closeWindow = true;
            else
                handleInput(&game, &event, &closeWindow, &up, &down, &left, &right);
        }
        goDown = goLeft = goRight = goUp = 0;
        setSpeed(up, down, left, right, &goUp, &goDown, &goLeft, &goRight, &upCounter, onGround, game.pPlayer[0]);
        updatePlayer(game.pPlayer, deltaTime, gameMap, blockRect, &upCounter, &onGround, &goUp, &goDown, &goLeft, &goRight, p, p2, &is_server, srvadd, &sd);
        // updatePlayer(game.pPlayer, blockRect);
        int CamX = getCamX(game.pCamera), CamY = getCamY(game.pCamera), PlyX = getPlyX(game.pPlayer[0]), PlyY = getPlyY(game.pPlayer[0]);
        updateCamera(game.pCamera, PlyX, PlyY);
        SDL_RenderClear(game.pRenderer);
        drawBackground(game.pBackground, CamX, CamY);
        buildTheMap(gameMap, game.pBlock, CamY);
        for (int i = 0; i < MAX_NROFPLAYERS; i++)
        {
            drawPlayer(game.pPlayer[i], CamX, CamY, display.window_width, display.window_height);
        }

        SDL_RenderPresent(game.pRenderer);
        SDL_Delay(1); // Undvik 100% CPU-användning men låt SDL hantera FPS
    }

    cleanUpNetwork(&sd, &p, &p2);
    cleanUpGame(&game);
    return 0;
}

void initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **p, UDPpacket **p2, int *is_server, int argc, char *argv[]) {
    *is_server = 0;

    if (argc > 1 && strcmp(argv[1], "server") == 0)
    {
        *is_server = 1;
    }

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    *sd = NULL;
    *p = NULL;
    *p2 = NULL;

    if (!(*sd = SDLNet_UDP_Open(*is_server ? 2000 : 0)))
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        cleanUpNetwork(sd, p, p2);
        exit(EXIT_FAILURE);
    }

    if (!(*is_server))
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: %s client <server_ip>\n", argv[0]);
            cleanUpNetwork(sd, p, p2);
            exit(EXIT_FAILURE);
        }

        if (SDLNet_ResolveHost(srvadd, argv[2], 2000) == -1)
        {
            fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            cleanUpNetwork(sd, p, p2);
            exit(EXIT_FAILURE);
        }
    }

    if (!((*p = SDLNet_AllocPacket(512)) && (*p2 = SDLNet_AllocPacket(512))))
    {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        cleanUpNetwork(sd, p, p2);
        exit(EXIT_FAILURE);
    }
}

void initGame(Game *pGame)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    // Initialize SDL_image for PNG loading
    int iconImage = IMG_INIT_PNG;
    if (!(IMG_Init(iconImage) & iconImage))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    // Create window with explicit cursor support
    pGame->pWindow = SDL_CreateWindow("Lavan?", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pGame->pWindow)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    pGame->pJumpSound = NULL; // Initialize jump sound to NULL

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    // Initialize SDL_image for cursor loading
    int cursor = IMG_INIT_PNG;
    if (!(IMG_Init(cursor) & cursor))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        cleanUpGame(pGame);
        exit(EXIT_FAILURE);
    }

    // Force cursor to be visible first
    SDL_ShowCursor(SDL_ENABLE);

    // Load and set custom cursor
    SDL_Surface *cursorSurface = IMG_Load("resources/cursor.png");
    if (!cursorSurface)
    {
        printf("Failed to load cursor image! SDL_image Error: %s\n", IMG_GetError());
    }
    else
    {
        // Create cursor with hotspot at top-left for better precision
        SDL_Cursor *cursor = SDL_CreateColorCursor(cursorSurface, 0, 0);
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
    if (SDL_ShowCursor(-1) != SDL_ENABLE)
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void initDisplayMode(Game *pGame, DisplayMode *pDisplay) {
    SDL_GetWindowSize(pGame->pWindow, &pDisplay->window_width, &pDisplay->window_height);
    float currentAspect = (float)pDisplay->window_width / pDisplay->window_height;

    pDisplay->modified_width = pDisplay->window_width;
    pDisplay->modified_height = pDisplay->window_height;

    if (currentAspect > TARGET_ASPECT_RATIO) { // Om skärmen är "för bred" - fyller ut med svarta fält på sidor
        int targetWidth = (int)(pDisplay->window_height * TARGET_ASPECT_RATIO);
        int sidePadding = (pDisplay->window_width - targetWidth) / 2;

        pDisplay->leftRect = (SDL_Rect){0, 0, sidePadding, pDisplay->window_height};
        pDisplay->rightRect = (SDL_Rect){pDisplay->window_width - sidePadding, 0, sidePadding, pDisplay->window_height};
        pDisplay->topRect = (SDL_Rect){0, 0, 0, 0};
        pDisplay->bottomRect = (SDL_Rect){0, 0, 0, 0};

        pDisplay->modified_width = targetWidth;
    }
    else if (currentAspect < TARGET_ASPECT_RATIO) { // Om skärmen är "för hög" - fyller ut med svarta fält ovanför och under
        int targetHeight = (int)(pDisplay->window_width / TARGET_ASPECT_RATIO);
        int verticalPadding = (pDisplay->window_height - targetHeight) / 2;

        pDisplay->topRect = (SDL_Rect){0, 0, pDisplay->window_width, verticalPadding};
        pDisplay->bottomRect = (SDL_Rect){0, pDisplay->window_height - verticalPadding, pDisplay->window_width, verticalPadding};
        pDisplay->leftRect = (SDL_Rect){0, 0, 0, 0};
        pDisplay->rightRect = (SDL_Rect){0, 0, 0, 0};

        pDisplay->modified_height = targetHeight;
    }
    else {  // Rätt aspect ratio – inga svarta fält behövs
        pDisplay->topRect = pDisplay->bottomRect = pDisplay->leftRect = pDisplay->rightRect = (SDL_Rect){0, 0, 0, 0};
    }

    // Skrivs ut i terminalen för att kontrollera siffrorna - behåll nu tills vidare
    printf("Original window width: %d \n", pDisplay->window_width);
    printf("Original window height: %d \n", pDisplay->window_height);
    printf("Modified window width: %d \n", pDisplay->modified_width); 
    printf("Modified window height: %d \n", pDisplay->modified_height);
    printf("topRect: x=%d, y=%d, w=%d, h=%d\n", pDisplay->topRect.x, pDisplay->topRect.y, pDisplay->topRect.w, pDisplay->topRect.h);
    printf("bottomRect: x=%d, y=%d, w=%d, h=%d\n", pDisplay->bottomRect.x, pDisplay->bottomRect.y, pDisplay->bottomRect.w, pDisplay->bottomRect.h);
    printf("leftRect: x=%d, y=%d, w=%d, h=%d\n", pDisplay->leftRect.x, pDisplay->leftRect.y, pDisplay->leftRect.w, pDisplay->leftRect.h);
    printf("rightRect: x=%d, y=%d, w=%d, h=%d\n", pDisplay->rightRect.x, pDisplay->rightRect.y, pDisplay->rightRect.w, pDisplay->rightRect.h);
}

void drawScreenPadding(Game *pGame, DisplayMode *pDisplay) {
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    
    if (pDisplay->topRect.w > 0 && pDisplay->topRect.h > 0) {
        SDL_RenderFillRect(pGame->pRenderer, &pDisplay->topRect);
    }
    if (pDisplay->bottomRect.w > 0 && pDisplay->bottomRect.h > 0) {
        SDL_RenderFillRect(pGame->pRenderer, &pDisplay->bottomRect);
    }
    if (pDisplay->leftRect.w > 0 && pDisplay->leftRect.h > 0) {
        SDL_RenderFillRect(pGame->pRenderer, &pDisplay->leftRect);
    }
    if (pDisplay->rightRect.w > 0 && pDisplay->rightRect.h > 0) {
        SDL_RenderFillRect(pGame->pRenderer, &pDisplay->rightRect);
    }
}

void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow,
                 bool *pUp, bool *pDown, bool *pLeft, bool *pRight)
{
    // First time jumping? I'll load the sound - then it's ready for next time
    if (!pGame->pJumpSound)
    {
        pGame->pJumpSound = Mix_LoadWAV("resources/jump_sound.wav");
        if (!pGame->pJumpSound)
        {
            printf("Failed to load jump sound! SDL_mixer Error: %s\n", Mix_GetError());
        }
    }

    if (pEvent->type == SDL_KEYDOWN)
    {
        switch (pEvent->key.keysym.scancode)
        {
        default: // Handle any unspecified keys
            break;

        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_SPACE:
            (*pUp) = true;
            if (pGame->pJumpSound)
            {
                Mix_PlayChannel(-1, pGame->pJumpSound, 0); // Play jump sound
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
            if (pGame->pJumpSound)
            {
                Mix_FreeChunk(pGame->pJumpSound); // Clean up jump sound
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

void cleanUpNetwork(UDPsocket *sd, UDPpacket **p, UDPpacket **p2) {
    if (*p != NULL) {
        SDLNet_FreePacket(*p);
        *p = NULL; 
    }
    if (*p2 != NULL) {
        SDLNet_FreePacket(*p2);
        *p2 = NULL; 
    }
    if (*sd != NULL) {
        SDLNet_UDP_Close(*sd);
        *sd = NULL; 
    }
    SDLNet_Quit();
}

void cleanUpGame(Game *pGame)
{
    if (pGame == NULL)
        return;
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
    if (pGame->pRenderer != NULL)
    {
        SDL_DestroyRenderer(pGame->pRenderer);
        pGame->pRenderer = NULL;
    }

    if (pGame->pWindow != NULL)
    {
        SDL_DestroyWindow(pGame->pWindow);
        pGame->pWindow = NULL;
    }

    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        if (pGame->pPlayer[i] != NULL)
        {
            destroyPlayer(pGame->pPlayer[i]);
            pGame->pPlayer[i] = NULL;
        }
    }
    if (pGame->pCamera != NULL)
    {
        destroyCamera(pGame->pCamera);
        pGame->pCamera = NULL;
    }

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