CC=gcc-14
SRCDIR=./source
INCDIR=./include

INCLUDE = /opt/homebrew/include/SDL2
LIBS = /opt/homebrew/lib
CFLAGS = -g -I$(INCLUDE) -c
LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lm


simpleSDLexample1: main.o
	$(CC) main.o -o simpleSDLexample1 $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

clean:
	rm simpleSDLexample1
	rm *.o
