#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/theme.h"

struct background
{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect *pScreenRect;
};

Mix_Music *initiateMusic()
{
    Mix_Music *pGameMusic = Mix_LoadMUS("resources/game_music.wav");
    if (!pGameMusic)
    {
        printf("Failed to load game music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    return pGameMusic;
    
}

void playMusic(Mix_Music *pGameMusic)
{
    Mix_VolumeMusic((int)(MIX_MAX_VOLUME * 0.5));  // 50% volym
    if (Mix_PlayMusic(pGameMusic, -1) == -1) 
    { 
        printf("Failed to play music: %s\n", Mix_GetError());
    }
}


Background *createBackground(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect)
{
    Background *pBackground = malloc(sizeof(struct background));
    if (!pBackground)
    {
        printf("Failed to allocate memory for background\n");
        return NULL;
    }
    SDL_Surface *pBackgroundSurface = IMG_Load("resources/game_background.png");
    if (!pBackgroundSurface)
    {
        printf("Error loading background: %s\n", SDL_GetError());
        free(pBackground);
        return 0;
    }
    pBackground->pRenderer = pRenderer;
    pBackground->pTexture = SDL_CreateTextureFromSurface(pRenderer, pBackgroundSurface);
    SDL_FreeSurface(pBackgroundSurface);
    if (!pBackground->pTexture)
    {
        printf("Error creating background texture: %s\n", SDL_GetError());
        free(pBackground);
        return 0;
    }
    pBackground->pScreenRect = pScreenRect; 
    return pBackground;
}

void drawBackground(Background *pBackground, int CamX, int CamY)
{

    // pBackground->rect.x = -CamX;
    // pBackground->rect.y = -CamY;
    SDL_RenderCopy(pBackground->pRenderer, pBackground->pTexture, NULL, pBackground->pScreenRect);
}

void destroyBackground(Background *pBackground)
{
    if (pBackground->pTexture != NULL)
    {
        SDL_DestroyTexture(pBackground->pTexture);
        pBackground->pTexture = NULL;
    }
    if (pBackground != NULL)
    {
        free(pBackground);
        pBackground = NULL;
    }
}