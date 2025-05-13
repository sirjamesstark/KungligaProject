#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/theme.h"
#include "../include/scaling.h"

struct background {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
};

struct lava {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
    int nrOfFrames, currentFrame;
    int frameDelay;
    Uint32 lastFrameTime;
};

struct button {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
    int nrOfFrames, currentFrame;
    bool isHovered, isPushed;
};

struct cursor {
    SDL_Cursor *pCursor;
    int x, y;
    bool isVisible;
};

struct audio {
    Mix_Music *pMusic;
    Mix_Chunk *pButtonSound;
    Mix_Chunk *pJumpSound;
    Mix_Chunk *pDeathSound;
    Mix_Chunk *pWinningSound;
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
    stretchRectToFitTarget(&pBackground->dstRect, *pBackground->pScreenRect);

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

Lava *createLava(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect) {
    if (!pRenderer || !pScreenRect) {
        printf("Error in createLava: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Lava *pLava = malloc(sizeof(struct lava));
    if (!pLava) {
        printf("Error in createLava: Failed to allocate memory for pLava.\n");
        return NULL;
    }

    SDL_Surface *pSurface = IMG_Load("resources/lava_spritesheet.png");
    if (!pSurface) {
        printf("Error in createLava: pSurface is NULL.\n");
        destroyLava(pLava);
        return NULL;
    }

    pLava->pRenderer = pRenderer;
    pLava->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    pLava->pScreenRect = pScreenRect;

    if (!pLava->pTexture) {
        printf("Error in createLava: pLava->pTexture is NULL.\n");
        destroyLava(pLava);        
        return NULL;
    }

    pLava->nrOfFrames = 4;
    pLava->currentFrame = 0;

    SDL_QueryTexture(pLava->pTexture, NULL, NULL, &pLava->srcRect.w, &pLava->srcRect.h);
    pLava->srcRect.w /= pLava->nrOfFrames;
    printf("\nLava size:\n");
    pLava->dstRect = scaleRect(pLava->srcRect, *pLava->pScreenRect, LAVA_SCALEFACTOR);
    pLava->dstRect.x = pLava->pScreenRect->x;
    pLava->dstRect.y = pLava->pScreenRect->y + pLava->pScreenRect->h - pLava->dstRect.h;

    pLava->frameDelay = 100;
    pLava->lastFrameTime = SDL_GetTicks();

    return pLava;
}

void updateLavaFrame(Lava *pLava) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - pLava->lastFrameTime < pLava->frameDelay) return;
    pLava->lastFrameTime = currentTime;
    pLava->currentFrame = (pLava->currentFrame + 1) % pLava->nrOfFrames;
    pLava->srcRect.x = pLava->srcRect.w * pLava->currentFrame;
}

void drawLava(Lava *pLava) {
    if (!pLava) return;
    updateLavaFrame(pLava);
    for (int i = pLava->pScreenRect->x; i < pLava->pScreenRect->w; i += pLava->dstRect.w) {
        SDL_Rect tmp_dstRect = pLava->dstRect;
        tmp_dstRect.x = i;
        SDL_RenderCopy(pLava->pRenderer, pLava->pTexture, &pLava->srcRect, &tmp_dstRect);
    }
}

void destroyLava(Lava *pLava) {
    if (!pLava) return;
    if (pLava->pTexture) {
        SDL_DestroyTexture(pLava->pTexture);
        pLava->pTexture = NULL;
    }
    free(pLava);
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
    case JOIN:
        pSurface = IMG_Load("resources/Fiery_Join_in_Flames_spritesheet.png");
        break;
    case BACK:
        pSurface = IMG_Load("resources/exit_spritesheet.png"); 
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
    pButton->isPushed = false;

    SDL_QueryTexture(pButton->pTexture, NULL, NULL, &pButton->srcRect.w, &pButton->srcRect.h);
    pButton->srcRect.w /= pButton->nrOfFrames;
    printf("\nButton[%d] size:\n", button_type);
    pButton->dstRect = scaleRect(pButton->srcRect, *pButton->pScreenRect, BUTTON_SCALEFACTOR);

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
            pButton->dstRect.y = (int)((pButton->pScreenRect->y * 2 + pButton->pScreenRect->h) / 2.0f + 0.5f) * 0.3f;
            break;
        case EXIT:
            pButton->dstRect.x = (int)((pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w) / 2.0f + 0.5f);
            pButton->dstRect.y = (int)((pButton->pScreenRect->y * 2 + pButton->pScreenRect->h) / 2.0f + 0.5f) * 0.8f;
            break;
        case SOUND:
            pButton->dstRect.w *= 0.55f;
            pButton->dstRect.h *= 0.55f;
            printf("Exception: Button[%d] adjusted size: w: %d, h: %d\n", button_type, pButton->dstRect.w, pButton->dstRect.h);
            pButton->dstRect.x = (int)(pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w * 1.5f + 0.5f);
            pButton->dstRect.y = (int)(pButton->pScreenRect->y + pButton->dstRect.h * 0.4f + 0.5f);
            break;
        case JOIN:
            pButton->dstRect.x = (int)((pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w) / 2.0f + 0.5f);
            pButton->dstRect.y = (int)((pButton->pScreenRect->y * 2 + pButton->pScreenRect->h) / 2.0f + 0.5f) * 1.3f;
            break;
        case BACK:
            pButton->dstRect.x = (int)((pButton->pScreenRect->x * 2 + pButton->pScreenRect->w - pButton->dstRect.w) / 2.0f + 0.5f);
            pButton->dstRect.y = (int)((pButton->pScreenRect->y * 2 + pButton->pScreenRect->h) / 2.0f + 0.5f) * 1.3f;
            break;
        default:
            return 0;
    }
    return 1;
}

