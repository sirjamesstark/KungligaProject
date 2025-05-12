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
#include "../include/scaling.h"
#include <SDL_net.h>

typedef struct
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Rect screenRect;
    Background *pBackground;
    Lava *pLava;
    Audio *pAudio;
    Cursor *pCursor;
    Camera *pCamera;
    Block *pBlock;
    Player *pPlayer[MAX_NROFPLAYERS];
} Game;

int initSDL();
void cleanUpSDL();
int initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **p, UDPpacket **p2, int *is_server, int argc, char *argv[]);
void cleanUpNetwork(UDPsocket *sd, UDPpacket **p, UDPpacket **p2);
int initGameBeforeMenu(Game *pGame);
int initGameAfterMenu(Game *pGame);
void cleanUpGame(Game *pGame);
bool readMap(int (*map)[BOX_COL]);
Movecheck setMoveCheck();
void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow, Movecheck *movecheck);

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
    Movecheck movecheck = setMoveCheck();

    // flytta ev in dessa initieringar i initGameAfterMenu
    SDL_Rect blockRect = getBlockRect(game.pBlock);
    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        game.pPlayer[i] = createPlayer(i, game.pRenderer, &game.screenRect);
        initStartPosition(game.pPlayer[i], blockRect);
    }

    if (!game.pPlayer[0])
    {
        cleanUpGame(&game);
        return 1;
    }

    playMusic(game.pAudio);
    bool closeWindow = false;
    float shiftLength = getShiftLength(game.pBlock);

    Uint32 lastTime = SDL_GetTicks(); // Tidpunkt för senaste uppdateringen
    Uint32 currentTime;
    float deltaTime;

    int gameMap[BOX_ROW][BOX_COL] = {0};
    if (!readMap(gameMap))
    {
        cleanUpGame(&game);
        return 1;
    }

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
                handleInput(&game, &event, &closeWindow, &movecheck);
        }

        movecheck.pGoDown = movecheck.pGoLeft = movecheck.pGoRight = movecheck.pGoUp = 0;
        setSpeed(game.pPlayer[0], &movecheck);
        setAnimation(game.pPlayer[1]);
        updatePlayer(game.pPlayer, deltaTime, gameMap, blockRect, p, p2, &is_server, srvadd, &sd, game.screenRect.h, shiftLength, &movecheck);

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
        drawPadding(game.pRenderer, game.screenRect);
        SDL_RenderPresent(game.pRenderer);
        SDL_Delay(1);
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
        printf("Error in initGameBeforeMenu: pGame->pWindow is NULL.\n");
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Error in initGameBeforeMenu: pGame->pRenderer is NULL.\n");
        return 0;
    }

    pGame->screenRect = getScreenRect(pGame->pWindow);

    return 1;
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

    pGame->pCursor = createCursor();
    if (!pGame->pCursor)
    {
        printf("Error in initGameAfterMenu: pGame->pCursor is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }
    toggleCursorVisibility(pGame->pCursor);

    pGame->pCamera = createCamera(&pGame->screenRect);
    if (!pGame->pCamera)
    {
        printf("Error in initGameAfterMenu: pGame->pCamera is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }

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

    if (pGame->pCamera)
    {
        destroyCamera(pGame->pCamera);
        pGame->pCamera = NULL;
    }

    if (pGame->pBlock)
    {
        destroyBlock(pGame->pBlock);
        pGame->pBlock = NULL;
    }

    for (int i = 0; i < MAX_NROFPLAYERS; i++)
    {
        if (pGame->pPlayer[i])
        {
            destroyPlayer(pGame->pPlayer[i]);
            pGame->pPlayer[i] = NULL;
        }
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
}

Movecheck setMoveCheck()
{
    Movecheck movecheck = {0};
    movecheck.up = false;
    movecheck.down = false;
    movecheck.left = false;
    movecheck.right = false;
    movecheck.pGoUp = false;
    movecheck.pGoDown = false;
    movecheck.pGoLeft = false;
    movecheck.pGoRight = false;
    movecheck.pUpCounter = 0;
    movecheck.onGround = true;

    return movecheck;
}

void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow, Movecheck *movecheck)
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
            (movecheck->up) = true;
            playJumpSound(pGame->pAudio);
            break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:
            (movecheck->left) = true;
            break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
            (movecheck->down) = true;
            break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT:
            (movecheck->right) = true;
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
            (movecheck->up) = false;
            break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:
            (movecheck->left) = false;
            break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
            (movecheck->down) = false;
            break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT:
            (movecheck->right) = false;
            break;
        }
    }
}

bool readMap(int (*map)[BOX_COL])
{
    FILE *fp;
    char tmp[BOX_COL + 1];
    int row_count = 0;
    fp = fopen("map.txt", "r");
    printf("\n");

    if (fp)
    {
        for (int i = 0; i < BOX_ROW; i++)
        {
            if (fscanf(fp, "%s", tmp) != 1)
            {
                printf("Error in readMap: Failed to read a line in map file.\n");
                fclose(fp);
                return false;
            }

            if (strlen(tmp) != BOX_COL)
            {
                printf("Error in readMap: Line %d does not have the correct number of columns.\n", i + 1);
                fclose(fp);
                return false;
            }

            for (int j = 0; j < BOX_COL; j++)
            {
                map[i][j] = tmp[j] - 48;
            }
            row_count++;
        }

        if (row_count != BOX_ROW)
        {
            printf("Error in readMap: Expected %d rows, but read %d rows.\n", BOX_ROW, row_count);
            fclose(fp);
            return false;
        }
        printf("Successfully read %d rows from the map file.\n", row_count);
        fclose(fp);
        return true;
    }
    else
    {
        printf("Error in readMap: No map was found\n");
        return false;
    }
}