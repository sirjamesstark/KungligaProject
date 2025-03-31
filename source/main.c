#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>

#include "../include/platform.h"
#include "../include/player.h"

#define PLATTFORM_COUNT

typedef struct {
    int window_width;
    int window_height; 
    int speed_x; 
    int speed_y; 
} displayMode; 

typedef struct {
    SDL_Rect rect;
    bool isOnPlatform;
    float fallFromHeight;
} Platform;


int main(int argv, char** args) {
    
    if (SDL_Init(SDL_INIT_VIDEO)!=0) {
        printf("Error: %s\n",SDL_GetError());
        return 1;
    }

    // Skärminformation för olika typer av skärmar
    SDL_DisplayMode windowMode;
    SDL_GetCurrentDisplayMode(0, &windowMode);
    
    // Initierarr struct
    displayMode DM; 
    DM.window_width = windowMode.w;
    DM.window_height = windowMode.h;
    DM.speed_x = DM.window_width/2; 
    DM.speed_y = DM.window_height/2;  

    SDL_Window* pWindow = SDL_CreateWindow("Enkelt exempel 1",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, DM.window_width, DM.window_height,SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!pWindow) {
        printf("Error: %s\n",SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!pRenderer) {
        printf("Error: %s\n",SDL_GetError());
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;    
    }

    SDL_Surface *pSurface = IMG_Load("resources/Ship.png");
    if(!pSurface){
        printf("Error: %s\n",SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;    
    }
    SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if(!pTexture){
        printf("Error: %s\n",SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;    
    }

    SDL_Rect shipRect;
    SDL_QueryTexture(pTexture,NULL,NULL,&shipRect.w,&shipRect.h);
    shipRect.w/=6;
    shipRect.h/=6;
    float shipX = (DM.window_width - shipRect.w)/2;//left side
    float shipY = (DM.window_height - shipRect.h)/2;//upper side
    
    float shipVelocityX = 0;//unit: pixels/s
    float shipVelocityY = 0;

    bool closeWindow = false;
    bool up,down,left,right;
    up = down = left = right = false;

    while(!closeWindow){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    closeWindow = true;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.scancode){
                        case SDL_SCANCODE_W:
                        case SDL_SCANCODE_UP:
                            up=true;
                            break;
                        case SDL_SCANCODE_A:
                        case SDL_SCANCODE_LEFT:
                            left=true;
                            break;
                        case SDL_SCANCODE_S:
                        case SDL_SCANCODE_DOWN:
                            down=true;
                            break;
                        case SDL_SCANCODE_D:
                        case SDL_SCANCODE_RIGHT:
                            right=true;
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch(event.key.keysym.scancode){
                        case SDL_SCANCODE_W:
                        case SDL_SCANCODE_UP:
                            up=false;
                        break;
                        case SDL_SCANCODE_A:
                        case SDL_SCANCODE_LEFT:
                            left=false;
                        break;
                        case SDL_SCANCODE_S:
                        case SDL_SCANCODE_DOWN:
                            down=false;
                        break;
                        case SDL_SCANCODE_D:
                        case SDL_SCANCODE_RIGHT:
                            right=false;
                        break;
                    }
                    break;
            }
        }

        shipVelocityX = shipVelocityY = 0;
        if(up && !down) shipVelocityY = -DM.speed_y;
        if(down && !up) shipVelocityY = DM.speed_y;
        if(left && !right) shipVelocityX = -DM.speed_x;
        if(right && !left) shipVelocityX = DM.speed_x;
        shipX += shipVelocityX/60;//60 frames/s
        shipY += shipVelocityY/60;
        if(shipX<0) shipX=0;
        if(shipY<0) shipY=0;
        if(shipX>DM.window_width-shipRect.w) shipX = DM.window_width-shipRect.w;
        if(shipY>DM.window_height-shipRect.h) shipY = DM.window_height-shipRect.h;
        shipRect.x = shipX;
        shipRect.y = shipY;

        SDL_RenderClear(pRenderer);
        SDL_RenderCopy(pRenderer,pTexture,NULL,&shipRect);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(1000/60);//60 frames/s
    }

    SDL_DestroyTexture(pTexture);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);

    SDL_Quit();
    return 0;
}