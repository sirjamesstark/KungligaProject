# Bestämmer om systemet är Windows eller Unix-liknande
UNAME_S := $(shell uname -s)

# Kompilatorn (beroende på plattform)
CC = gcc-14
ifeq ($(UNAME_S),Darwin)
    # macOS (kanske en annan version av GCC, här använder vi gcc-14 som exempel)
    INCLUDE = /opt/homebrew/include/SDL2
    LIBS = /opt/homebrew/lib
else
    # Windows (t.ex. MinGW eller liknande, justera för din miljö)
    INCLUDE = C:/path/to/SDL2/include
    LIBS = C:/path/to/SDL2/lib
endif

# Sökvägar relativt Makefilen:
# Källkodsfilernas mapp med .c-filer (funktionsdefinitioner)
SRCDIR = ./source
# Headerfilernas mapp med .h-filer (funktionsdeklarationer)
INCDIR = ./include
# Objektfilernas mapp med .o-filer (filerna genereras efter kompilering)
OBJDIR = ./obj

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
$(OBJDIR)/main.o: $(SRCDIR)/main.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/main.o $(SRCDIR)/main.c

# Kompilerar platform.c till platform.o
$(OBJDIR)/platform.o: $(SRCDIR)/platform.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/platform.o $(SRCDIR)/platform.c

# Kompilerar player.c till player.o
$(OBJDIR)/player.o: $(SRCDIR)/player.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/player.o $(SRCDIR)/player.c

# Skapar obj-mappen om den inte finns
$(OBJDIR):
ifeq ($(UNAME_S),Darwin)  # macOS
	@mkdir -p $(OBJDIR)
else ifeq ($(UNAME_S),Linux)  # Linux
	@mkdir -p $(OBJDIR)
else  # Windows
	@mkdir $(OBJDIR)
endif

# Städar upp genererade filer (körbar fil och .o-filer)
clean:
ifeq ($(UNAME_S),Darwin)  # macOS
	rm -f KungligaProjekt
	rm -f $(OBJS)
else ifeq ($(UNAME_S),Linux)  # Linux
	rm -f KungligaProjekt
	rm -f $(OBJS)
else  # Windows
	del KungligaProjekt.exe
	del $(OBJDIR)\*.o
endif