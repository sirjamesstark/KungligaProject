#ifndef menu_h
#define menu_h

#define NROFPICS 7
#define NROFBUTTONS 4

typedef struct buttonImages ButtonImages;
typedef struct buttonsAndBackground ButtonsAndBackground;
typedef struct menuVariables MenuVariables;

bool showMenu(SDL_Renderer *pRenderer, int window_width, int window_height);
int createAllImages(ButtonImages *pButtonImages[NROFPICS] ,SDL_Renderer *pRenderer);
void createButtonsAndBackground(ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS], ButtonImages *pButtonImages[NROFPICS],
                                int window_width,int window_height);
void freeAllSurface(SDL_Surface *pBackgroundSurface, SDL_Surface *pStart0Surface,
                    SDL_Surface *pStart1Surface, SDL_Surface *pExit0Surface, 
                    SDL_Surface *pExit1Surface,SDL_Surface *pSoundOnSurface,
                    SDL_Surface *pSoundOffSurface);
void runMenu(ButtonImages *pButtonImages[NROFPICS], ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS], 
                MenuVariables *pMenuVariables, Mix_Chunk *pButtonSound, Mix_Music *pMenuMusic, SDL_Renderer *pRenderer);
void chooseMenuOption(ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS], SDL_Event *pEvent, 
                        Mix_Chunk *pButtonSound, Mix_Music *pMenuMusic,  MenuVariables *pMenuVariables);
void cleanMenu(ButtonImages *pButtonImages[NROFPICS], ButtonsAndBackground *pButtonsAndBackground[NROFBUTTONS],MenuVariables *pMenuVariables);
#endif