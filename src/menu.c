#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/menu.h"
#include "../include/theme.h"
#include "../include/common.h"

enum buttons {
    START = 0,
    EXIT =  1,
    SOUND = 2
};

struct button {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
    int nrOfFrames, currentFrame;
    bool is_hovered; 
};

struct menu {
    Theme *pTheme;
    Button *pButton[NROFBUTTONS];
    Mix_Chunk *pSound;
    bool menuRunning, startGame;
};

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect) {
    if (!pRenderer || !pScreenRect) {
        printf("Error in createMenu: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Menu *pMenu = malloc(sizeof(struct menu));
    if (!pMenu) {
        printf("Error in createMenu: Failed to allocate memory for pMenu.\n");
        return NULL;
    }

    pMenu->pTheme = createTheme(pRenderer, pScreenRect, MENU);
    if (!pMenu->pTheme) {
        printf("Error in createMenu: pMenu->pTheme is NULL.\n");
        destroyMenu(pMenu);
        return NULL;
    }

    for (int i = 0; i < NROFBUTTONS; i++) {
        pMenu->pButton[i] = createButton(pRenderer, pScreenRect, i);
        if (!pMenu->pButton[i]) {
            printf("Error in createMenu: pMenu->pButton[%d] is NULL.\n", i);
            destroyMenu(pMenu);
            return NULL;
        }
    }

    pMenu->pSound = Mix_LoadWAV("resources/button_selection_sound.wav");
    if (!pMenu->pSound) {
        printf("Error in createMenu: pMenu->pSound is NULL.\n");
        destroyMenu(pMenu);
        return NULL;
    }

    return pMenu;
}

Button *createButton(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, Buttons button_type) {
    if (!pRenderer || !pScreenRect) {
        printf("Error in createButton: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Button *pButton = malloc(sizeof(struct button));
    if (!pButton) {
        printf("Error in createButton: Failed to allocate memory for pButton.\n");
        return NULL;
    }

    pButton->pRenderer = pRenderer;
    pButton->pScreenRect = pScreenRect;
    pButton->nrOfFrames = 2;
    pButton->currentFrame = 0; 
    pButton->is_hovered = false;

    SDL_Surface *pSurface = NULL;
    if (button_type == START) pSurface = IMG_Load("resources/start_spritesheet.png");
    else if (button_type == EXIT) pSurface = IMG_Load("resources/exit_spritesheet.png");
    else if (button_type == SOUND) pSurface = IMG_Load("resources/sound_spritesheet.png");
    else pSurface = NULL;
    if (!pSurface) {
        destroyButton(pButton);
        printf("Error in createButton: pSurface is NULL.\n");
        return NULL;
    }

    pButton->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pButton->pTexture) {
        destroyButton(pButton);
        printf("Error in createButton: pButton->pTexture is NULL.\n");
        return NULL;
    }

    SDL_QueryTexture(pButton->pTexture, NULL, NULL, &pButton->srcRect.w, &pButton->srcRect.h);
    pButton->srcRect.w /= pButton->nrOfFrames;
    printf("\nButton[%d] size:\n", button_type);
    pButton->dstRect = scaleAndCenterRect(pButton->srcRect, *pButton->pScreenRect, BOTTON_SCALEFACTOR);

    switch (button_type) {
        case START:
            pButton->dstRect.x = (int)((pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w) / 2.0f + 0.5f);
            pButton->dstRect.y = (int)((pButton->pScreenRect->y * 2 + pButton->pScreenRect->h) / 2.0f + 0.5f) * 0.4f;
            break;
        case EXIT:
            pButton->dstRect.x = (int)((pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w) / 2.0f + 0.5f);
            pButton->dstRect.y = (int)((pButton->pScreenRect->y * 2 + pButton->pScreenRect->h) / 2.0f + 0.5f) * 1.1f;
            break;
        case SOUND:
            pButton->dstRect.w *= 0.55f;
            pButton->dstRect.h *= 0.55f;
            pButton->dstRect.x = (int)(pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w * 1.5f + 0.5f);
            pButton->dstRect.y = (int)(pButton->pScreenRect->y + pButton->dstRect.h * 0.4f + 0.5f);
            break;
        default:
            destroyButton(pButton);
            return NULL; 
    }

    return pButton;
}

void destroyMenu(Menu *pMenu) {
    if (!pMenu) return;

    if (pMenu->pTheme) {
        destroyTheme(pMenu->pTheme);
        pMenu->pTheme = NULL; 
    }

    for (int i = 0; i < NROFBUTTONS; i++) {
        if (pMenu->pButton[i]) {
            destroyButton(pMenu->pButton[i]);
            pMenu->pButton[i] = NULL;
        }
    }

    if (pMenu->pSound) {
        Mix_FreeChunk(pMenu->pSound);
        pMenu->pSound = NULL;
    }

    free(pMenu);
}

void destroyButton(Button *pButton) {
    if (!pButton) return;

    if (pButton->pTexture) {
        SDL_DestroyTexture(pButton->pTexture);
        pButton->pTexture = NULL;
    }
    free(pButton);
}

void drawButton(Button *pButton) {
    if (!pButton) return;
    pButton->srcRect.x = pButton->srcRect.w * pButton->currentFrame;
    SDL_RenderCopy(pButton->pRenderer, pButton->pTexture, &pButton->srcRect, &pButton->dstRect);
}

bool runMenu(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect) {
    Menu *pMenu = createMenu(pRenderer, pScreenRect);
    if (!pMenu) return false;

    pMenu->menuRunning = true;
    pMenu->startGame = false;

    SDL_Event event;
    while (pMenu->menuRunning) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    pMenu->menuRunning = false;
                    pMenu->startGame = false;
                    break;
                case SDL_KEYDOWN:
                    handleKeyDown(&event, pMenu);
                    break;
                case SDL_MOUSEMOTION:
                    handleMouseMotion(&event, pMenu);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    handleMouseButtonDown(&event, pMenu);
                    break;
                default:
                    break;
            }
        }
        playMusic(pMenu->pTheme);
        drawBackground(pMenu->pTheme, 0, 0);
        for (int i = 0; i < NROFBUTTONS; i++) {
            if (i == SOUND) {
                drawButton(pMenu->pButton[i]);
                continue;
            }
            if (pMenu->pButton[i]->is_hovered) pMenu->pButton[i]->currentFrame = 1;
            else pMenu->pButton[i]->currentFrame = 0;
            drawButton(pMenu->pButton[i]);
        }
        drawPadding(pRenderer, *pScreenRect);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(1);
    }

    bool returnValue = pMenu->startGame;
    SDL_Delay(100);
    destroyMenu(pMenu);
    return returnValue;
}

