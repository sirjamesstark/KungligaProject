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

#define COUNTER 20

typedef struct {
    int window_width;
    int window_height; 
    int speed_x; 
    int speed_y;
    bool continue_game;
} DisplayMode; 

void initiateFullscreen(DisplayMode *pdM);
//void runGame();                             // Det är här spelet körs
//void handleInput();                         // Styr spelarens tangentval
//void exitGame();                            // Här avslutas spelet

int main(int argv, char** args) {
    DisplayMode dM ={0};
    initiateFullscreen(&dM);
    if (dM.continue_game) { // OBS!! Funktionerna innanför denna if-sats bör delas upp i funktioner och andra filer (se förslag på funktioner ovanför)
        SDL_Window* pWindow = SDL_CreateWindow("Enkelt exempel 1",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, dM.window_width, dM.window_height,SDL_WINDOW_FULLSCREEN_DESKTOP);
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

        SDL_Rect playerRect;
        SDL_QueryTexture(pTexture,NULL,NULL,&playerRect.w,&playerRect.h);
        playerRect.w/=6;
        playerRect.h/=6;
        float shipX = (dM.window_width - playerRect.w)/2;//left side
        float shipY = (dM.window_height - playerRect.h)/2;//upper side
        
        float shipVelocityX = 0;//unit: pixels/s
        float shipVelocityY = 0;

        bool closeWindow = false;
        bool up,down,left,right;
        bool onAir = 0;
        up = down = left = right = false;
        int upCounter = COUNTER, downCounter = COUNTER;

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
            
            if(up && !down && !onAir) //    Om man är på golvet och trycker bara upp
            {
                onAir = true;   //Sätter vi på hoppande boolsk variabel på True (Nu hoppar vi)
            }
            if (onAir == true)  //Om vi hoppar just nu
            {
                if (upCounter > 0)      //Om räknaren för hopp är mer än 0
                {
                    shipVelocityY = -(dM.speed_y*5);     //Rörelse upp
                    upCounter--;    // Minska räknaren för hopp
                }
                else        // Om räknaren för hopp är 0
                {
                    if ((downCounter > 0) && (shipY < (dM.window_height - playerRect.h)))   //Om räknaren för gravitation är mer än 0 och karaktären är i luften
                    {
                        shipVelocityY = (dM.speed_y)*5;  //Rörelse ner
                        downCounter--;    // Minska räknare för gravitation
                    }
                    else    // Om man har landat
                    {
                        upCounter = COUNTER;    //Resetta räknare för hpop
                        downCounter = COUNTER;  //Resetta räknare för gravitation
                        onAir = false;  //Vi hoppar inte längre
                    }
                }
            }

            if(left && !right) shipVelocityX = -dM.speed_x;
            if(right && !left) shipVelocityX = dM.speed_x;
            shipX += shipVelocityX/20;//60 frames/s
            shipY += shipVelocityY/60;
            if(shipX<0) shipX=0;
            if(shipY<0) shipY=0;
            if(shipX>dM.window_width-playerRect.w) shipX = dM.window_width-playerRect.w;
            if(shipY>dM.window_height-playerRect.h) shipY = dM.window_height-playerRect.h;
            playerRect.x = shipX;
            playerRect.y = shipY;

            SDL_RenderClear(pRenderer);
            SDL_RenderCopy(pRenderer,pTexture,NULL,&playerRect);
            SDL_RenderPresent(pRenderer);
            SDL_Delay(1000/60);//60 frames/s
        }

        SDL_DestroyTexture(pTexture);
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);

        SDL_Quit();
        return 0;
    }
}

void initiateFullscreen(DisplayMode *pdM) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        pdM->continue_game = false;
        return;
    }

    SDL_DisplayMode windowMode;
    if (SDL_GetCurrentDisplayMode(0, &windowMode) != 0) {
        printf("Failed to get display mode: %s\n", SDL_GetError());
        pdM->continue_game = false;
    }
    else {
        pdM->window_width = windowMode.w;
        pdM->window_height = windowMode.h;
        pdM->speed_x = pdM->window_width /20;;  // Exempel på hastighet i x-led, kan justeras i efterhand
        pdM->speed_y = pdM->window_height/20;   // Exempel på hastighet i y-led, kan justeras i efterhand
        pdM->continue_game = true;
    }
}    
