#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "../include/menu.h"

struct buttonImages
{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;    
};
struct buttonsAndBackground 
{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect; 
};
struct menuVariables 
{
    bool menuRunning, muteHover, soundGame, startGame;
    int menuChoice, mousex, mousey;
};

bool showMenu(SDL_Renderer *pRenderer, int window_width, int window_height)
{
    ButtonImages *pButtonImages[NROFPICS];
    ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS];
    MenuVariables *pMenuVariables = malloc(sizeof(struct menuVariables));
    Mix_Music *pMenuMusic = Mix_LoadMUS("resources/menu_music.wav");
    if (!pMenuMusic) {
        printf("Failed to load menu music! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    Mix_PlayMusic(pMenuMusic, -1);  // -1 means loop infinitely

    // Load button click sound effect
    Mix_Chunk *pButtonSound = Mix_LoadWAV("resources/button_selection_sound.wav");
    if (!pButtonSound) {
        printf("Failed to load button sound! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_FreeMusic(pMenuMusic);
        return false;
    }
    if (!createAllImages(pButtonImages,pRenderer))
    {
        return false;
    } 
    createButtonsAndBackground(pButtonsAndBackground, pButtonImages, window_width, window_height);
    runMenu(pButtonImages,pButtonsAndBackground,pMenuVariables,pButtonSound,pMenuMusic,pRenderer);
    cleanMenu(pButtonImages,pButtonsAndBackground,pMenuVariables);
    // Clean up all audio resources
    if (!pMenuVariables->startGame) {
        Mix_HaltMusic();  // Stop music if exiting without starting game
        Mix_FreeMusic(pMenuMusic);
    }
    
    // Wait a tiny bit to ensure sound effects finish playing
    SDL_Delay(100);
    Mix_FreeChunk(pButtonSound);
    Mix_CloseAudio();
    
    return pMenuVariables->startGame;
}

int createAllImages(ButtonImages *pButtonImages[NROFPICS] ,SDL_Renderer *pRenderer)
{
    for (int i = 0; i < NROFPICS; i++)
    {
        pButtonImages[i] = malloc(sizeof(struct buttonImages));
    }
    SDL_Surface *pBackgroundSurface = IMG_Load("resources/main_background.png");
    SDL_Surface *pStart0Surface = IMG_Load("resources/menu_start0.png");
    SDL_Surface *pStart1Surface = IMG_Load("resources/menu_start1.png");
    SDL_Surface *pExit0Surface = IMG_Load("resources/menu_exit0.png");
    SDL_Surface *pExit1Surface = IMG_Load("resources/menu_exit1.png");
    SDL_Surface *pSoundOnSurface = IMG_Load("resources/soundon0.png");
    SDL_Surface *pSoundOffSurface = IMG_Load("resources/soundoff0.png");
    if (!pBackgroundSurface || !pStart0Surface || !pStart1Surface || !pExit0Surface ||
        !pExit1Surface || !pSoundOnSurface || !pSoundOffSurface) 
    {
        printf("Error loading one or more surfaces: %s\n", IMG_GetError());
        freeAllSurface(pBackgroundSurface,pStart0Surface,pStart1Surface,pExit0Surface,pExit1Surface,pSoundOnSurface,pSoundOnSurface);
        return 0;
    }
    for (int i = 0; i < NROFPICS; i++)
    {
        pButtonImages[i]->pRenderer = pRenderer;
    }
    pButtonImages[0]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[0]->pRenderer, pBackgroundSurface);
    pButtonImages[1]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[1]->pRenderer, pStart0Surface);
    pButtonImages[2]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[2]->pRenderer, pStart1Surface);
    pButtonImages[3]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[3]->pRenderer, pExit0Surface);
    pButtonImages[4]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[4]->pRenderer, pExit1Surface);
    pButtonImages[5]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[5]->pRenderer, pSoundOnSurface);
    pButtonImages[6]->pTexture = SDL_CreateTextureFromSurface(pButtonImages[6]->pRenderer, pSoundOffSurface);
    for (int i = 0; i < NROFPICS; i++)
    {
        if (!(pButtonImages[i]->pTexture))
        {
            printf("Error creating textures: %s\n", SDL_GetError());
            return 0;
        }   
    }
    freeAllSurface(pBackgroundSurface,pStart0Surface,pStart1Surface,pExit0Surface,pExit1Surface,pSoundOnSurface,pSoundOffSurface);
    return 1;
}
void freeAllSurface(SDL_Surface *pBackgroundSurface, SDL_Surface *pStart0Surface,
    SDL_Surface *pStart1Surface, SDL_Surface *pExit0Surface, 
    SDL_Surface *pExit1Surface,SDL_Surface *pSoundOnSurface,
    SDL_Surface *pSoundOffSurface)
{
    if (pBackgroundSurface) SDL_FreeSurface(pBackgroundSurface);
    if (pStart0Surface)     SDL_FreeSurface(pStart0Surface);
    if (pStart1Surface)     SDL_FreeSurface(pStart1Surface);
    if (pExit0Surface)      SDL_FreeSurface(pExit0Surface);
    if (pExit1Surface)      SDL_FreeSurface(pExit1Surface);
    if (pSoundOnSurface)    SDL_FreeSurface(pSoundOnSurface);
    if (pSoundOffSurface)   SDL_FreeSurface(pSoundOffSurface);
}

