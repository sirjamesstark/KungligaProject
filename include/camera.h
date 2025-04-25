#ifndef CAMERA_H
#define CAMERA_H
#include <SDL.h>

typedef struct camera Camera;
typedef struct view View;

Camera *camera(int screenWidth, int screenHeight);
int getCamX(Camera *pCamera);
int getCamY(Camera *pCamera);
void updateCamera(Camera *pCamera, int targetX, int targetY);
void destroyCamera(Camera *pCamera);

#endif
