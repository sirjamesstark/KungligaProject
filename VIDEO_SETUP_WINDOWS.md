# KungligaProject Video Setup Guide for Windows

This guide will help you set up the intro video feature for KungligaProject on Windows systems.

## Prerequisites

1. **Install MinGW (GCC for Windows)**:
   - Download MinGW installer from: https://sourceforge.net/projects/mingw/
   - During installation, select at minimum:
     - mingw32-base
     - mingw32-gcc-g++
     - msys-base
   - Add MinGW bin directory to your PATH (typically C:\MinGW\bin)

2. **Install SDL2 Libraries**:
   - Create folder: `C:\SDL2`
   - Download the following development libraries for Windows from https://www.libsdl.org/download-2.0.php:
     - SDL2 (https://www.libsdl.org/release/SDL2-devel-2.0.20-mingw.tar.gz)
     - SDL2_image (https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-mingw.tar.gz)
     - SDL2_mixer (https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.4-mingw.tar.gz)
     - SDL2_ttf (https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-mingw.tar.gz)
     - SDL2_net (https://www.libsdl.org/projects/SDL_net/release/SDL2_net-devel-2.0.1-mingw.tar.gz)
   - Extract all to `C:\SDL2`, ensuring the include and lib folders merge correctly
   - Copy all .dll files from `C:\SDL2\lib\x86` to your KungligaProject directory

3. **Install FFmpeg**:
   - Download FFmpeg for Windows from: https://www.gyan.dev/ffmpeg/builds/
   - Choose "ffmpeg-release-essentials.zip"
   - Extract to `C:\FFmpeg`
   - Add `C:\FFmpeg\bin` to your PATH environment variable:
     - Right-click on "This PC" > Properties > Advanced system settings > Environment Variables
     - Edit the "Path" variable and add `C:\FFmpeg\bin`
   - Create folders:
     - `C:\FFmpeg\include` (for header files)
     - `C:\FFmpeg\lib` (for library files)

4. **FFmpeg Development Files**:
   - Download FFmpeg development files from: https://www.gyan.dev/ffmpeg/builds/packages/
   - Download "ffmpeg-X.X.X-full_build-dev.zip" (where X.X.X is the version)
   - Extract and copy:
     - All .h files from include/ to `C:\FFmpeg\include`
     - All .lib files to `C:\FFmpeg\lib`
   - Copy all .dll files from the FFmpeg bin folder to your KungligaProject directory

## Video File Setup

1. **Prepare Video Files**:
   - Ensure you have a "resources" folder in your KungligaProject directory
   - For Windows, MP4 format works best
   - Convert your video to MP4 format:
     ```
     ffmpeg -i video.mov -c:v h264 -c:a aac resources/video.mp4
     ```
   - Also ensure "KungligaProjectSound.wav" exists in the same folder

2. **Check File Paths**:
   - Make sure the resources folder is in the same directory as the executable
   - Windows paths use backslashes, but the code handles this automatically

## Compiling the Game

1. **Edit Makefile (if needed)**:
   - Open the Makefile and ensure these paths are correct:
     ```
     INCLUDE = C:/SDL2/include
     LIBS = C:/SDL2/lib
     FFMPEG_INCLUDE = C:/FFmpeg/include
     FFMPEG_LIBS = C:/FFmpeg/lib
     FFMPEG_INSTALLED = 1
     ```

2. **Compile the Game**:
   - Open Command Prompt
   - Navigate to your KungligaProject directory
   - Run:
     ```
     mingw32-make clean
     mingw32-make
     ```

3. **Running the Game**:
   - After compilation, run:
     ```
     KungligaProject.exe
     ```
   - You should now see the intro video play before the main menu appears

## Troubleshooting

If the video doesn't play:

1. **Check DLL Files**:
   - Ensure all SDL2 and FFmpeg .dll files are in the same directory as KungligaProject.exe
   - Missing DLLs are the most common cause of problems

2. **Check Video Format**:
   - Windows works best with .mp4 files
   - If your video is in a different format, convert it using FFmpeg:
     ```
     ffmpeg -i video.mov -c:v h264 -c:a aac resources/video.mp4
     ```

3. **Check Console Output**:
   - Look for any error messages when running the game
   - If you see "Could not open video file", check the file path and format

4. **Path Issues**:
   - Windows is sensitive to file paths
   - Make sure your resources folder is in the correct location
   - Try using both forward slashes and backslashes in paths

5. **FFmpeg Installation**:
   - Verify FFmpeg is correctly installed by running `ffmpeg -version` in Command Prompt
   - If not found, check your PATH environment variable

For further assistance, please contact the development team / Group - 4.