void makeButtonHoverd(Button *pButton) {
    if (!pButton) return;
    pButton->isHovered = true;
}

void makeButtonNotHovered(Button *pButton) {
    if (!pButton) return;
    pButton->isHovered = false;
}

void toggleHoveredButton(Button *pButton) {
    if (!pButton) return;
    pButton->isHovered = !pButton->isHovered;
}

bool isButtonPushed(Button *pButton) {
    if (!pButton) return false;
    if (pButton->isHovered) pButton->isPushed = true;
    else pButton->isPushed = false;
    return pButton->isPushed;
}

void setButton_isHovered(Button *pButton, Cursor *pCursor) {
    if (!pButton || !pCursor) return;
    updateCursorPosition(pCursor);
    if (pCursor->x >= pButton->dstRect.x && pCursor->x <= pButton->dstRect.x + pButton->dstRect.w &&
        pCursor->y >= pButton->dstRect.y && pCursor->y <= pButton->dstRect.y + pButton->dstRect.h) {
            pButton->isHovered = true;
    }
    else pButton->isHovered = false;
}

bool getButton_isHovered(Button *pButton){
    return pButton->isHovered;
}

void drawButton(Button *pButton) {
    if (!pButton) return;
    if (pButton->isHovered) pButton->currentFrame = 1;
    else pButton->currentFrame = 0;
    pButton->srcRect.x = pButton->srcRect.w * pButton->currentFrame;
    SDL_RenderCopy(pButton->pRenderer, pButton->pTexture, &pButton->srcRect, &pButton->dstRect);
}

void destroyButton(Button *pButton) {
    if (!pButton) return;
    if (pButton->pTexture) {
        SDL_DestroyTexture(pButton->pTexture);
        pButton->pTexture = NULL;
    }
    free(pButton);
}

Cursor *createCursor() {
    Cursor *pCursor = malloc(sizeof(struct cursor));
    if (!pCursor) {
        printf("Error in createCursor: Failed to allocate memory for pCursor.\n");
        return NULL;
    }

    SDL_Surface *pSurface = IMG_Load("resources/cursor.png");
    if (!pSurface) {
        printf("Error in createCursor: pSurface is NULL.\n");
        pCursor->pCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    }
    else {
        pCursor->pCursor = SDL_CreateColorCursor(pSurface, 0, 0);
        SDL_FreeSurface(pSurface);
    }

    if (!pCursor->pCursor) {
        printf("Error in createCursor: pCursor->pCursor is NULL.\n");
        destroyCursor(pCursor);
        return NULL;
    }

    SDL_SetCursor(pCursor->pCursor);
    SDL_ShowCursor(SDL_ENABLE);
    pCursor->isVisible = true;
    SDL_GetMouseState(&pCursor->x, &pCursor->y);
    
    return pCursor;
}

void updateCursorPosition(Cursor *pCursor) {
    if (!pCursor) return;
    SDL_GetMouseState(&pCursor->x, &pCursor->y);
}

void toggleCursorVisibility(Cursor *pCursor) {
    if (!pCursor) return;
    pCursor->isVisible = !pCursor->isVisible;
    if (pCursor->isVisible) SDL_ShowCursor(SDL_ENABLE);
    else SDL_ShowCursor(SDL_DISABLE);
}

void destroyCursor(Cursor *pCursor) {
    if (!pCursor) return;
    if (pCursor->pCursor) {
        SDL_FreeCursor(pCursor->pCursor);
        pCursor->pCursor = NULL;
    }
    free (pCursor);
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
    pAudio->pDeathSound = Mix_LoadWAV("resources/jump_sound.wav"); // Ersätt detta ljud med dödsljudet sen
    pAudio->pWinningSound = Mix_LoadWAV("resources/jump_sound.wav"); // Ersätt detta ljud med vinnarljudet sen

    if (!pAudio->pMusic || !pAudio->pButtonSound || !pAudio->pJumpSound || !pAudio->pDeathSound || !pAudio->pWinningSound) {
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

void playWinningSound(Audio *pAudio) {
    if (!pAudio || !pAudio->pWinningSound) return;
    if (pAudio->isMuted) return;
    Mix_PlayChannel(-1, pAudio->pWinningSound, 0);
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

    if (pAudio->pWinningSound) {
        Mix_FreeChunk(pAudio->pWinningSound);
        pAudio->pWinningSound = NULL;
    }

    free(pAudio);
}