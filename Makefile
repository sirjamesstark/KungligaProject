SDL_PATH = C:/msys64/mingw64

AllFile: 
	gcc -o movingTwoMenWithUDP movingTwoMenWithUDP.c -L$(SDL_PATH)/lib -I$(SDL_PATH)/include/SDL2 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_net

clean: 
	del /Q *.o
	del /Q movingTwoMenWithUDP.exe
