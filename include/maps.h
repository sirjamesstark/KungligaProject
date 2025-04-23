#ifndef maps_h
#define maps_h

#define NROFMAPS 1

typedef struct maps Maps;

void drawBlueprints(Maps *pMaps[NROFMAPS]);
Maps* drawMap1();
void chooseMap(int gameMap[BOX_ROW][BOX_COL],Maps *pChosenMap);
void destroyMap(Maps *pMap);

#endif