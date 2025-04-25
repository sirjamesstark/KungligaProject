# Identifierar operativsystem: Windows eller macOS
# INCLUDE anger fullständig sökväg till SDL2:s headerfiler
# LIB anger fullständig sökväg till SDL2:s bibliotek
# CFLAGS & LDFLAGS säts för att bygga och länka SDL2 bibliotek till operativsystemet
ifeq ($(OS), Windows_NT)
    UNAME_S = Windows
    INCLUDE = C:/msys64/mingw64/include/SDL2
    LIBS = C:/msys64/mingw64/lib
    CFLAGS = -g -I$(INCLUDE) -Dmain=SDL_main -c
    LDFLAGS = -L$(LIBS) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm
else
    UNAME_S := $(shell uname -s)
    INCLUDE = /opt/homebrew/include/SDL2
    LIBS = /opt/homebrew/lib
    CFLAGS = -g -I$(INCLUDE) -c
    LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm
endif

# Kompilator GCC
CC = gcc

# Sökvägar relativt Makefilen
SRCDIR = ./src
INCDIR = ./include
OBJDIR = ./obj

# Lista över filer
SRC = $(SRCDIR)/main.c $(SRCDIR)/menu.c $(SRCDIR)/platform.c \
       $(SRCDIR)/player.c $(SRCDIR)/theme.c $(SRCDIR)/maps.c \
       $(SRCDIR)/camera.c

OBJS = $(OBJDIR)/main.o $(OBJDIR)/menu.o $(OBJDIR)/platform.o \
        $(OBJDIR)/player.o  $(OBJDIR)/theme.o $(OBJDIR)/maps.o \
        $(OBJDIR)/camera.o

HEADERS =  $(INCDIR)/menu.h $(INCDIR)/platform.h $(INCDIR)/player.h \
            $(INCDIR)/theme.h $(INCDIR)/maps.h $(INCDIR)/camera.h


# Välj rätt kommando för att skapa kataloger och radera filer

ifeq ($(UNAME_S), Windows)
    MKDIR = if not exist $(subst /,\,$(OBJDIR)) mkdir $(subst /,\,$(OBJDIR))
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    MKDIR = mkdir -p $(OBJDIR)
    RM = rm -f
    RMDIR = rm -rf
endif

# Skapar en exekverbar fil utifrån .o-filerna
KungligaProject: $(OBJS)
	$(CC) $(OBJS) -o KungligaProject $(LDFLAGS)

# Kompilerar källfiler till objektfiler
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $<

# Skapar obj-mappen om den inte finns
$(OBJDIR):
	@echo Running: $(MKDIR)
	@$(MKDIR)

# Städar upp genererade filer och tar bort obj-mappen
clean:
ifeq ($(UNAME_S), Windows)
	@if exist KungligaProject.exe $(RM) KungligaProject.exe
	@if exist $(NULLCHECK) $(RM) $(OBJDIR)\*.o
	@if exist $(NULLCHECK) $(RMDIR) $(OBJDIR)
else
	$(RM) KungligaProject
	$(RM) $(OBJS)
	$(RMDIR) $(OBJDIR)
endif
