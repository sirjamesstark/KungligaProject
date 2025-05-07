# macOS Setup Instructions

This document explains the necessary steps to run KungligaProject on macOS.

## FFmpeg Installation

FFmpeg libraries must be installed for the video playback feature to work.

### 1. Homebrew Installation

If you don't have Homebrew installed yet, install it by running this command in Terminal:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Follow the instructions to complete the installation.

### 2. Installing FFmpeg Libraries

Run the following commands in Terminal:

```bash
# Update Homebrew
brew update

# Install FFmpeg
brew install ffmpeg

# Install SDL2 libraries (if not already installed)
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_net
```

## Compiling and Running

1. Open Terminal
2. Navigate to the project directory
3. Run the following commands:

```bash
make clean
make
./KungligaProject server
```

## Troubleshooting

If video playback doesn't work:

1. Make sure FFmpeg is correctly installed by running `ffmpeg -version` in Terminal
2. Check that `video.mov` and `KungligaProjectSound.wav` files exist in the `resources` folder
3. Check the terminal output for error messages

### Common Issues

#### Missing Libraries
If you get errors about missing libraries, try reinstalling the dependencies:

```bash
brew reinstall ffmpeg sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_net
```

#### Permission Issues
If you get permission errors when running the game:

```bash
chmod +x KungligaProject
```

#### Resource Not Found
If resources can't be found, make sure you're running the game from the project root directory, not from inside the `src` or another subdirectory.
