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

struct theme {
    Background *pBackground;
    SDL_Cursor *pCursor;
    Mix_Music *pMusic;
    bool is_muted;
};

Theme *createTheme(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type) {
    if (!pRenderer || !pScreenRect) {
        printf("Error in createTheme: pRenderer or pScreenRect is NULL.\n");
        return NULL;
    }

    Theme *pTheme = malloc(sizeof(struct theme));
    if (!pTheme) {
        printf("Error in createTheme: Failed to allocate memory for pTheme.\n");
        return NULL;
    }

    pTheme->pBackground = createBackground(pRenderer, pScreenRect, theme_type);
    if (!pTheme->pBackground) {
        printf("Error in createTheme: pTheme->pBackground is NULL.\n");
        destroyTheme(pTheme);
        return NULL;
    }

    pTheme->pCursor = initCursor(pTheme);
    if (!pTheme->pCursor) {
        printf("Error in createTheme: pTheme->pCursor is NULL.\n");
        destroyTheme(pTheme);
        return NULL;
    }
    SDL_SetCursor(pTheme->pCursor);
    SDL_ShowCursor(SDL_QUERY);
    if (theme_type == GAME) SDL_ShowCursor(SDL_DISABLE);

    pTheme->pMusic = initMusic(theme_type);
    if (!pTheme->pMusic) {
        printf("Error in createTheme: pTheme->pMusic is NULL.\n");
        destroyTheme(pTheme);
        return NULL;
    }
    pTheme->is_muted = false;

    return pTheme;
}

Background *createBackground(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect, State theme_type) {
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

SDL_Cursor *initCursor(Theme *pTheme) {
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

Mix_Music *initMusic(State theme_type) {
    Mix_Music *pMusic = NULL;
    switch (theme_type) {
        case MENU:
            pMusic = Mix_LoadMUS("resources/menu_music.wav");
            break;
        case GAME:
            pMusic = Mix_LoadMUS("resources/game_music.wav");
            break;
        default:
            pMusic = NULL; 
            break;
    }
    if (!pMusic) {
        printf("Error in createMusic: pMusic is NULL.\n");
        return NULL;
    }
    return pMusic;
}

void drawBackground(Theme *pTheme, int CamX, int CamY) {
    if (!pTheme->pBackground) return;
    // pBackground->rect.x = -CamX;
    // pBackground->rect.y = -CamY;
    SDL_RenderCopy(pTheme->pBackground->pRenderer, pTheme->pBackground->pTexture, &pTheme->pBackground->srcRect, &pTheme->pBackground->dstRect);
}

void playMusic(Theme *pTheme) {
    if (!pTheme->pMusic) return;
    if (pTheme->is_muted) {
        Mix_VolumeMusic(0);
        return;
    }
    if (!Mix_PlayingMusic()) {
        if (Mix_PlayMusic(pTheme->pMusic, -1) == -1) { 
            printf("Failed to play music: %s\n", Mix_GetError());
        }
    }
    Mix_VolumeMusic((int)(MIX_MAX_VOLUME * 0.5));
}

void playSound(Mix_Chunk *pSound, Theme *pTheme) {
    if (pTheme->is_muted) return;
    Mix_PlayChannel(-1, pSound, 0);
}

void muteOrUnmute(Theme *pTheme) {
    if (pTheme->is_muted) pTheme->is_muted = false;
    else pTheme->is_muted = true;
}

void destroyBackground(Background *pBackground) {
    if (!pBackground) return;
    if (pBackground->pTexture) {
        SDL_DestroyTexture(pBackground->pTexture);
        pBackground->pTexture = NULL;
    }
    free(pBackground);
}

void destroyTheme(Theme *pTheme) {
    if (!pTheme) return;
    if (pTheme->pBackground) {
        destroyBackground(pTheme->pBackground);
        pTheme->pBackground = NULL;
    }
    if (pTheme->pCursor) {
        SDL_FreeCursor(pTheme->pCursor);
        pTheme->pCursor = NULL;
    }
    if (pTheme->pMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(pTheme->pMusic);
        pTheme->pMusic = NULL;
    }
    free(pTheme);
}