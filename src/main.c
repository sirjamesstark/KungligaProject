#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>

#include "../include/game.h"
#include "../include/menu.h"
#include "../include/platform.h"
#include "../include/player.h"
#include "../include/renderer.h"

#define NUM_MENU 2
#define MAX_NROFPLAYERS 4


typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
    Mix_Chunk *jumpSound;
    // AsteroidImage *pAsteroidImage;
    // Asteroid *pAsteroids[MAX_ASTEROIDS];
} Game;

typedef struct {
    int window_width;
    int window_height;
    int speed_x;
    int speed_y;
    bool continue_game;
} DisplayMode;

int initiate(DisplayMode *pdM,Game *pGame);
bool showMenu(Game *pGame, DisplayMode position);
void handleInput(Game *pGame,SDL_Event *pEvent,bool *pCloseWindow,
                bool *pUp,bool *pDown,bool *pLeft,bool *pRight);
void cleanUp(Game *pGame);

int main(int argc, char *argv[])
{
    Game game = {0};
    DisplayMode dM = {0};

    bool startGame = false;

    // Initialize SDL_image for PNG loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    if (!initiate(&dM,&game))
    {
        return 1;
    } if (!showMenu(&game, dM))
    {
        SDL_DestroyRenderer(game.pRenderer);
        SDL_DestroyWindow(game.pWindow);
        SDL_Quit();
        return 1;
    }

    // Load and play game music
    Mix_Music *gameMusic = Mix_LoadMUS("resources/game_music.wav");
    if (!gameMusic) {
        printf("Failed to load game music! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        Mix_VolumeMusic((int)(MIX_MAX_VOLUME * 0.5));  // Set volume to 50%
        Mix_PlayMusic(gameMusic, -1);  // -1 means loop infinitely
    }

    // Load game background
    SDL_Surface *pBackgroundSurface = IMG_Load("resources/game_background.png");
    if (!pBackgroundSurface)
    {
        printf("Error loading background: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Texture *pBackgroundTexture = SDL_CreateTextureFromSurface(game.pRenderer, pBackgroundSurface);
    SDL_FreeSurface(pBackgroundSurface);
    if (!pBackgroundTexture)
    {
        printf("Error creating background texture: %s\n", SDL_GetError());
        return 0;
    }

    // Create background rect
    SDL_Rect backgroundRect;
    backgroundRect.x = 0;
    backgroundRect.y = 0;
    backgroundRect.w = dM.window_width;
    backgroundRect.h = dM.window_height;

    // Load platform blocks
    SDL_Surface *pBlockSurface = IMG_Load("resources/box8.png");
    if (!pBlockSurface)
    {
        printf("Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_Texture *pBlockTexture = SDL_CreateTextureFromSurface(game.pRenderer, pBlockSurface);
    SDL_FreeSurface(pBlockSurface);
    if (!pBlockTexture)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(game.pRenderer);
        SDL_DestroyWindow(game.pWindow);
        SDL_Quit();
        return 1;
    }

    SDL_Rect blockRect;
    // Calculate block size to exactly fit window
    blockRect.w = dM.window_width / BOX_COL;
    blockRect.h = dM.window_height / BOX_ROW;
    blockRect.x = 0;
    blockRect.y = 0;

    (&game)->pPlayer = createPlayer(blockRect,(&game)->pRenderer,dM.window_width,dM.window_height);

    bool closeWindow = false;
    bool up, down, left, right, goUp, goDown, goLeft, goRight;
    bool onGround = true;
    up = down = left = right = false;
    int upCounter = 0;

    Uint32 lastTime = SDL_GetTicks(); // Tidpunkt för senaste uppdateringen
    Uint32 currentTime;
    float deltaTime;

    int gameMap[BOX_ROW][BOX_COL] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
                                     1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,
                                     0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,
                                     0,1,0,0,0,0,0,1,1,0,0,1,1,1,1,0,0,0,0,1,0,0,0,1,1,0,
                                     0,1,0,0,1,1,0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,
                                     0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,
                                     0,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,0,
                                     0,1,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,0,
                                     0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                                     0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
                                     0,1,0,1,1,1,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,1,0,
                                     0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,1,0,0,1,1,0,
                                     0,1,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,0,
                                     0,1,1,0,0,1,1,0,1,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,1,0,
                                     0,1,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,0,1,0,0,0,0,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,
                                     0,1,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,1,1,0,
                                     0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};
    while (!closeWindow)
    {
        // Beräkna tid sedan senaste frame
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; // Omvandla till sekunder
        lastTime = currentTime;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if(event.type==SDL_QUIT) closeWindow = true;
            else handleInput(&game,&event,&closeWindow,&up,&down,&left,&right);
        }
        goDown = goLeft = goRight = goUp = 0;
        setSpeed(up,down,left,right,&goUp,&goDown,&goLeft,&goRight,&upCounter,onGround,game.pPlayer,dM.speed_x,dM.speed_y);
        updatePlayer(game.pPlayer,deltaTime,gameMap,blockRect,&upCounter,&onGround,&goUp,&goDown,&goLeft,&goRight);
        SDL_RenderClear(game.pRenderer);
        
        // Draw background first
        SDL_RenderCopy(game.pRenderer, pBackgroundTexture, NULL, &backgroundRect);

        int numBlocksX = dM.window_width / blockRect.w;  // Antal lådor per rad
        int numBlocksY = (dM.window_height / blockRect.h);  // Antal rader
        for (int row = 0; row < numBlocksY; row++) {
            for (int col = 0; col < numBlocksX; col++) {
                if (gameMap[row][col] == 1)
                {
                    // Position blocks without any gaps
                    blockRect.x = col * blockRect.w;
                    blockRect.y = row * blockRect.h;

                    SDL_RenderCopy(game.pRenderer, pBlockTexture, NULL, &blockRect);
                }
            }
        }

        // Draw player on top of platforms
        drawPlayer(game.pPlayer);

        SDL_RenderPresent(game.pRenderer);
        SDL_Delay(1); // Undvik 100% CPU-användning men låt SDL hantera FPS
    }

    SDL_DestroyTexture(pBackgroundTexture);
    // Stop and free game music
    Mix_HaltMusic();
    Mix_FreeMusic(gameMusic);
    
    cleanUp(&game);
    return 0;
}

int initiate(DisplayMode *pdM,Game *pGame)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }

    SDL_DisplayMode currentDisplay;
    if (SDL_GetCurrentDisplayMode(0, &currentDisplay) != 0) {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }

    pdM->window_width = currentDisplay.w;
    pdM->window_height = currentDisplay.h;
    
    /*
    float aspect_ratio = (float)(pdM->window_width/pdM->window_height);
    if (aspect_ratio != (16.0f / 9.0f)) {
        // Här tvingar vi till en 16:9 aspect ratio genom att justera höjd eller bredd
        if (aspect_ratio > (16.0f / 9.0f)) {
            // Om bredden är för stor (mer än 16:9)
            pdM->window_width = pdM->window_height * 16 / 9;
        } else {
            // Om höjden är för stor (mer än 9:16)
            pdM->window_height = pdM->window_width * 9 / 16;
        }
    }
    */

    pdM->speed_x = pdM->window_width / 20;
    pdM->speed_y = pdM->window_height / 20;

    // Create window with explicit cursor support
    pGame->pWindow = SDL_CreateWindow("Meny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pdM->window_width, pdM->window_height, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pGame->pWindow)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }
    
    pGame->jumpSound = NULL;  // Initialize jump sound to NULL

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Error: %s\n", SDL_GetError());
        cleanUp(pGame);
        return 0;
    }

    // Initialize SDL_image for cursor loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 0;
    }

    // Force cursor to be visible first
    SDL_ShowCursor(SDL_ENABLE);

    // Load and set custom cursor
    SDL_Surface* cursorSurface = IMG_Load("resources/cursor.png");
    if (!cursorSurface)
    {
        printf("Failed to load cursor image! SDL_image Error: %s\n", IMG_GetError());
    }
    else
    {
        // Create cursor with hotspot at top-left for better precision
        SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 0, 0);
        if (!cursor)
        {
            printf("Failed to create cursor! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            SDL_SetCursor(cursor);
            // Make sure cursor stays visible
            SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
        }
        SDL_FreeSurface(cursorSurface);
    }

    // Double check cursor visibility
    if (SDL_ShowCursor(-1) != SDL_ENABLE) {
        SDL_ShowCursor(SDL_ENABLE);
    }

    return 1;
}

