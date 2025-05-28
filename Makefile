ifeq ($(OS), Windows_NT)
    UNAME_S = Windows
    INCLUDE = C:/msys64/mingw64/include/SDL2
    LIBS = C:/msys64/mingw64/lib
    CFLAGS = -g -I$(INCLUDE) -Dmain=SDL_main -DUSE_FFMPEG -c
    LDFLAGS = -L$(LIBS) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lavformat -lavcodec -lavutil -lswscale -lswresample -lm
else
    UNAME_S := $(shell uname -s)
    INCLUDE = /opt/homebrew/include/SDL2
    FFMPEG_INCLUDE = /opt/homebrew/Cellar/ffmpeg/7.1.1_3/include
    LIBS = /opt/homebrew/lib
    CFLAGS = -g -I$(INCLUDE) -I$(FFMPEG_INCLUDE) -DUSE_FFMPEG -c
    LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lavformat -lavcodec -lavutil -lswscale -lswresample -lm
endif

CC = gcc

SRCDIR = ./src
INCDIR = ./include
OBJDIR = ./obj

SRC = $(SRCDIR)/main.c $(SRCDIR)/menu.c $(SRCDIR)/platform.c \
       $(SRCDIR)/player.c $(SRCDIR)/theme.c $(SRCDIR)/camera.c \
       $(SRCDIR)/scaling.c $(SRCDIR)/net.c $(SRCDIR)/video.c

OBJS = $(OBJDIR)/main.o $(OBJDIR)/menu.o $(OBJDIR)/platform.o \
        $(OBJDIR)/player.o  $(OBJDIR)/theme.o $(OBJDIR)/camera.o \
        $(OBJDIR)/scaling.o $(OBJDIR)/net.o $(OBJDIR)/video.o


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

LavaRun: $(OBJS) check-ffmpeg
	$(CC) $(OBJS) -o LavaRun $(LDFLAGS)
	@echo "\033[1;32mCompilation successful. LavaRun is ready to play!\033[0m"

check-ffmpeg:
	@echo "\033[1;34mChecking for FFmpeg installation...\033[0m"
ifeq ($(UNAME_S), Windows)
	@if exist C:\msys64\mingw64\bin\avcodec-*.dll (\
		echo "\033[1;32mFFmpeg detected: Intro video feature is enabled.\033[0m" \
	) else (\
		echo "\033[1;33mWARNING: FFmpeg not detected. The game will run, but the intro video will be skipped.\033[0m" \
		echo "\033[1;33mPlease see FFMPEG_SETUP_WINDOWS.md for installation instructions.\033[0m" \
	)
else
	@if [ -f /opt/homebrew/lib/libavcodec.dylib ] || [ -f /usr/local/lib/libavcodec.dylib ]; then \
		echo "\033[1;32mFFmpeg detected: Intro video feature is enabled.\033[0m"; \
	else \
		echo "\033[1;33mWARNING: FFmpeg not detected. The game will run, but the intro video will be skipped.\033[0m"; \
		echo "\033[1;33mPlease see FFMPEG_SETUP_MAC.md for installation instructions.\033[0m"; \
	fi
endif

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
