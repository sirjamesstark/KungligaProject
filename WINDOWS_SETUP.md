# Windows Setup Instructions

This document explains the necessary steps to run KungligaProject on Windows.

## FFmpeg Installation

FFmpeg libraries must be installed for the video playback feature to work.

### 1. MSYS2 Installation

1. Download the installer from the [MSYS2 download page](https://www.msys2.org/)
2. Complete the MSYS2 installation
3. Open the MSYS2 MinGW64 terminal

### 2. Installing FFmpeg Libraries

Run the following commands in the MSYS2 MinGW64 terminal:

```bash
# Update package database
pacman -Syu

# Install FFmpeg development libraries
pacman -S mingw-w64-x86_64-ffmpeg

# Install SDL2 libraries (if not already installed)
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_net
```

### 3. Modifying the Makefile

Open the `Makefile` and make the following change in the Windows section:

```makefile
# Find line 11 and change it to:
FFMPEG_INSTALLED = 1
```

## Compiling and Running

1. Open the MSYS2 MinGW64 terminal
2. Navigate to the project directory
3. Run the following commands:

```bash
make clean
make
./KungligaProject server
```

## Troubleshooting

If video playback still doesn't work:

1. Make sure the FFmpeg libraries are correctly installed
2. Check that `video.mov` and `KungligaProjectSound.wav` files exist in the `resources` folder
3. Check the terminal output for error messages

## Alternative: Audio Only

If you don't want to deal with FFmpeg installation, you can compile the project to play only audio:

```bash
make clean
make
```

In this case, only the audio will play, and the game will transition to the main menu after a short delay.
