# FFmpeg Installation Guide for Windows

This installation is **only required for the intro video playback** feature of the game. The game will function without FFmpeg, but you will not be able to see the intro video. If FFmpeg is not installed, the game will automatically skip the intro video and proceed to the main menu.

## Installation Steps

1. **Install MSYS2** (if not already installed):
   - Download the installer from [https://www.msys2.org/](https://www.msys2.org/)
   - Run the installer and follow the prompts
   - After installation, open the MSYS2 terminal

2. **Update the package database**:
   ```bash
   pacman -Syu
   ```
   (You may need to close and reopen the terminal after this step)

3. **Install FFmpeg and development packages**:
   ```bash
   pacman -S mingw-w64-x86_64-ffmpeg
   ```

4. **Add FFmpeg to your PATH**:
   - Right-click on "This PC" or "My Computer" and select "Properties"
   - Click on "Advanced system settings"
   - Click on "Environment Variables"
   - Under "System variables", find the "Path" variable, select it and click "Edit"
   - Click "New" and add the path to FFmpeg binaries (typically `C:\msys64\mingw64\bin`)
   - Click "OK" to close all dialogs

5. **Verify Installation**:
   - Open Command Prompt
   - Run:
     ```
     ffmpeg -version
     ```
   - This should display the FFmpeg version information

6. **Update the Game's Makefile** (if necessary):
   - Open the Makefile in the game directory
   - Find the FFmpeg include path (around line 4)
   - Update the path if your FFmpeg installation is in a different location:
     ```makefile
     INCLUDE = C:/msys64/mingw64/include/SDL2
     ```

## Troubleshooting for Windows

1. **DLL not found errors**:
   - Make sure the FFmpeg bin directory is in your PATH
   - You may need to copy the DLLs from `C:\msys64\mingw64\bin` to your game directory

2. **Compilation errors**:
   - Make sure you've installed the development packages with MSYS2
   - Check that the include and library paths in the Makefile match your installation

3. **SDL2 related errors**:
   - Install SDL2 and related libraries:
     ```bash
     pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_net
     ```

## Additional Resources for Windows

- [FFmpeg Official Documentation](https://ffmpeg.org/documentation.html)
- [MSYS2 Documentation](https://www.msys2.org/docs/)
