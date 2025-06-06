#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>
#include <SDL_net.h>

#include "../include/menu.h"
#include "../include/platform.h"
#include "../include/player.h"
#include "../include/theme.h"
#include "../include/camera.h"
#include "../include/scaling.h"
#include "../include/net.h"
#include "../include/video_player.h"

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
int initGameBeforeMenu(Game *pGame);
int initGameAfterMenu(Game *pGame);
int initGameWin(Game *pGame);
int initGameLose(Game *pGame);
void cleanUpGame(Game *pGame);
bool readMap(int (*map)[BOX_COL]);
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
    UDPpacket *sendPacket, *receivePacket;
    Game game = {0};
    int is_server = 0, my_id = -1, nrOfPlayers = 0, playerjoined = 1;
    int PlayerActive = 0;
    if (!initNetwork(&sd, &srvadd, &sendPacket, &receivePacket, &is_server, argc, argv, &my_id))
    {
        cleanUpNetwork(&sd, &sendPacket, &receivePacket);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    if (!initGameBeforeMenu(&game))
    {
        cleanUpGame(&game);
        cleanUpNetwork(&sd, &sendPacket, &receivePacket);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    // Play the intro video first, before showing the menu
    #ifdef USE_FFMPEG
    if (!playIntroVideo(game.pRenderer)) {
        fprintf(stderr, "Failed to play intro video\n");
        // Continue even if video fails
    }
    #endif

    // Only show menu after video has finished playing
    if (!runMenu(game.pRenderer, &game.screenRect))
    {
        cleanUpGame(&game);
        cleanUpNetwork(&sd, &sendPacket, &receivePacket);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }

    if (!initGameAfterMenu(&game))
    {
        cleanUpGame(&game);
        cleanUpNetwork(&sd, &sendPacket, &receivePacket);
        cleanUpSDL();
        exit(EXIT_FAILURE);
    }
    Movecheck movecheck = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    SDL_Rect ScreenSize = getScreenRect(game.pWindow);

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
    connectToServer(game.pPlayer, is_server, &my_id, srvadd, &nrOfPlayers, sendPacket, receivePacket, sd, &playerjoined);
    playMusic(game.pAudio);
    bool closeWindow = false;
    float shiftX = getShiftX(game.pBlock);
    float shiftY = getShiftY(game.pBlock);
    Uint32 lastTime = SDL_GetTicks(); 
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
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; 
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
        setSpeed(game.pPlayer[my_id], &movecheck);
        for (int i = 1; i < MAX_NROFPLAYERS; i++)
        {
            setAnimation(game.pPlayer[i]);
        }
        Offsets offset = setOffsets(game.screenRect, my_id, shiftX);
        networkUDP(game.pPlayer, my_id, sendPacket, receivePacket, is_server, srvadd, sd, &nrOfPlayers);
        updatePlayer(game.pPlayer, offset, my_id, deltaTime, gameMap, blockRect, &movecheck);
        float PlyY = game.screenRect.y;
        for (int i = 0; i < MAX_NROFPLAYERS; i++)
        {
            float PlyYtemp = getPlyY(game.pPlayer[i]);
            if (PlyY > PlyYtemp)
            {
                PlyY = PlyYtemp;
            }
        }
        int CamX = getCamX(game.pCamera), CamY = getCamY(game.pCamera), PlyX = getPlyRectX(game.pPlayer[0]);
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
        int controll = 0;
        for (int i = 0; i < MAX_NROFPLAYERS; i++)
        {
            float playerY = getPlyY(game.pPlayer[i]);
            if (playerY > CamY + ScreenSize.h + 10 && getPlayerActive(game.pPlayer[i]))
            {
                printf(" Playeralive: %d", PlayerActive);
                SetAlivefalse(game.pPlayer[i]);
                controll++;
            }
        }

        PlayerActive = controll;

        bool waitingForExit = false;
        if (PlayerActive == 1 && playerjoined)
        {
            printf(" Playeralive: %d", PlayerActive);
            if (!initGameWin(&game))
            {
                cleanUpGame(&game);
                cleanUpNetwork(&sd, &sendPacket, &receivePacket);
                cleanUpSDL();
                exit(EXIT_FAILURE);
            }
            SDL_RenderClear(game.pRenderer);
            drawBackground(game.pBackground);
            SDL_RenderPresent(game.pRenderer);
            waitingForExit = true;
        }

        if (!getAlive(game.pPlayer[my_id]))
        {
            setActive(game.pPlayer[my_id], false);

            if (!initGameLose(&game))
            {
                cleanUpGame(&game);
                cleanUpNetwork(&sd, &sendPacket, &receivePacket);
                cleanUpSDL();
                exit(EXIT_FAILURE);
            }
            SDL_RenderClear(game.pRenderer);
            drawBackground(game.pBackground);
            SDL_RenderPresent(game.pRenderer);
            waitingForExit = true;
        }

        while (waitingForExit)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    waitingForExit = false;
                    closeWindow = true;
                }
                else if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        waitingForExit = false;
                        closeWindow = true;
                    }
                }
            }

            SDL_RenderClear(game.pRenderer);
            drawBackground(game.pBackground);
            SDL_RenderPresent(game.pRenderer);
            SDL_Delay(16); 
        }
    }

    cleanUpGame(&game);
    cleanUpNetwork(&sd, &sendPacket, &receivePacket);
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

int initGameBeforeMenu(Game *pGame)
{
    pGame->pWindow = SDL_CreateWindow("LavaRun", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
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

int initGameWin(Game *pGame)
{
    pGame->pBackground = createBackground(pGame->pRenderer, &pGame->screenRect, WIN);
    if (!pGame->pBackground)
    {
        printf("Error in initGameWin: pGame->pBackground is NULL.\n");
        cleanUpGame(pGame);
        return 0;
    }
    return 1;
}

int initGameLose(Game *pGame)
{
    pGame->pBackground = createBackground(pGame->pRenderer, &pGame->screenRect, LOSE);
    if (!pGame->pBackground)
    {
        printf("Error in initGameLose: pGame->pBackground is NULL.\n");
        cleanUpGame(pGame);
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

void handleInput(Game *pGame, SDL_Event *pEvent, bool *pCloseWindow, Movecheck *movecheck)
{
    if (pEvent->type == SDL_KEYDOWN)
    {
        switch (pEvent->key.keysym.scancode)
        {
        default: 
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
        default:
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