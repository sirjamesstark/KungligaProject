#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/theme.h"
#include "../include/common.h"

struct background {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
};

struct button {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
    int nrOfFrames, currentFrame;
    bool isHovered;
};

struct audio {
    Mix_Music *pMusic;
    Mix_Chunk *pButtonSound;
    Mix_Chunk *pJumpSound;
    Mix_Chunk *pDeathSound;
    bool isMuted;
};

Background *createBackground(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type) {
    if (!pRenderer || !pScreenRect) {
        printf("Error in createBackground: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }
    
    Background *pBackground = malloc(sizeof(struct background));
    if (!pBackground) {
        printf("Error in createBackground: Failed to allocate memory for pBackground.\n");
        return NULL;
    }

    SDL_Surface *pSurface = NULL;
    switch (theme_type) {
        case MENU:
            pSurface = IMG_Load("resources/main_background.png");         
            break;
        case GAME:
            pSurface = IMG_Load("resources/game_background.png");
            break;
        default:
            pSurface = NULL;
            break;
    }

    if (!pSurface) {
        printf("Error in createBackground: pSurface is NULL.\n");
        destroyBackground(pBackground);
        return NULL;
    }

    pBackground->pRenderer = pRenderer;
    pBackground->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    pBackground->pScreenRect = pScreenRect;

    if (!pBackground->pTexture) {
        printf("Error in createBackground: pBackground->pTexture is NULL.\n");
        destroyBackground(pBackground);
        return NULL;
    }

    SDL_QueryTexture(pBackground->pTexture, NULL, NULL, &pBackground->srcRect.w, &pBackground->srcRect.h);
    pBackground->srcRect.x = pBackground->srcRect.y = 0;
    printf("\nBackground size:\n");
    pBackground->dstRect = stretchRectToScreen(*pBackground->pScreenRect);

    return pBackground;
}

void drawBackground(Background *pBackground) {
    if (!pBackground) return;
    SDL_RenderCopy(pBackground->pRenderer, pBackground->pTexture, &pBackground->srcRect, &pBackground->dstRect);
}

void destroyBackground(Background *pBackground) {
    if (!pBackground) return;
    if (pBackground->pTexture) {
        SDL_DestroyTexture(pBackground->pTexture);
        pBackground->pTexture = NULL;
    }
    free(pBackground);
}

Button *createButton(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, ButtonType button_type) {
    if (!pRenderer || !pScreenRect) {
        printf("Error in createButton: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Button *pButton = malloc(sizeof(struct button));
    if (!pButton) {
        printf("Error in createButton: Failed to allocate memory for pButton.\n");
        return NULL;
    }

    SDL_Surface *pSurface = NULL;
    switch (button_type) {
        case START:
            pSurface = IMG_Load("resources/start_spritesheet.png");
            break;
        case EXIT:
            pSurface = IMG_Load("resources/exit_spritesheet.png");
            break;
        case SOUND:
            pSurface = IMG_Load("resources/sound_spritesheet.png");
            break;
        default:
            pSurface = NULL;
            break;
    }

    pButton->pRenderer = pRenderer;
    pButton->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    pButton->pScreenRect = pScreenRect;

    if (!pButton->pTexture) {
        printf("Error in createButton: pButton->pTexture is NULL.\n");
        destroyButton(pButton);
        return NULL;
    }

    pButton->nrOfFrames = 2;
    pButton->currentFrame = 0;
    pButton->isHovered = false;

    SDL_QueryTexture(pButton->pTexture, NULL, NULL, &pButton->srcRect.w, &pButton->srcRect.h);
    pButton->srcRect.w /= pButton->nrOfFrames;
    printf("\nButton[%d] size:\n", button_type);
    pButton->dstRect = scaleAndCenterRect(pButton->srcRect, *pButton->pScreenRect, BUTTON_SCALEFACTOR);

    if (!setButtonPlacement(pButton, button_type)) {
        destroyButton(pButton);
        return NULL;
    }

    return pButton;
}

int setButtonPlacement(Button *pButton, ButtonType button_type) {
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
            printf("Exception: Button[%d] adjusted size: w: %d, h: %d\n", button_type, pButton->dstRect.w, pButton->dstRect.h);
            pButton->dstRect.x = (int)(pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w * 1.5f + 0.5f);
            pButton->dstRect.y = (int)(pButton->pScreenRect->y + pButton->dstRect.h * 0.4f + 0.5f);
            break;
        default:
            return 0;
    }
    return 1;
}

void drawButton(Button *pButton) {
    if (!pButton) return;
    pButton->srcRect.x = pButton->srcRect.w * pButton->currentFrame;
    SDL_RenderCopy(pButton->pRenderer, pButton->pTexture, &pButton->srcRect, &pButton->dstRect);
}

void makeButtonHoverd(Button *pButton) {
    if (!pButton) return;
    pButton->isHovered = true;
    pButton->currentFrame = 1;
}

void makeButtonNotHovered(Button *pButton) {
    if (!pButton) return;
    pButton->isHovered = false;
    pButton->currentFrame = 0;
}

void toggleHoveredButton(Button *pButton) {
    if (!pButton) return;
    if (pButton->currentFrame == 0) {
        pButton->currentFrame = 1;
        pButton->isHovered = true;
    }
    else {
        pButton->currentFrame = 0;
        pButton->isHovered = false;
    }
}

bool isButtonHovered(Button *pButton) {
    if (!pButton) return false;
    return pButton->isHovered; 
}

bool isMouseOverButton(int x, int y, Button *pButton) {
    return (x >= pButton->dstRect.x && x <= pButton->dstRect.x + pButton->dstRect.w &&
            y >= pButton->dstRect.y && y <= pButton->dstRect.y + pButton->dstRect.h);
}

void destroyButton(Button *pButton) {
    if (!pButton) return;
    if (pButton->pTexture) {
        SDL_DestroyTexture(pButton->pTexture);
        pButton->pTexture = NULL;
    }
    free(pButton);
}

Audio *createAudio(State theme_type) {
    Audio *pAudio = malloc(sizeof(struct audio));
    if (!pAudio) {
        printf("Error in createAudio: Failed to allocate memory for pAudio.\n");
        return NULL;
    }

    switch (theme_type) {
        case MENU:
            pAudio->pMusic = Mix_LoadMUS("resources/menu_music.wav");
            break;
        case GAME:
            pAudio->pMusic = Mix_LoadMUS("resources/game_music.wav");
            break;
        default:
            pAudio->pMusic = NULL; 
            break;
    }

    pAudio->pButtonSound = Mix_LoadWAV("resources/button_selection_sound.wav");
    pAudio->pJumpSound = Mix_LoadWAV("resources/jump_sound.wav");
    pAudio->pDeathSound = Mix_LoadWAV("resources/jump_sound.wav");  // Ersätt detta ljud med dödsljudet sen
    if (!pAudio->pMusic || !pAudio->pButtonSound || !pAudio->pJumpSound || !pAudio->pDeathSound) {
        printf("Error in createAudio: Failed to load one or more audio files.\n");
        destroyAudio(pAudio);
        return NULL;
    }

    pAudio->isMuted = false;

    return pAudio;
}

void playMusic(Audio *pAudio) {
    if (!pAudio || !pAudio->pMusic) return;

    if (pAudio->isMuted) {
        Mix_VolumeMusic(0);
        return;
    }

    if (!Mix_PlayingMusic()) {
        if (Mix_PlayMusic(pAudio->pMusic, -1) == -1) { 
            printf("Failed to play music: %s\n", Mix_GetError());
        }
    }

    Mix_VolumeMusic((int)(MIX_MAX_VOLUME * 0.5));
}

void playButtonSound(Audio *pAudio) {
    if (!pAudio || !pAudio->pButtonSound) return;
    if (pAudio->isMuted) return;
    Mix_PlayChannel(-1, pAudio->pButtonSound, 0);
}

void playJumpSound(Audio *pAudio) {
    if (!pAudio || !pAudio->pJumpSound) return;
    if (pAudio->isMuted) return;
    Mix_PlayChannel(-1, pAudio->pJumpSound, 0);
}

void playDeathSound(Audio *pAudio) {
    if (!pAudio || !pAudio->pDeathSound) return;
    if (pAudio->isMuted) return;
    Mix_PlayChannel(-1, pAudio->pDeathSound, 0);
}

void toggleMuteAudio(Audio *pAudio) {
    if (!pAudio) return;
    pAudio->isMuted = !pAudio->isMuted;
    playMusic(pAudio);
}

bool isMusicMuted(Audio *pAudio) {
    if (!pAudio) return false;
    return pAudio->isMuted; 
}

void destroyAudio(Audio *pAudio) {
    if (!pAudio) return;

    if (pAudio->pMusic) {
        Mix_FreeMusic(pAudio->pMusic);
        pAudio->pMusic = NULL;
    }

    if (pAudio->pButtonSound) {
        Mix_FreeChunk(pAudio->pButtonSound);
        pAudio->pButtonSound = NULL;
    }

    if (pAudio->pJumpSound) {
        Mix_FreeChunk(pAudio->pJumpSound);
        pAudio->pJumpSound = NULL;
    }

    if (pAudio->pDeathSound) {
        Mix_FreeChunk(pAudio->pDeathSound);
        pAudio->pDeathSound = NULL;
    }

    free(pAudio);
}

SDL_Cursor *initCursor() {
    SDL_Cursor *pCursor = NULL; 
    SDL_Surface *pSurface = IMG_Load("resources/cursor.png");

    if (!pSurface) {
        printf("Error in initCursor: pSurface is NULL.\n");
        return SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    }

    pCursor = SDL_CreateColorCursor(pSurface, 0, 0);
    SDL_FreeSurface(pSurface);
    return pCursor; 
}

void destroyCursor(SDL_Cursor *pCursor) {
    if (!pCursor) return;
    SDL_FreeCursor(pCursor);
}