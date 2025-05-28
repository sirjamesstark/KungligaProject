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

CC = gcc

SRCDIR = ./src
INCDIR = ./include
OBJDIR = ./obj

SRC = $(SRCDIR)/main.c $(SRCDIR)/menu.c $(SRCDIR)/platform.c \
       $(SRCDIR)/player.c $(SRCDIR)/theme.c $(SRCDIR)/camera.c \
       $(SRCDIR)/scaling.c $(SRCDIR)/net.c

OBJS = $(OBJDIR)/main.o $(OBJDIR)/menu.o $(OBJDIR)/platform.o \
        $(OBJDIR)/player.o  $(OBJDIR)/theme.o $(OBJDIR)/camera.o \
        $(OBJDIR)/scaling.o $(OBJDIR)/net.o


HEADERS =  $(INCDIR)/menu.h $(INCDIR)/platform.h $(INCDIR)/player.h \
            $(INCDIR)/theme.h $(INCDIR)/camera.h $(INCDIR)/scaling.h \
            $(INCDIR)/scaling.h $(INCDIR)/net.h 

ifeq ($(UNAME_S), Windows)
    MKDIR = if not exist $(subst /,\,$(OBJDIR)) mkdir $(subst /,\,$(OBJDIR))
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    MKDIR = mkdir -p $(OBJDIR)
    RM = rm -f
    RMDIR = rm -rf
endif

LavaRun: $(OBJS)
	$(CC) $(OBJS) -o LavaRun $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR):
	@echo Running: $(MKDIR)
	@$(MKDIR)

clean:
ifeq ($(UNAME_S), Windows)
	@if exist LavaRun.exe $(RM) LavaRun.exe
	@if exist $(NULLCHECK) $(RM) $(OBJDIR)\*.o
	@if exist $(NULLCHECK) $(RMDIR) $(OBJDIR)
else
	$(RM) LavaRun
	$(RM) $(OBJS)
	$(RMDIR) $(OBJDIR)
endif
