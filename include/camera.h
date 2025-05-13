#ifndef CAMERA_H
#define CAMERA_H
#include <SDL.h>

typedef struct camera Camera;
typedef struct view View;

Camera *createCamera(SDL_Rect *pScreenRect);
int getCamX(Camera *pCamera);
int getCamY(Camera *pCamera);
void updateCamera(Camera *pCamera, int targetX, float targetY);
void destroyCamera(Camera *pCamera);

#endif