void createButtonsAndBackground(ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS], ButtonImages *pButtonImages[NROFPICS],
                                int window_width,int window_height)
{
    for (int i = 0; i < NROFBUTTONS; i++)
    {
        pButtonsAndBackground[i] = malloc(sizeof(struct buttonsAndBackground));
    }
    //start button
    pButtonsAndBackground[0]->pRenderer = pButtonImages[2]->pRenderer;
    pButtonsAndBackground[0]->pTexture = pButtonImages[2]->pTexture;
    SDL_QueryTexture(pButtonsAndBackground[0]->pTexture,NULL,NULL,&(pButtonsAndBackground[0]->rect.w),&(pButtonsAndBackground[0]->rect.h));
    pButtonsAndBackground[0]->rect.w = window_width / 4;
    pButtonsAndBackground[0]->rect.h = window_height / 8;
    pButtonsAndBackground[0]->rect.x = (window_width - pButtonsAndBackground[0]->rect.w) / 2;
    pButtonsAndBackground[0]->rect.y = (window_height / 3) - 60;
    //exit button
    pButtonsAndBackground[1]->pRenderer = pButtonImages[3]->pRenderer;
    pButtonsAndBackground[1]->pTexture = pButtonImages[3]->pTexture;
    SDL_QueryTexture(pButtonsAndBackground[1]->pTexture,NULL,NULL,&(pButtonsAndBackground[1]->rect.w),&(pButtonsAndBackground[1]->rect.h));
    pButtonsAndBackground[1]->rect.w = window_width / 4;
    pButtonsAndBackground[1]->rect.h = window_height / 8;
    pButtonsAndBackground[1]->rect.x = (window_width - pButtonsAndBackground[1]->rect.w) / 2;
    pButtonsAndBackground[1]->rect.y = (window_height / 2) + 57;  
    //background pic
    pButtonsAndBackground[2]->pRenderer = pButtonImages[0]->pRenderer;
    pButtonsAndBackground[2]->pTexture = pButtonImages[0]->pTexture;
    SDL_QueryTexture(pButtonsAndBackground[2]->pTexture,NULL,NULL,&(pButtonsAndBackground[2]->rect.w),&(pButtonsAndBackground[2]->rect.h));
    pButtonsAndBackground[2]->rect.x = 0;
    pButtonsAndBackground[2]->rect.y = 0;
    pButtonsAndBackground[2]->rect.w = window_width;
    pButtonsAndBackground[2]->rect.h = window_height;
    //sound button
    pButtonsAndBackground[3]->pRenderer = pButtonImages[5]->pRenderer;
    pButtonsAndBackground[3]->pTexture = pButtonImages[5]->pTexture;
    SDL_QueryTexture(pButtonsAndBackground[3]->pTexture,NULL,NULL,&(pButtonsAndBackground[3]->rect.w),&(pButtonsAndBackground[3]->rect.h));
    pButtonsAndBackground[3]->rect.w = ((window_width) / 15);
    pButtonsAndBackground[3]->rect.h = ((window_height) / 8); 
    pButtonsAndBackground[3]->rect.x = (window_width - pButtonsAndBackground[3]->rect.w - 58); 
    pButtonsAndBackground[3]->rect.y = 39; 
}

