helloSDL: main.o
	gcc-14 -o helloSDL main.o -lSDL2main -lSDL2 -L/opt/homebrew/lib

main.o: ./source/main.c
	gcc-14 -c -g -I/opt/homebrew/include/SDL2 ./source/main.c

clean:
	rm helloSDL
	rm *.o