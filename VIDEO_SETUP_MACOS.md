# KungligaProject Video Setup Guide for macOS

This guide will help you set up the intro video feature for KungligaProject on macOS systems.

## Prerequisites

1. **Homebrew Installation**:
   - Open Terminal (Finder > Applications > Utilities > Terminal)
   - Paste the following command and press Enter:
     ```
     /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
     ```
   - Wait for the installation to complete

2. **Install SDL2 Libraries**:
   - Run the following command in Terminal:
     ```
     brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_net
     ```

3. **Install FFmpeg**:
   - Run the following command in Terminal:
     ```
     brew install ffmpeg
     ```
   - This is essential for video playback functionality

## Video File Setup

1. **Check Video Files**:
   - Ensure you have a "resources" folder in your KungligaProject directory
   - Make sure "video.mov" file exists in this folder
   - Also ensure "KungligaProjectSound.wav" exists in the same folder

2. **File Permissions**:
   - Set proper permissions by running:
     ```
     chmod 755 resources/video.mov
     chmod 755 resources/KungligaProjectSound.wav
     ```

## Compiling the Game

1. **Compile with FFmpeg Support**:
   - Open Terminal and navigate to your KungligaProject directory
   - Run:
     ```
     make clean
     make
     ```
   - The Makefile will automatically detect FFmpeg and enable video support

2. **Running the Game**:
   - After compilation, run:
     ```
     ./KungligaProject
     ```
   - You should now see the intro video play before the main menu appears

## Troubleshooting

If the video doesn't play:

1. **Check FFmpeg Installation**:
   - Run `brew info ffmpeg` to verify it's installed correctly
   - If not, try reinstalling with `brew reinstall ffmpeg`

2. **Check Video Format**:
   - macOS works best with .mov files
   - If your video is in a different format, convert it to .mov using:
     ```
     ffmpeg -i yourvideo.mp4 -c:v prores -c:a pcm_s16le resources/video.mov
     ```

3. **Check Console Output**:
   - Look for any error messages when running the game
   - If you see "FFmpeg not detected", ensure FFmpeg is properly installed

4. **Alternative Video Format**:
   - If .mov doesn't work, try using .mp4 format:
     ```
     ffmpeg -i yourvideo.mov -c:v h264 -c:a aac resources/video.mp4
     ```
   - The game will automatically try both formats

For further assistance, please contact the development team / Group - 4.