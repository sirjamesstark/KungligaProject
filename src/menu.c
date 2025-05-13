#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/menu.h"
#include "../include/theme.h"
#include "../include/scaling.h"
#include "../include/lobby.h"

struct menu
{
    Background *pBackground;

    Button *pButton[NROFBUTTONS_MENU];
    Audio *pAudio;
    Cursor *pCursor;
};

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect)
{
    if (!pRenderer || !pScreenRect)
    {
        printf("Error in createMenu: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Menu *pMenu = malloc(sizeof(struct menu));
    if (!pMenu)
    {
        printf("Error in createMenu: Failed to allocate memory for pMenu.\n");
        return NULL;
    }

    pMenu->pBackground = createBackground(pRenderer, pScreenRect, MENU);
    if (!pMenu->pBackground)
    {
        printf("Error in createMenu: pMenu->pBackground is NULL.\n");
        destroyMenu(pMenu);
        return NULL;
    }

    for (int i = 0; i < NROFBUTTONS_MENU; i++) {
        pMenu->pButton[i] = createButton(pRenderer, pScreenRect, i);
        if (!pMenu->pButton[i])
        {
            printf("Error in createMenu: pMenu->pButton[%d] is NULL.\n", i);
            destroyMenu(pMenu);
            return NULL;
        }
    }

    pMenu->pAudio = createAudio(MENU);
    if (!pMenu->pAudio)
    {
        printf("Error in createMenu: pMenu->pAudio is NULL.\n");
        destroyMenu(pMenu);
        return NULL;
    }

    pMenu->pCursor = createCursor();
    if (!pMenu->pCursor)
    {
        printf("Error in createMenu: pMenu->pCursor is NULL.\n");
        destroyMenu(pMenu);
        return NULL;
    }

    return pMenu;
}

void destroyMenu(Menu *pMenu)
{
    if (!pMenu)
        return;

    if (pMenu->pBackground)
    {
        destroyBackground(pMenu->pBackground);
        pMenu->pBackground = NULL;
    }

    for (int i = 0; i < NROFBUTTONS_MENU; i++) {
        if (pMenu->pButton[i]) {
            destroyButton(pMenu->pButton[i]);
            pMenu->pButton[i] = NULL;
        }
    }

    if (pMenu->pAudio)
    {
        destroyAudio(pMenu->pAudio);
        pMenu->pAudio = NULL;
    }

    if (pMenu->pCursor)
    {
        destroyCursor(pMenu->pCursor);
        pMenu->pCursor = NULL;
    }

    free(pMenu);
}

bool runMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, char IPinput[15]) {
    Menu *pMenu = createMenu(pRenderer, pScreenRect);
    if (!pMenu)
        return false;

    bool menuRunning = true;
    bool startGame   = false;
    bool runLobby    = false;

    SDL_Event event;
    while (menuRunning) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    menuRunning = false;
                    startGame   = false;
                    runLobby    = false;
                    break;
                case SDL_KEYDOWN:
                    handleKey(&event, pMenu, &menuRunning, &startGame, &runLobby);
                    break;
                case SDL_MOUSEMOTION:
                case SDL_MOUSEBUTTONDOWN:
                    handleMouse(&event, pMenu, &menuRunning, &startGame, &runLobby);
                    break;
                default:
                    break;
            }
        }
        drawBackground(pMenu->pBackground);
        playMusic(pMenu->pAudio);

        if (runLobby){
            printf("runLobby true\n");
            for (int i = 0; i < NROFBUTTONS_MENU; i++) {
                if (pMenu->pButton[i]) {
                    destroyButton(pMenu->pButton[i]);
                    pMenu->pButton[i] = NULL;
                }
            }

            if (runningLobby(pRenderer,pScreenRect,&menuRunning,&startGame,pMenu->pAudio,pMenu->pBackground,IPinput,pMenu->pCursor)){
                printf("Error in lobby\n");
                return false;
            }
            runLobby = false;

            if (menuRunning){
                for (int i = 0; i < NROFBUTTONS_MENU; i++) {
                    pMenu->pButton[i] = createButton(pRenderer, pScreenRect, i);
                    if (!pMenu->pButton[i]) {
                        printf("Error in createMenu: pMenu->pButton[%d] is NULL.\n", i);
                        destroyMenu(pMenu);
                        return NULL;
                    }
                } 
            }
        }

        if (isMusicMuted(pMenu->pAudio)) makeButtonHoverd(pMenu->pButton[SOUND]); 
        for (int i = 0; i < NROFBUTTONS_MENU; i++) {
            drawButton(pMenu->pButton[i]);
        }
        drawPadding(pRenderer, *pScreenRect);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(1);
    }

    SDL_Delay(100);
    destroyMenu(pMenu);
    return startGame;
}

void handleKey(SDL_Event *pEvent, Menu *pMenu, bool *pMenuRunning, bool *pStartGame, bool *pRunLobby) {
    static int buttonSelected = START;

    switch (pEvent->key.keysym.scancode) {
        case SDL_SCANCODE_UP:
            if (buttonSelected != START) buttonSelected--;  
            break;
        case SDL_SCANCODE_DOWN:
            if (buttonSelected != JOIN) buttonSelected++;
            break;
        case SDL_SCANCODE_SPACE:
        case SDL_SCANCODE_RETURN:
            handlePushedButton(pMenu, pMenuRunning, pStartGame, pRunLobby);
            break;
        case SDL_SCANCODE_ESCAPE:
            *pMenuRunning = false;
            *pStartGame = false;
            break;
        default:
            for (int i = 0; i < NROFBUTTONS_MENU; i++){
                makeButtonNotHovered(pMenu->pButton[i]);
            }
            break;
    }

    for (int i = 0; i < NROFBUTTONS_MENU; i++)
    {
        if (i == buttonSelected) toggleHoveredButton(pMenu->pButton[i]);
        else makeButtonNotHovered(pMenu->pButton[i]);
    }
    
}

void handleMouse(SDL_Event *pEvent, Menu *pMenu, bool *pMenuRunning, bool *pStartGame, bool *pRunLobby) {
    int x = pEvent->motion.x;
    int y = pEvent->motion.y;

    switch (pEvent->type) {
        case SDL_MOUSEMOTION:
            for (int i = 0; i < NROFBUTTONS_MENU; i++) {
                setButton_isHovered(pMenu->pButton[i], pMenu->pCursor);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (pEvent->button.button == SDL_BUTTON_LEFT) {
                handlePushedButton(pMenu, pMenuRunning, pStartGame, pRunLobby);
            }
            break;
    }
}

void handlePushedButton(Menu *pMenu, bool *pMenuRunning, bool *pStartGame, bool *pRunLobby) {
    if (isButtonPushed(pMenu->pButton[START])) {
        playButtonSound(pMenu->pAudio);
        *pMenuRunning = false;
        *pStartGame = true;
    }
    else if (isButtonPushed(pMenu->pButton[EXIT]))
    {
        playButtonSound(pMenu->pAudio);
        *pMenuRunning = false;
        *pStartGame = false;
    }
    else if (isButtonPushed(pMenu->pButton[SOUND]))
    {
        playButtonSound(pMenu->pAudio);
        toggleMuteAudio(pMenu->pAudio);
        toggleHoveredButton(pMenu->pButton[SOUND]);
        *pMenuRunning = true;
        *pStartGame = false;
    }
    else if (isButtonPushed(pMenu->pButton[JOIN])) {
        playButtonSound(pMenu->pAudio);
        *pMenuRunning = true;
        *pStartGame = false;
        *pRunLobby = true;
    }
}