void runMenu(ButtonImages *pButtonImages[NROFPICS], ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS], 
                MenuVariables *pMenuVariables, Mix_Chunk *pButtonSound, Mix_Music *pMenuMusic, SDL_Renderer *pRenderer)
{
    pMenuVariables->menuRunning = true;
    pMenuVariables->muteHover = false;
    pMenuVariables->soundGame = true;
    pMenuVariables->startGame = false;
    pMenuVariables->menuChoice = 1;
    pMenuVariables->mousex = 0;
    pMenuVariables->mousey = 0;
    while (pMenuVariables->menuRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
           if (event.type == SDL_QUIT)
            {
                pMenuVariables->menuRunning = false;
            }
            else chooseMenuOption(pButtonsAndBackground,&event,pButtonSound,pMenuMusic,pMenuVariables);
        }

        // Clear renderer and draw background first
        SDL_RenderClear(pButtonsAndBackground[2]->pRenderer);
        SDL_RenderCopy(pButtonsAndBackground[2]->pRenderer, pButtonsAndBackground[2]->pTexture, NULL, &(pButtonsAndBackground[2]->rect));

        // Render sound icon based on state
        if (pMenuVariables->soundGame) 
        {
            SDL_RenderCopy(pRenderer, pButtonImages[5]->pTexture, NULL, &(pButtonsAndBackground[3]->rect));
        } 
        else 
        {
            SDL_RenderCopy(pRenderer, pButtonImages[6]->pTexture, NULL, &(pButtonsAndBackground[3]->rect));
        }

        // Default state (no hover)
        if (pMenuVariables->menuChoice == 0)
        {
            SDL_RenderCopy(pRenderer, pButtonImages[1]->pTexture, NULL, &(pButtonsAndBackground[0]->rect));
            SDL_RenderCopy(pRenderer, pButtonImages[3]->pTexture, NULL, &(pButtonsAndBackground[1]->rect));
        }
        // Start button hover
        else if (pMenuVariables->menuChoice == 1)
        {
            SDL_RenderCopy(pRenderer, pButtonImages[2]->pTexture, NULL, &(pButtonsAndBackground[0]->rect));
            SDL_RenderCopy(pRenderer, pButtonImages[3]->pTexture, NULL, &(pButtonsAndBackground[1]->rect));
        }
        // Exit button hover
        else if (pMenuVariables->menuChoice == 2)
        {
            SDL_RenderCopy(pRenderer, pButtonImages[1]->pTexture, NULL, &(pButtonsAndBackground[0]->rect));
            SDL_RenderCopy(pRenderer, pButtonImages[4]->pTexture, NULL, &(pButtonsAndBackground[1]->rect));
        }
        SDL_RenderPresent(pRenderer);
    }
}

