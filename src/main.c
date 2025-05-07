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
#include "../include/camera.h"
#include "../include/common.h"
#include <SDL_net.h>

#define TARGET_ASPECT_RATIO (16.0f / 9.0f)

typedef struct
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Rect screenRect;
    Background *pBackground;
    Audio *pAudio;
    SDL_Cursor *pCursor;
    Player *pPlayer[MAX_NROFPLAYERS];
    Block *pBlock;
    Camera *pCamera;
    Lava *pLava;
} Game;

int initSDL();
void cleanUpSDL();
int initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **p, UDPpacket **p2, int *is_server, int argc, char *argv[]);
void cleanUpNetwork(UDPsocket *sd, UDPpacket **p, UDPpacket **p2);
int initGameBeforeMenu(Game *pGame);
void initScreenRect(Game *pGame);
int initGameAfterMenu(Game *pGame);
void cleanUpGame(Game *pGame);
void readMap(int (*map)[BOX_COL]);

void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow, bool *pUp, bool *pDown, bool *pLeft, bool *pRight);

int main(int argc, char *argv[])
{
    printf("\n \n");
    if (!initSDL())
    {
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    UDPsocket sd;
    IPaddress srvadd;
    UDPpacket *p, *p2;
    int is_server = 0;

    if (!initNetwork(&sd, &srvadd, &p, &p2, &is_server, argc, argv))
    {
        cleanUpNetwork(&sd, &p, &p2);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    Game game = {0};
    if (!initGameBeforeMenu(&game))
    {
        cleanUpGame(&game);
        cleanUpNetwork(&sd, &p, &p2);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    if (!runMenu(game.pRenderer, &game.screenRect))
    {
        cleanUpGame(&game);
        cleanUpNetwork(&sd, &p, &p2);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    if (!initGameAfterMenu(&game))
    {
        cleanUpGame(&game);
        cleanUpNetwork(&sd, &p, &p2);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    // flytta ev in dessa initieringar i initGameAfterMenu
    SDL_Rect blockRect = getBlockRect(game.pBlock);
    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        game.pPlayer[i] = createPlayer(i, game.pRenderer, &game.screenRect);
        initStartPosition(game.pPlayer[i], blockRect);
    }
    game.pCamera = createCamera(&game.screenRect);
    if (!game.pPlayer[0])
    {
        cleanUpGame(&game);
        return 1;
    }

    playMusic(game.pAudio);
    bool closeWindow = false;
    bool up, down, left, right, goUp, goDown, goLeft, goRight;
    bool onGround = true;
    up = down = left = right = false;
    int upCounter = 0, chosenMap = 0, gameMap[BOX_ROW][BOX_COL] = {0};
    float shiftLength = getShiftLength(game.pBlock);

    Uint32 lastTime = SDL_GetTicks(); // Tidpunkt för senaste uppdateringen
    Uint32 currentTime;
    float deltaTime;

    readMap(gameMap);

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
        setAnimation(game.pPlayer[1]);
        updatePlayer(game.pPlayer, deltaTime, gameMap, blockRect, &upCounter, &onGround, &goUp, &goDown,
                     &goLeft, &goRight, p, p2, &is_server, srvadd, &sd, game.screenRect.h, shiftLength);

        int PlyY = game.screenRect.y;
        for (int i = 0; i < MAX_NROFPLAYERS; i++)
        {
            int PlyYtemp = getPlyY(game.pPlayer[i]);
            if (PlyY > PlyYtemp)
            {
                PlyY = PlyYtemp;
            }
        }

        int CamX = getCamX(game.pCamera), CamY = getCamY(game.pCamera), PlyX = getPlyX(game.pPlayer[0]);
        updateCamera(game.pCamera, PlyX, PlyY);
        SDL_RenderClear(game.pRenderer);
        drawBackground(game.pBackground);
        buildTheMap(gameMap, game.pBlock, CamY, &game.screenRect);

        for (int i = 0; i < MAX_NROFPLAYERS; i++)
        {
            drawPlayer(game.pPlayer[i], CamX, CamY);
        }
        
        drawLava(game.pLava);
        drawPadding(game.pRenderer, game.screenRect); // fyller ut med svarta kanter
        SDL_RenderPresent(game.pRenderer);
        SDL_Delay(1); // Undvik 100% CPU-användning men låt SDL hantera FPS
    }

    cleanUpGame(&game);
    cleanUpNetwork(&sd, &p, &p2);
    cleanUpSDL();

    return 0;
}

int initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error initializing SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("Error initializing IMG_Init: %s\n", IMG_GetError());
        return 0;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Error initializing Mix_OpenAudio: %s\n", Mix_GetError());
        return 0;
    }
    return 1;
}

void cleanUpSDL()
{
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

int initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **p, UDPpacket **p2, int *is_server, int argc, char *argv[])
{
    *is_server = 0;

    if (argc > 1 && strcmp(argv[1], "server") == 0)
    {
        *is_server = 1;
    }

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    *sd = NULL;
    *p = NULL;
    *p2 = NULL;

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

    if (!((*p = SDLNet_AllocPacket(512)) && (*p2 = SDLNet_AllocPacket(512))))
    {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    return 1;
}

void cleanUpNetwork(UDPsocket *sd, UDPpacket **p, UDPpacket **p2)
{
    if (*p != NULL)
    {
        SDLNet_FreePacket(*p);
        *p = NULL;
    }
    if (*p2 != NULL)
    {
        SDLNet_FreePacket(*p2);
        *p2 = NULL;
    }
    if (*sd != NULL)
    {
        SDLNet_UDP_Close(*sd);
        *sd = NULL;
    }
    SDLNet_Quit();
}

int initGameBeforeMenu(Game *pGame)
{
    pGame->pWindow = SDL_CreateWindow("KungligaProject", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pGame->pWindow)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        return 0;
    }

    initScreenRect(pGame);

    return 1;
}

void initScreenRect(Game *pGame)
{
    int window_width, window_height;

    SDL_GetWindowSize(pGame->pWindow, &window_width, &window_height);
    float currentAspect = (float)window_width / window_height;
    int targetWidth = window_width;
    int targetHeight = window_height;

    if (currentAspect > TARGET_ASPECT_RATIO)
    {
        targetWidth = (int)(window_height * TARGET_ASPECT_RATIO + 0.5f);
    }
    else if (currentAspect < TARGET_ASPECT_RATIO)
    {
        targetHeight = (int)(window_width / TARGET_ASPECT_RATIO + 0.5f);
    }

    pGame->screenRect.w = (int)(targetWidth + 0.5f);
    pGame->screenRect.h = (int)(targetHeight + 0.5f);
    pGame->screenRect.x = (int)((window_width - targetWidth) / 2 + 0.5f);
    pGame->screenRect.y = (int)((window_height - targetHeight) / 2 + 0.5f);

    printf("Original window size: w: %d, h: %d \n", window_width, window_height);
    printf("screenRect: x=%d, y=%d, w=%d, h=%d\n", pGame->screenRect.x, pGame->screenRect.y, pGame->screenRect.w, pGame->screenRect.h);
}

int initGameAfterMenu(Game *pGame)
{

    pGame->pBackground = createBackground(pGame->pRenderer, &pGame->screenRect, GAME);
    if (!pGame->pBackground)
    {
        printf("Error in initGameAfterMenu: pGame->pBackground is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }

    pGame->pLava = createLava(pGame->pRenderer, &pGame->screenRect);
    if (!pGame->pLava)
    {
        printf("Error in initGameAfterMenu: pGame->pLava is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }

    pGame->pAudio = createAudio(GAME);
    if (!pGame->pAudio)
    {
        printf("Error in initGameAfterMenu: pGame->pAudio is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }

    pGame->pCursor = initCursor();
    if (!pGame->pCursor)
    {
        printf("Error in initGameAfterMenu: pGame->pCursor is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }
    SDL_SetCursor(pGame->pCursor);
    SDL_ShowCursor(SDL_DISABLE); // Gör muspekaren osynlig tills vidare

    pGame->pBlock = createBlock(pGame->pRenderer, &pGame->screenRect);
    if (!pGame->pBlock)
    {
        cleanUpGame(pGame);
        printf("Error creating pBlock: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

void cleanUpGame(Game *pGame)
{
    if (pGame->pBackground)
    {
        destroyBackground(pGame->pBackground);
        pGame->pBackground = NULL;
    }

    if (pGame->pLava)
    {
        destroyLava(pGame->pLava);
        pGame->pLava = NULL;
    }

    if (pGame->pAudio)
    {
        destroyAudio(pGame->pAudio);
        pGame->pAudio = NULL;
    }

    if (pGame->pCursor)
    {
        destroyCursor(pGame->pCursor);
        pGame->pCursor = NULL;
    }

    if (pGame->pRenderer)
    {
        SDL_DestroyRenderer(pGame->pRenderer);
        pGame->pRenderer = NULL;
    }

    if (pGame->pWindow)
    {
        SDL_DestroyWindow(pGame->pWindow);
        pGame->pWindow = NULL;
    }

    // Raderna under behöver vi gå igenom mer noggrant sen
    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        if (pGame->pPlayer[i] != NULL)
        {
            destroyPlayer(pGame->pPlayer[i]);
            pGame->pPlayer[i] = NULL;
        }
    }

    if (pGame->pBlock != NULL)
    {
        destroyBlock(pGame->pBlock);
        pGame->pBlock = NULL;
    }
    if (pGame->pCamera != NULL)
    {
        destroyCamera(pGame->pCamera);
        pGame->pCamera = NULL;
    }
}

void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow,
                 bool *pUp, bool *pDown, bool *pLeft, bool *pRight)
{
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
            playJumpSound(pGame->pAudio);
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
            // Pop up ruta??
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

void readMap(int (*map)[BOX_COL])
{
    FILE *fp;
    char tmp[BOX_COL + 1];

    fp = fopen("map.txt", "r");

    if (fp != NULL)
    {
        for (int i = 0; i < BOX_ROW; i++)
        {
            fscanf(fp, "%s", tmp);
            for (int j = 0; j < BOX_COL; j++)
            {
                map[i][j] = tmp[j] - 48;
            }
        }
    }
    else
        printf("no map file\n");
}