#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/theme.h"

struct background {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
};

Mix_Music* initiateMusic()
{
    Mix_Music *pGameMusic = Mix_LoadMUS("resources/game_music.wav");
    if (!pGameMusic) {
        printf("Failed to load game music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    } 
    else 
    {
        Mix_VolumeMusic((int)(MIX_MAX_VOLUME * 0.5));  // Set volume to 50%
        Mix_PlayMusic(pGameMusic, -1);  // -1 means loop infinitely
        return pGameMusic;
    }
}

Background *createBackground(SDL_Renderer *pRenderer, int window_width, int window_height)
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
    pBackground->rect = (SDL_Rect){0,0,window_width,window_height};
    return pBackground;
}

void drawBackground(Background *pBackground)
{
    SDL_RenderCopy(pBackground->pRenderer, pBackground->pTexture, NULL, &(pBackground->rect));
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