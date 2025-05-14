# Identifierar operativsystem: Windows eller macOS
# INCLUDE anger fullständig sökväg till SDL2:s headerfiler
# LIB anger fullständig sökväg till SDL2:s bibliotek
# CFLAGS & LDFLAGS säts för att bygga och länka SDL2 bibliotek till operativsystemet
ifeq ($(OS), Windows_NT)
    UNAME_S = Windows
    # SDL2 and FFmpeg paths for Windows
    INCLUDE = C:/SDL2/include
    LIBS = C:/SDL2/lib
    
    # FFmpeg control - Set to 1 if FFmpeg is installed on Windows, 0 otherwise
    FFMPEG_INSTALLED = 1
    
    ifeq ($(FFMPEG_INSTALLED), 1)
        CFLAGS = -g -I$(INCLUDE) -DUSE_FFMPEG -DDEBUG -c
        LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lavformat -lavcodec -lavutil -lswscale -lswresample -lm
        $(info FFmpeg enabled for Windows. Using video playback.)
    else
        CFLAGS = -g -I$(INCLUDE) -DDEBUG -c
        LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm
        $(info FFmpeg disabled for Windows. Using audio only.)
    endif
else
    # macOS eller Linux
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)
        # macOS
        INCLUDE = /opt/homebrew/include/SDL2
        LIBS = /opt/homebrew/lib
        
        # Check if FFmpeg is available
        FFMPEG_AVAILABLE := $(shell brew list ffmpeg >/dev/null 2>&1 && echo 1 || echo 0)
        
        ifeq ($(FFMPEG_AVAILABLE), 1)
            # FFmpeg is available
            CFLAGS = -g -I$(INCLUDE) -I/opt/homebrew/include -DUSE_FFMPEG -DDEBUG -c
            LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lavformat -lavcodec -lavutil -lswscale -lswresample -lm
            $(info FFmpeg detected on macOS. Using video playback.)
        else
            # FFmpeg is not available
            CFLAGS = -g -I$(INCLUDE) -DDEBUG -c
            LDFLAGS = -L$(LIBS) -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_net -lm
            $(info FFmpeg not detected on macOS. Using audio only.)  
        endif
    endif
endif

# Kompilator GCC
ifeq ($(OS), Windows_NT)
    CC = gcc
    # Windows'ta make komutu farklı olabilir
    MAKE_COMMAND = mingw64-make
    # Windows için resource compiler
    RC = windres
else
    CC = gcc
    MAKE_COMMAND = make
endif

# Sökvägar relativt Makefilen
SRCDIR = ./src
INCDIR = ./include
OBJDIR = ./obj

# Lista över filer
SRC = $(SRCDIR)/main.c $(SRCDIR)/menu.c $(SRCDIR)/platform.c \
       $(SRCDIR)/player.c $(SRCDIR)/theme.c $(SRCDIR)/camera.c \
       $(SRCDIR)/video_player.c $(SRCDIR)/scaling.c

OBJS = $(OBJDIR)/main.o $(OBJDIR)/menu.o $(OBJDIR)/platform.o \
        $(OBJDIR)/player.o  $(OBJDIR)/theme.o $(OBJDIR)/camera.o \
        $(OBJDIR)/video_player.o $(OBJDIR)/scaling.o

HEADERS =  $(INCDIR)/menu.h $(INCDIR)/platform.h $(INCDIR)/player.h \
            $(INCDIR)/theme.h $(INCDIR)/camera.h \
            $(INCDIR)/video_player.h $(INCDIR)/scaling.h

ifeq ($(UNAME_S), Windows)
    MKDIR = if not exist $(subst /,\,$(OBJDIR)) mkdir $(subst /,\,$(OBJDIR))
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    MKDIR = mkdir -p $(OBJDIR)
    RM = rm -f
    RMDIR = rm -rf
endif

# Windows için resource dosyasını derle
ifeq ($(OS), Windows_NT)
$(OBJDIR)/KungligaProject.res: resources/KungligaProject.rc | $(OBJDIR)
	$(RC) -i $< -o $@
endif

# Skapar en exekverbar fil utifrån .o-filerna
ifeq ($(OS), Windows_NT)
KungligaProject: $(OBJS) $(OBJDIR)/KungligaProject.res
	$(CC) $(OBJS) $(OBJDIR)/KungligaProject.res -o KungligaProject $(LDFLAGS)
else
KungligaProject: $(OBJS)
	$(CC) $(OBJS) -o KungligaProject $(LDFLAGS)
endif

# Kompilerar källfiler till objektfiler
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $<

# Skapar obj-mappen om den inte finns
$(OBJDIR):
	@echo Running: $(MKDIR)
	@$(MKDIR)

# macOS için .app paketi oluşturma
ifeq ($(UNAME_S), Darwin)
app: KungligaProject
	@echo "Creating macOS application bundle"
	@mkdir -p KungligaProject.app/Contents/MacOS
	@mkdir -p KungligaProject.app/Contents/Resources
	@cp KungligaProject KungligaProject.app/Contents/MacOS/
	@cp -R resources/* KungligaProject.app/Contents/Resources/
	@echo "Application bundle created: KungligaProject.app"
	@echo "Note: Place KungligaProject.icns in KungligaProject.app/Contents/Resources/ for custom icon"
endif

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
	$(RM) -rf KungligaProject.app
endif
