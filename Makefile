# Bestämmer om systemet är Windows eller Unix-liknande
ifeq ($(OS),Windows_NT)
    UNAME_S = Windows
else
    UNAME_S := $(shell uname -s)
endif

# Kompilatorn (beroende på plattform)
CC = gcc
ifeq ($(UNAME_S),Darwin)
    INCLUDE = /opt/homebrew/include/SDL2
    LIBS = /opt/homebrew/lib
else ifeq ($(UNAME_S),Linux)
    INCLUDE = /usr/include/SDL2
    LIBS = /usr/lib
else  # Windows
    INCLUDE = C:/path/to/SDL2/include
    LIBS = C:/path/to/SDL2/lib
endif

# Sökvägar relativt Makefilen
SRCDIR = ./source
INCDIR = ./include
OBJDIR = ./obj

# Flaggor
CFLAGS = -g -I$(INCLUDE) -c
LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm

# Lista över filer
SRC = $(SRCDIR)/main.c $(SRCDIR)/platform.c $(SRCDIR)/player.c
OBJS = $(OBJDIR)/main.o $(OBJDIR)/platform.o $(OBJDIR)/player.o
HEADERS = $(INCDIR)/platform.h $(INCDIR)/player.h

# Välj rätt kommando för att skapa kataloger och radera filer
ifeq ($(UNAME_S),Windows)
    MKDIR = if not exist $(OBJDIR) mkdir $(OBJDIR)
    RM = del /Q
    RMDIR = rmdir /S /Q
else
	MKDIR = mkdir -p $(OBJDIR)
    RM = rm -f
    RMDIR = rm -rf
endif

# Skapar en exekverbar fil utifrån .o-filerna
KungligaProjekt: $(OBJS)
	$(CC) $(OBJS) -o KungligaProjekt $(LDFLAGS)

# Kompilerar källfiler till objektfiler
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $<

# Skapar obj-mappen om den inte finns
$(OBJDIR):
	@$(MKDIR)

# Städar upp genererade filer och tar bort obj-mappen
clean:
ifeq ($(UNAME_S),Windows)
	$(RM) KungligaProjekt.exe
	$(RM) $(OBJDIR)\*.o
	$(RMDIR) $(OBJDIR)
else
	$(RM) KungligaProjekt
	$(RM) $(OBJS)
	$(RMDIR) $(OBJDIR)
endif