// I'm setting up the audio system here - this is where all the menu magic happens
bool showMenu(Game *pGame, DisplayMode position)
{

    // Let's get that sweet background music going - it'll loop forever with -1
    Mix_Music *menuMusic = Mix_LoadMUS("resources/menu_music.wav");
    if (!menuMusic) {
        printf("Failed to load menu music! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    Mix_PlayMusic(menuMusic, -1);  // -1 means loop infinitely

    // Load button click sound effect
    Mix_Chunk *buttonSound = Mix_LoadWAV("resources/button_selection_sound.wav");
    if (!buttonSound) {
        printf("Failed to load button sound! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_FreeMusic(menuMusic);
        return false;
    }

    int menuChoice = 1;
    SDL_Surface *pBackgroundSurface = IMG_Load("resources/main_background.png");
    if (!pBackgroundSurface) {
        printf("Failed to load main_background.png: %s\n", IMG_GetError());
        return false;
    }

    SDL_Surface *pStart0Surface = IMG_Load("resources/menu_start0.png");
    if (!pStart0Surface) {
        printf("Failed to load menu_start0.png: %s\n", IMG_GetError());
        SDL_FreeSurface(pBackgroundSurface);
        return false;
    }

    SDL_Surface *pStart1Surface = IMG_Load("resources/menu_start1.png");
    if (!pStart1Surface) {
        printf("Failed to load menu_start1.png: %s\n", IMG_GetError());
        SDL_FreeSurface(pBackgroundSurface);
        SDL_FreeSurface(pStart0Surface);
        return false;
    }

    SDL_Surface *pExit0Surface = IMG_Load("resources/menu_exit0.png");
    if (!pExit0Surface) {
        printf("Failed to load menu_exit0.png: %s\n", IMG_GetError());
        SDL_FreeSurface(pBackgroundSurface);
        SDL_FreeSurface(pStart0Surface);
        SDL_FreeSurface(pStart1Surface);
        return false;
    }

    SDL_Surface *pExit1Surface = IMG_Load("resources/menu_exit1.png");
    if (!pExit1Surface) {
        printf("Failed to load menu_exit1.png: %s\n", IMG_GetError());
        SDL_FreeSurface(pBackgroundSurface);
        SDL_FreeSurface(pStart0Surface);
        SDL_FreeSurface(pStart1Surface);
        SDL_FreeSurface(pExit0Surface);
        return false;
    }

    SDL_Surface *pSoundOnSurface = IMG_Load("resources/soundon0.png");
    SDL_Surface *pSoundOffSurface = IMG_Load("resources/soundoff0.png");

    if (!pSoundOnSurface || !pSoundOffSurface) {
        printf("Error loading sound icons: %s\n", IMG_GetError());
        SDL_FreeSurface(pBackgroundSurface);
        SDL_FreeSurface(pStart0Surface);
        SDL_FreeSurface(pStart1Surface);
        SDL_FreeSurface(pExit0Surface);
        SDL_FreeSurface(pExit1Surface);
        return false;
    }

    SDL_Texture *pBackgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pBackgroundSurface);
    SDL_Texture *pStart0Texture = SDL_CreateTextureFromSurface(pGame->pRenderer, pStart0Surface);
    SDL_Texture *pStart1Texture = SDL_CreateTextureFromSurface(pGame->pRenderer, pStart1Surface);
    SDL_Texture *pExit0Texture = SDL_CreateTextureFromSurface(pGame->pRenderer, pExit0Surface);
    SDL_Texture *pExit1Texture = SDL_CreateTextureFromSurface(pGame->pRenderer, pExit1Surface);
    SDL_Texture *pSoundOnTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pSoundOnSurface);
    SDL_Texture *pSoundOffTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pSoundOffSurface);

    if (!pBackgroundTexture || !pStart0Texture || !pStart1Texture || !pExit0Texture || !pExit1Texture || 
        !pSoundOnTexture || !pSoundOffTexture)
    {
        printf("Error creating textures: %s\n", SDL_GetError());
        return false;
    }

    SDL_FreeSurface(pBackgroundSurface);
    SDL_FreeSurface(pStart0Surface);
    SDL_FreeSurface(pStart1Surface);
    SDL_FreeSurface(pExit0Surface);
    SDL_FreeSurface(pExit1Surface);
    SDL_FreeSurface(pSoundOnSurface);
    SDL_FreeSurface(pSoundOffSurface);

    if (!pBackgroundTexture || !pStart0Texture || !pStart1Texture || !pExit0Texture || !pExit1Texture || 
        !pSoundOnTexture || !pSoundOffTexture)
    {
        printf("Error creating textures: %s\n", SDL_GetError());
        return false;
    }

    SDL_Rect startRect;
    // Make menu items a bit larger and adjust their positions
    startRect.w = position.window_width / 4;  // Larger width
    startRect.h = position.window_height / 8; // Larger height
    startRect.x = (position.window_width - startRect.w) / 2;
    startRect.y = (position.window_height / 3) - 60;  // Moved up by 1.6cm (≈60 pixels = original 68 - 8)

    SDL_Rect exitRect;
    exitRect.w = position.window_width / 4;
    exitRect.h = position.window_height / 8;
    exitRect.x = (position.window_width - exitRect.w) / 2;
    exitRect.y = (position.window_height / 2) + 57;  // Below start button + 1.5cm (57 pixels = original 38 + 19)
    SDL_Rect backgroundRect;
    backgroundRect.x = 0;
    backgroundRect.y = 0;
    backgroundRect.w = position.window_width;
    backgroundRect.h = position.window_height;

    SDL_Rect soundRect;
    soundRect.w = ((position.window_width) / 15);  // Slightly larger
    soundRect.h = ((position.window_height) / 8);  // Match menu item height
    soundRect.x = (position.window_width - soundRect.w - 58);  // Move right by 1cm (96 - 38)
    soundRect.y = 39;  // Move up by 0.5cm (58 - 19)

    int mousex, mousey;
    bool menuRunning = true;
    bool startGame = false;
    bool soundgame = true;
    bool muteHover = false;

    while (menuRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
           if (event.type == SDL_QUIT)
            {
                menuRunning = false;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&mousex, &mousey);
                if ((mousex > exitRect.x && mousex < exitRect.x + exitRect.w) && (mousey > exitRect.y && mousey < exitRect.y + exitRect.h))
                {
                    menuChoice = 2;
                }
                else if ((mousex > startRect.x && mousex < startRect.x + startRect.w) && (mousey > startRect.y && mousey < startRect.y + startRect.h))
                {
                    menuChoice = 1;
                }
                else
                {
                    menuChoice = 0;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (SDL_BUTTON_LEFT == event.button.button)
                {
                    bool buttonClicked = false;

                    // Handle sound toggle button click
                    // When you click the sound icon, I'll toggle the music for you
                    if ((mousex > soundRect.x && mousex < soundRect.x + soundRect.w) && 
                        (mousey > soundRect.y && mousey < soundRect.y + soundRect.h))
                    {
                        Mix_PlayChannel(-1, buttonSound, 0);  // Play click sound
                        if (soundgame)
                        {
                            Mix_PauseMusic();  // Pause menu music
                            soundgame = false;
                        }
                        else
                        {
                            Mix_ResumeMusic();  // Resume menu music
                            soundgame = true;
                        }
                    }
                    // When you click Start, I'll play a sound and kick off the game
                    else if ((mousex > startRect.x && mousex < startRect.x + startRect.w) && 
                             (mousey > startRect.y && mousey < startRect.y + startRect.h))
                    {
                        Mix_PlayChannel(-1, buttonSound, 0);  // Play click sound
                        SDL_Delay(50);  // Brief delay for sound
                        Mix_HaltMusic();  // Stop menu music
                        Mix_FreeMusic(menuMusic);
                        startGame = true;
                        menuRunning = false;
                    }
                    // Exit button gets a sound too - keeping it consistent!
                    else if ((mousex > exitRect.x && mousex < exitRect.x + exitRect.w) && 
                             (mousey > exitRect.y && mousey < exitRect.y + exitRect.h))
                    {
                        Mix_PlayChannel(-1, buttonSound, 0);  // Play click sound
                        menuRunning = false;
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.scancode)
                {
                    default: // Handle any unspecified keys
                        break;
                        
                    case SDL_SCANCODE_UP:
                        if (menuChoice > 1) {
                            menuChoice--;
                            Mix_PlayChannel(-1, buttonSound, 0);
                        }
                        break;
                        
                    case SDL_SCANCODE_DOWN:
                        if (menuChoice < 3) {
                            menuChoice++;
                            Mix_PlayChannel(-1, buttonSound, 0);
                        }
                        break;
                        
                    case SDL_SCANCODE_SPACE:
                    case SDL_SCANCODE_RETURN:
                        Mix_PlayChannel(-1, buttonSound, 0);
                        if (menuChoice == 1)
                        {
                            startGame = true;
                            menuRunning = false;
                        }
                        else if (menuChoice == 2)
                        {
                            Mix_HaltMusic();
                            soundgame = false;
                        }
                        else
                        {
                            menuRunning = false;
                        }
                        break;
                }
            }
        }

        // Clear renderer and draw background first
        SDL_RenderClear(pGame->pRenderer);
        SDL_RenderCopy(pGame->pRenderer, pBackgroundTexture, NULL, &backgroundRect);

        // Render sound icon based on state
        if (soundgame) {
            SDL_RenderCopy(pGame->pRenderer, pSoundOnTexture, NULL, &soundRect);
        } else {
            SDL_RenderCopy(pGame->pRenderer, pSoundOffTexture, NULL, &soundRect);
        }

        // Default state (no hover)
        if (menuChoice == 0)
        {
            SDL_RenderCopy(pGame->pRenderer, pStart0Texture, NULL, &startRect);
            SDL_RenderCopy(pGame->pRenderer, pExit0Texture, NULL, &exitRect);
        }
        // Start button hover
        else if (menuChoice == 1)
        {
            SDL_RenderCopy(pGame->pRenderer, pStart1Texture, NULL, &startRect);
            SDL_RenderCopy(pGame->pRenderer, pExit0Texture, NULL, &exitRect);
        }
        // Exit button hover
        else if (menuChoice == 2)
        {
            SDL_RenderCopy(pGame->pRenderer, pStart0Texture, NULL, &startRect);
            SDL_RenderCopy(pGame->pRenderer, pExit1Texture, NULL, &exitRect);
        }
        SDL_RenderPresent(pGame->pRenderer);
    }

    SDL_DestroyTexture(pStart0Texture);
    SDL_DestroyTexture(pStart1Texture);
    SDL_DestroyTexture(pExit0Texture);
    SDL_DestroyTexture(pExit1Texture);
    SDL_DestroyTexture(pSoundOnTexture);
    SDL_DestroyTexture(pSoundOffTexture);
    SDL_DestroyTexture(pBackgroundTexture);

    // Clean up all audio resources
    if (!startGame) {
        Mix_HaltMusic();  // Stop music if exiting without starting game
        Mix_FreeMusic(menuMusic);
    }
    
    // Wait a tiny bit to ensure sound effects finish playing
    SDL_Delay(100);
    Mix_FreeChunk(buttonSound);
    Mix_CloseAudio();
    
    return startGame;
}

