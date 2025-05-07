# Identifierar operativsystem: Windows eller macOS
# INCLUDE anger fullständig sökväg till SDL2:s headerfiler
# LIB anger fullständig sökväg till SDL2:s bibliotek
# CFLAGS & LDFLAGS säts för att bygga och länka SDL2 bibliotek till operativsystemet
ifeq ($(OS), Windows_NT)
    UNAME_S = Windows
    INCLUDE = C:/msys64/mingw64/include/SDL2
    LIBS = C:/msys64/mingw64/lib
    
    # ===== WINDOWS FFMPEG KURULUMU =====
    # FFmpeg'i Windows'ta kullanmak için:
    # 1. MSYS2 kurun: https://www.msys2.org/
    # 2. MSYS2 MinGW64 terminalinde şu komutu çalıştırın: pacman -S mingw-w64-x86_64-ffmpeg
    # 3. Aşağıdaki değeri 1 olarak değiştirin
    # ===================================
    FFMPEG_INSTALLED = 0
    
    ifeq ($(FFMPEG_INSTALLED), 1)
        # FFmpeg is available
        CFLAGS = -g -I$(INCLUDE) -IC:/msys64/mingw64/include -Dmain=SDL_main -DUSE_FFMPEG -DDEBUG -c
        LDFLAGS = -L$(LIBS) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lavformat -lavcodec -lavutil -lswscale -lswresample -lm
    else
        # FFmpeg is not available
        CFLAGS = -g -I$(INCLUDE) -Dmain=SDL_main -DDEBUG -c
        LDFLAGS = -L$(LIBS) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm
        $(info FFmpeg not detected on Windows. Using fallback mode without video playback.)  
    endif
else
    UNAME_S := $(shell uname -s)
    INCLUDE = /opt/homebrew/include/SDL2
    LIBS = /opt/homebrew/lib
    FFMPEG_INCLUDE = /opt/homebrew/include
    # Check if FFmpeg is available
    FFMPEG_EXISTS := $(shell [ -f $(FFMPEG_INCLUDE)/libavcodec/avcodec.h ] && echo 1 || echo 0)
    
    ifeq ($(FFMPEG_EXISTS), 1)
        # FFmpeg is available
        CFLAGS = -g -I$(INCLUDE) -I$(FFMPEG_INCLUDE) -DUSE_FFMPEG -DDEBUG -c
        LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lavformat -lavcodec -lavutil -lswscale -lswresample -lm
        $(info FFmpeg detected on macOS. Using full video playback.)  
    else
        # FFmpeg is not available
        CFLAGS = -g -I$(INCLUDE) -DDEBUG -c
        LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm
        $(info FFmpeg not detected on macOS. Using fallback mode without video playback.)  
    endif
endif

# Kompilator GCC
CC = gcc

# Sökvägar relativt Makefilen
SRCDIR = ./src
INCDIR = ./include
OBJDIR = ./obj

# Lista över filer
SRC = $(SRCDIR)/main.c $(SRCDIR)/menu.c $(SRCDIR)/platform.c \
       $(SRCDIR)/player.c $(SRCDIR)/theme.c $(SRCDIR)/camera.c \
       $(SRCDIR)/common.c $(SRCDIR)/video_player.c $(SRCDIR)/ffmpeg_checker.c

OBJS = $(OBJDIR)/main.o $(OBJDIR)/menu.o $(OBJDIR)/platform.o \
        $(OBJDIR)/player.o  $(OBJDIR)/theme.o $(OBJDIR)/camera.o \
        $(OBJDIR)/common.o $(OBJDIR)/video_player.o $(OBJDIR)/ffmpeg_checker.o


HEADERS =  $(INCDIR)/menu.h $(INCDIR)/platform.h $(INCDIR)/player.h \
            $(INCDIR)/theme.h $(INCDIR)/camera.h $(INCDIR)/common.h \
            $(INCDIR)/video_player.h $(INCDIR)/ffmpeg_checker.h


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
