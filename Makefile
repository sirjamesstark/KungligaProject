CC = gcc-14 # Kompilatorn

# Sökvägar relativt Makefilen:
# Källkodsfilernas mapp med .c-filer (funktionsdefinitioner)
SRCDIR = ./source
# Headerfilernas mapp med .h-filer (funktionsdeklarationer)
INCDIR = ./include
# Objektfilernas mapp med .o-filer (filerna genereras efter kompilering)
OBJDIR = ./obj

# Fullständiga sökvägar till SDL2
# Sökväg till SDL2:s headerfiler 
INCLUDE = /opt/homebrew/include/SDL2
# Sökväg till SLD2:s bibliotek
LIBS = /opt/homebrew/lib

# Flaggor
# Kompilatorflaggor som används vid kompliering av egna .c-filer samt ger kompliatorn information om SLD2:s headerfiler
CFLAGS = -g -I$(INCLUDE) -c
# Länkflaggor som ser till att länkaren kan hitta SDL2:s bibliotek som ska användas i spelet
LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm

# Listar projektets filer
# Listar samtliga källkodsfilerna (.c)
SRC = $(SRCDIR)/main.c $(SRCDIR)/platform.c $(SRCDIR)/player.c
# Listar samtliga objektfiler (.o)
OBJS = $(OBJDIR)/main.o $(OBJDIR)/platform.o $(OBJDIR)/player.o
# Listar samtliga headerfiler (.h)
HEADERS = $(INCDIR)/platform.h $(INCDIR)/player.h

# Skapar en exekverbar fil utifrån .o-filerna
KungligaProjekt: $(OBJS)
	$(CC) $(OBJS) -o KungligaProjekt $(LDFLAGS)

# Kompilerar main.c till main.o
$(OBJDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/main.o $(SRCDIR)/main.c

# Kompilerar platform.c till platform.o
$(OBJDIR)/platform.o: $(SRCDIR)/platform.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/platform.o $(SRCDIR)/platform.c

# Kompilerar player.c till player.o
$(OBJDIR)/player.o: $(SRCDIR)/player.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/player.o $(SRCDIR)/player.c

# Städar upp genererade filer (körbar fil och .o-filer)
clean:
	rm KungligaProjekt
	rm $(OBJS)