void handleKeyDown(SDL_Event *pEvent, Menu *pMenu) {
    switch (pEvent->key.keysym.scancode) {
        case SDL_SCANCODE_UP:
            if (pMenu->pButton[START]->is_hovered) {
                pMenu->pButton[START]->is_hovered = false;
                pMenu->pButton[EXIT]->is_hovered = false;
            }
            else {
                pMenu->pButton[START]->is_hovered = true;
                pMenu->pButton[EXIT]->is_hovered = false;
                playSound(pMenu->pSound, pMenu->pTheme);
            }
            break;
        case SDL_SCANCODE_DOWN:
            if (pMenu->pButton[EXIT]->is_hovered) {
                pMenu->pButton[START]->is_hovered = false;
                pMenu->pButton[EXIT]->is_hovered = false;
            }
            else {
                pMenu->pButton[START]->is_hovered = false;
                pMenu->pButton[EXIT]->is_hovered = true;
                playSound(pMenu->pSound, pMenu->pTheme);
            }
            break;
        case SDL_SCANCODE_SPACE:
        case SDL_SCANCODE_RETURN:
            if (pMenu->pButton[START]->is_hovered) {
                pMenu->startGame = true; 
                pMenu->menuRunning = false;
                playSound(pMenu->pSound, pMenu->pTheme);
            }
            else if (pMenu->pButton[EXIT]->is_hovered) {
                pMenu->startGame = false; 
                pMenu->menuRunning = false;
                playSound(pMenu->pSound, pMenu->pTheme);
            }
            else if (pMenu->pButton[SOUND]->is_hovered) {
                pMenu->pButton[START]->is_hovered = false;
                pMenu->pButton[EXIT]->is_hovered = false;
                pMenu->pButton[SOUND]->currentFrame = 1 - pMenu->pButton[SOUND]->currentFrame;
                muteOrUnmute(pMenu->pTheme);
                playSound(pMenu->pSound, pMenu->pTheme); 
            }
            break;
        case SDL_SCANCODE_ESCAPE:
            pMenu->startGame = false; 
            pMenu->menuRunning = false;
            break;
        default:
            pMenu->pButton[START]->is_hovered = false;
            pMenu->pButton[EXIT]->is_hovered = false;
            pMenu->pButton[SOUND]->is_hovered = false;
            break;
    }
}

void handleMouseMotion(SDL_Event *pEvent, Menu *pMenu) {
    int x = pEvent->motion.x;
    int y = pEvent->motion.y;

    for (int i = 0; i < NROFBUTTONS; i++) {
        if (!pMenu->pButton[i]) continue;
        bool hovered = isMouseOverButton(x, y, pMenu->pButton[i]);
        
        if (hovered && !pMenu->pButton[i]->is_hovered) {
            pMenu->pButton[i]->is_hovered = true;
            playSound(pMenu->pSound, pMenu->pTheme);
        }
        else if (!hovered) pMenu->pButton[i]->is_hovered = false;
    }
}

void handleMouseButtonDown(SDL_Event *pEvent, Menu *pMenu) {
    int x = pEvent->button.x;
    int y = pEvent->button.y;

    for (int i = 0; i < NROFBUTTONS; i++) {
        if (!pMenu->pButton[i]) continue;
        if (isMouseOverButton(x, y, pMenu->pButton[i])) {
            if (pEvent->button.button == SDL_BUTTON_LEFT) {
                if (i == START) {
                    pMenu->startGame = true;
                    pMenu->menuRunning = false;
                    playSound(pMenu->pSound, pMenu->pTheme);
                } 
                else if (i == EXIT) {
                    pMenu->startGame = false;
                    pMenu->menuRunning = false;
                    playSound(pMenu->pSound, pMenu->pTheme);
                } 
                else if (i == SOUND) {
                    pMenu->pButton[SOUND]->is_hovered = true;
                    pMenu->pButton[SOUND]->currentFrame = 1 - pMenu->pButton[SOUND]->currentFrame;
                    muteOrUnmute(pMenu->pTheme);
                    playSound(pMenu->pSound, pMenu->pTheme); 
                }
            }
        }
    }
}

bool isMouseOverButton(int x, int y, Button *pButton) {
    return (x >= pButton->dstRect.x && x <= pButton->dstRect.x + pButton->dstRect.w &&
            y >= pButton->dstRect.y && y <= pButton->dstRect.y + pButton->dstRect.h);
}