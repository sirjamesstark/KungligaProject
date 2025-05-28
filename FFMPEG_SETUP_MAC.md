# FFmpeg Installation Guide for macOS

This installation is **only required for the intro video playback** feature of the game. The game will function without FFmpeg, but you will not be able to see the intro video. If FFmpeg is not installed, the game will automatically skip the intro video and proceed to the main menu.

## Installation Steps

1. **Install Homebrew** (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. **Install FFmpeg using Homebrew**:
   ```bash
   brew install ffmpeg
   ```

3. **Verify Installation**:
   ```bash
   ffmpeg -version
   ```
   This should display the FFmpeg version information if the installation was successful.

4. **Update the Game's Makefile** (if necessary):
   - Open the Makefile in the game directory
   - Find the FFmpeg include path (around line 10)
   - Update the path if your FFmpeg installation is in a different location:
     ```makefile
     FFMPEG_INCLUDE = /opt/homebrew/Cellar/ffmpeg/[your_version]/include
     ```
   - You can find your FFmpeg installation path with:
     ```bash
     brew --prefix ffmpeg
     ```

## Troubleshooting for macOS

1. **FFmpeg not found in PATH**:
   - Run: `echo 'export PATH="/opt/homebrew/bin:$PATH"' >> ~/.zshrc`
   - Then: `source ~/.zshrc`

2. **Missing dependencies**:
   - Run: `brew install pkg-config sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_net`

3. **Compilation errors**:
   - Make sure you have the Xcode Command Line Tools installed:
     ```bash
     xcode-select --install
     ```

## Additional Resources for macOS

- [FFmpeg Official Documentation](https://ffmpeg.org/documentation.html)
- [Homebrew Documentation](https://docs.brew.sh/)
