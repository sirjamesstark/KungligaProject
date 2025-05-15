#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <string.h>
#include <stdbool.h>
#include "../include/menu.h"
#include "../include/theme.h"
#include "../include/lobby.h"
#define NROFBUTTONSLOBBY 2
#define MAX_TEXT_LENGTH 20

TTF_Font *initTTF();

int runningLobby(SDL_Renderer *pRenderer, SDL_Rect *pScreenRect,bool *pMenuRunning, bool *pStartGame,
                 Audio *pAudio, Background *pBackground,char pIPinput[],Cursor *pCursor) {
    Button *pLobbyButtons[NROFBUTTONSLOBBY];
    SDL_Rect textBox;
    bool lobbyRunning = true;

    TTF_Font *pFont;
    if ((!(pFont = initTTF()))){
        printf("Fail to create pFont\n");
        return 1;
    }

    char inputText[MAX_TEXT_LENGTH] = "";
    SDL_StartTextInput();

    textBox = createInputBox(inputText,pFont,pRenderer,*pScreenRect);

    pLobbyButtons[START] = createButton(pRenderer, pScreenRect, START);
    if (!pLobbyButtons[START]) {
        printf("Error in createlobby: pLobbyButtons[%d] is NULL.\n", START);
        return 1;
    }

    pLobbyButtons[1] = createButton(pRenderer, pScreenRect, BACK);
    if (!pLobbyButtons[1]) {
        printf("Error in createlobby: pLobbyButtons[%d] is NULL.\n", 1);
        return 1;
    }

    SDL_Event event;
    while (lobbyRunning) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    lobbyRunning = false;
                    (*pStartGame) = false;
                    break;
                case SDL_TEXTINPUT:
                    if (strlen(inputText) + strlen(event.text.text) < MAX_TEXT_LENGTH - 1) {
                        strcat(inputText, event.text.text);
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(inputText) > 0) {
                        inputText[strlen(inputText) - 1] = '\0';
                    }
                    break;
                case SDL_MOUSEMOTION:
                case SDL_MOUSEBUTTONDOWN:
                    handleMouseInLobby(&event, pLobbyButtons, pMenuRunning, pStartGame, pAudio, &lobbyRunning, pCursor);
                    break;
                default:
                    break;
            }
        }

        SDL_RenderClear(pRenderer);

        drawBackground(pBackground);
        drawInputBox(pRenderer,pFont,inputText,&textBox);
        playMusic(pAudio);

        for (int i = 0; i < NROFBUTTONSLOBBY; i++) {
            drawButton(pLobbyButtons[i]);
        }
        drawPadding(pRenderer, *pScreenRect);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(1);
    }

    cleanUpLobby(pLobbyButtons,pFont,&textBox);
    strcpy(pIPinput, inputText);
    
    return 0;
}

void handleMouseInLobby(SDL_Event *pEvent, Button *pButtons[], bool *pMenuRunning, bool *pStartGame, Audio *pAudio, bool *pLobbyRunning,Cursor *pCursor) {
    int x = pEvent->motion.x;
    int y = pEvent->motion.y;

    switch (pEvent->type) {
        case SDL_MOUSEMOTION:
            for (int i = 0; i < NROFBUTTONSLOBBY; i++) {
                setButton_isHovered(pButtons[i],pCursor);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (pEvent->button.button == SDL_BUTTON_LEFT) {
                handleMouseClickInLobby(pButtons, pMenuRunning, pStartGame, pAudio, pLobbyRunning);
            }
            break;
    }
}

void handleMouseClickInLobby(Button *pButtons[], bool *pMenuRunning, bool *pStartGame, Audio *pAudio, bool *pLobbyRunning) {
    if (isButtonPushed(pButtons[0])) { // start
        playButtonSound(pAudio);
        *pMenuRunning = false;
        *pStartGame = true;
        *pLobbyRunning = false;
    }
    else if (isButtonPushed(pButtons[1])) { // back
        playButtonSound(pAudio);
        *pMenuRunning = true;
        *pStartGame = false;
        *pLobbyRunning = false;
    }
}

TTF_Font *initTTF(){
    if (TTF_Init() < 0) {
        printf("TTF Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return NULL;
    }

    TTF_Font *font = TTF_OpenFont("resources/Gloss_And_Bloom.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }

    return font;
}

SDL_Rect createInputBox(char inputText[],TTF_Font *pFont, SDL_Renderer *pRenderer, SDL_Rect screenRect){
    SDL_Rect inputRect = {(int)((screenRect.x * 2 + screenRect.w - 300) / 2.0f + 0.5f), 
                               ((screenRect.y * 2 + screenRect.h - 40 ) / 2.0f + 0.5f), 300, 40};
    return inputRect;
}

void drawInputBox(SDL_Renderer* pRenderer, TTF_Font *pFont, char* inputText, SDL_Rect* box) {
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255); //Box colur
    SDL_RenderFillRect(pRenderer, box);

    if (!inputText[0]) return;

    SDL_Color textColor = { 255, 255, 255, 255 }; // WHITE
    SDL_Surface* textSurface = TTF_RenderText_Blended(pFont, inputText, textColor);
    if(!textSurface){
        SDL_Log("Failed to render text: %s", TTF_GetError());
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(pRenderer, textSurface);
    if (!textTexture) {
        SDL_Log("Failed to make text texture: %s", TTF_GetError());
        return;//
    }
    SDL_Rect textRect = { box->x + 5, box->y + 5, textSurface->w, textSurface->h };
    SDL_RenderCopy(pRenderer, textTexture, NULL, &textRect);
    
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void cleanUpLobby(Button *pLobbyButtons[], TTF_Font *pFont, SDL_Rect *pTextBox){
    for (int i = 0; i < NROFBUTTONSLOBBY; i++)
    {
        destroyButton(pLobbyButtons[i]);
    } 

    SDL_StopTextInput();
    TTF_CloseFont(pFont);
    TTF_Quit();
}