void handleInput(Game *pGame,SDL_Event *pEvent,bool *pCloseWindow,
    bool*pUp,bool *pDown,bool *pLeft,bool *pRight)
{
    // First time jumping? I'll load the sound - then it's ready for next time
    if (!pGame->jumpSound) {
        pGame->jumpSound = Mix_LoadWAV("resources/jump_sound.wav");
        if (!pGame->jumpSound) {
            printf("Failed to load jump sound! SDL_mixer Error: %s\n", Mix_GetError());
        }
    }

    if(pEvent->type == SDL_KEYDOWN)
    {
        switch (pEvent->key.keysym.scancode)
        {
            default: // Handle any unspecified keys
                break;

            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_SPACE:
                (*pUp) = true;
                if (pGame->jumpSound) {
                    Mix_PlayChannel(-1, pGame->jumpSound, 0);  // Play jump sound
                }
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                (*pLeft) = true;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                (*pDown) = true;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                (*pRight) = true;
                break;
            case SDL_SCANCODE_ESCAPE:
                if (pGame->jumpSound) {
                    Mix_FreeChunk(pGame->jumpSound);  // Clean up jump sound
                    pGame->jumpSound = NULL;
                }
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
                (*pUp) = false;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                (*pLeft) = false;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                (*pDown) = false;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                (*pRight) = false;
                break;
        }
    }
}

void cleanUp(Game *pGame) {
    if (pGame == NULL) return;

    /*
    if (pGame->pTexture != NULL) {
        SDL_DestroyTexture(pGame->pTexture);
        pGame->pTexture = NULL; 
    }
    */

    if (pGame->pRenderer != NULL) {
        SDL_DestroyRenderer(pGame->pRenderer);
        pGame->pRenderer = NULL;
    }

    if (pGame->pWindow != NULL) {
        SDL_DestroyWindow(pGame->pWindow);
        pGame->pWindow = NULL;
    }

    destroyPlayer(pGame->pPlayer);
    /*
    for (int i=0; i<MAX_NROFPLAYERS; i++) {
        if (pGame->pPlayers[i] != NULL) {
            destroyPlayer(pGame->pPlayer[i]);
            pGame->pPlayer[i] = NULL; 
        }
    }
    */

    // Här lägger vi till mer kod som frigör tidigare allokerat minne ifall det behövs (t.ex. för platforms sen)
    SDL_Quit();
}