void chooseMenuOption(ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS], SDL_Event *pEvent, Mix_Chunk *pButtonSound, 
                        Mix_Music *pMenuMusic, MenuVariables *pMenuVariables)
{
    if (pEvent->type == SDL_MOUSEMOTION)
    {
        SDL_GetMouseState(&(pMenuVariables->mousex), &(pMenuVariables->mousey));
        if ((pMenuVariables->mousex > pButtonsAndBackground[1]->rect.x && pMenuVariables->mousex < 
                pButtonsAndBackground[1]->rect.x + pButtonsAndBackground[1]->rect.w) && 
            (pMenuVariables->mousey > pButtonsAndBackground[1]->rect.y && pMenuVariables->mousey < 
                pButtonsAndBackground[1]->rect.y + pButtonsAndBackground[1]->rect.h))
        {
            pMenuVariables->menuChoice = 2;
        }
        else if ((pMenuVariables->mousex > pButtonsAndBackground[0]->rect.x && pMenuVariables->mousex < 
                    pButtonsAndBackground[0]->rect.x + pButtonsAndBackground[0]->rect.w) && 
                (pMenuVariables->mousey > pButtonsAndBackground[0]->rect.y && pMenuVariables->mousey < 
                    pButtonsAndBackground[0]->rect.y + pButtonsAndBackground[0]->rect.h))
        {
            pMenuVariables->menuChoice = 1;
        }
        else
        {
            pMenuVariables->menuChoice = 0;
        }
    }
    else if (pEvent->type == SDL_MOUSEBUTTONDOWN)
    {
        if (SDL_BUTTON_LEFT == pEvent->button.button)
        {
            bool buttonClicked = false;
            if ((pMenuVariables->mousex > pButtonsAndBackground[3]->rect.x && pMenuVariables->mousex < 
                    pButtonsAndBackground[3]->rect.x + pButtonsAndBackground[3]->rect.w) && 
                (pMenuVariables->mousey > pButtonsAndBackground[3]->rect.y && pMenuVariables-> mousey < 
                    pButtonsAndBackground[3]->rect.y + pButtonsAndBackground[3]->rect.h))
            {
                Mix_PlayChannel(-1, pButtonSound, 0);  // Play click sound
                if (pMenuVariables->soundGame)
                {
                    Mix_PauseMusic();  // Pause menu music
                    pMenuVariables->soundGame = false;
                }
                else
                {
                    Mix_ResumeMusic();  // Resume menu music
                    pMenuVariables->soundGame = true;
                }
            }
            // When you click Start, I'll play a sound and kick off the game
            else if ((pMenuVariables->mousex > pButtonsAndBackground[0]->rect.x && pMenuVariables->mousex < 
                        pButtonsAndBackground[0]->rect.x + pButtonsAndBackground[0]->rect.w) && 
                     (pMenuVariables->mousey > pButtonsAndBackground[0]->rect.y && pMenuVariables->mousey < 
                        pButtonsAndBackground[0]->rect.y + pButtonsAndBackground[0]->rect.h))
            {
                Mix_PlayChannel(-1, pButtonSound, 0);  // Play click sound
                SDL_Delay(50);  // Brief delay for sound
                Mix_HaltMusic();  // Stop menu music
                Mix_FreeMusic(pMenuMusic);
                pMenuVariables->startGame = true;
                pMenuVariables->menuRunning = false;
            }
            // Exit button gets a sound too - keeping it consistent!
            else if ((pMenuVariables->mousex > pButtonsAndBackground[1]->rect.x && pMenuVariables->mousex < 
                        pButtonsAndBackground[1]->rect.x + pButtonsAndBackground[1]->rect.w) && 
                     (pMenuVariables->mousey > pButtonsAndBackground[1]->rect.y && pMenuVariables->mousey < 
                        pButtonsAndBackground[1]->rect.y + pButtonsAndBackground[1]->rect.h))
            {
                Mix_PlayChannel(-1, pButtonSound, 0);  // Play click sound
                pMenuVariables->menuRunning = false;
            }
        }
    }
    else if (pEvent->type == SDL_KEYDOWN)
    {
        switch (pEvent->key.keysym.scancode)
        {
            default: // Handle any unspecified keys
                break;
                
            case SDL_SCANCODE_UP:
                if (pMenuVariables->menuChoice > 1) {
                    pMenuVariables->menuChoice--;
                    Mix_PlayChannel(-1, pButtonSound, 0);
                }
                else
                {
                    pMenuVariables->menuChoice = 1;
                    Mix_PlayChannel(-1, pButtonSound, 0);
                }
                
                break;
                
            case SDL_SCANCODE_DOWN:
                if (pMenuVariables->menuChoice < 2) {
                    pMenuVariables->menuChoice++;
                    Mix_PlayChannel(-1, pButtonSound, 0);
                }
                else
                {
                    pMenuVariables->menuChoice = 2;
                    Mix_PlayChannel(-1, pButtonSound, 0);
                }
                break;
                
            case SDL_SCANCODE_SPACE:
            case SDL_SCANCODE_RETURN:
                Mix_PlayChannel(-1, pButtonSound, 0);
                if (pMenuVariables->menuChoice == 1)
                {
                    pMenuVariables->startGame = true;
                    pMenuVariables->menuRunning = false;
                }
                else if (pMenuVariables->menuChoice == 2)
                {
                    Mix_HaltMusic();
                    pMenuVariables->menuRunning = false;
                }
                break;
        }
    }
}

void cleanMenu(ButtonImages *pButtonImages[NROFPICS], ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS],MenuVariables *pMenuVariables)
{
    for (int i = 0; i < NROFPICS; i++)
    {
        SDL_DestroyTexture(pButtonImages[i]->pTexture);
        free(pButtonImages[i]);
    }
    for (int i = 0; i < NROFBUTTONS; i++)
    {
        if (pButtonsAndBackground[i]->pTexture != NULL)
        {
            SDL_DestroyTexture(pButtonsAndBackground[i]->pTexture);
        }
        if (pButtonsAndBackground[i] != NULL)
        {
            free(pButtonsAndBackground[i]);
        }
    }
    if (pMenuVariables != NULL)
    {
        free(pMenuVariables);
    }
}