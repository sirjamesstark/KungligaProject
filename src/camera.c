#include <SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/camera.h"

struct view
{
    int x, y, h, w;
};

struct camera
{
    View view;
    SDL_Rect *pScreenRect;
    int window_height, window_width;
    int latestY;
    int count;
};

int getCamX(Camera *pCamera)
{
    return pCamera->view.x;
}

int getCamY(Camera *pCamera)
{
    return pCamera->view.y;
}

Camera *createCamera(SDL_Rect *pScreenRect)
{
    Camera *pCamera = malloc(sizeof(struct camera));
    if (pCamera == NULL)
        return NULL;

    pCamera->pScreenRect = pScreenRect;
    pCamera->view.x = pScreenRect->x;
    pCamera->view.y = pScreenRect->y;
    pCamera->view.w = pScreenRect->w;
    pCamera->view.h = pScreenRect->h;
    pCamera->window_height = pScreenRect->h;
    pCamera->window_width = pScreenRect->w;

    return pCamera;
}

void updateCamera(Camera *pCamera, int targetX, int targetY)
{
    // pCamera->view.x = targetX - (pCamera->view.w / 2);
    pCamera->view.y = targetY - (int)round((pCamera->pScreenRect->h + pCamera->pScreenRect->y * 2) / 2.0);
    // pCamera->view.y = targetY - (pCamera->view.h / 2);

    if (pCamera->latestY < pCamera->view.y)
    {
        pCamera->view.y = pCamera->latestY;
    }

    if (pCamera->view.x < pCamera->pScreenRect->x)
        pCamera->view.x = pCamera->pScreenRect->x;
    // if (pCamera->view.y < 0)
    //     pCamera->view.y = 0;

    if (pCamera->view.x + pCamera->view.w > pCamera->pScreenRect->x * 2 + pCamera->pScreenRect->w)
        pCamera->view.x = pCamera->pScreenRect->x * 2 + pCamera->pScreenRect->w;
    if (pCamera->view.y + pCamera->view.h > pCamera->pScreenRect->y * 2 + pCamera->pScreenRect->h)
        pCamera->view.y = pCamera->pScreenRect->y * 2 + pCamera->pScreenRect->h; //- pCamera->view.h;

    pCamera->latestY = pCamera->view.y;
    // if (pCamera->view.x < 0)
    //     pCamera->view.x = 0;
    // // if (pCamera->view.y < 0)
    // //     pCamera->view.y = 0;
    // if (pCamera->view.x + pCamera->view.w > pCamera->window_width)
    //     pCamera->view.x = pCamera->window_width - pCamera->view.w;
    // if (pCamera->view.y + pCamera->view.h > pCamera->window_height)
    //     pCamera->view.y = pCamera->window_height - pCamera->view.h;

    // pCamera->latestY = pCamera->view.y;
}

void destroyCamera(Camera *pCamera)
{
    if (pCamera != NULL)
    {
        free(pCamera);
        pCamera = NULL;
    }
}