#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR "\\"
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define PATH_SEPARATOR "/"
#endif

#include "../include/ffmpeg_checker.h"

// Check if FFmpeg is installed
bool is_ffmpeg_installed() {
    printf("Checking FFmpeg installation...\n");
    
#ifdef _WIN32
    // Windows'ta FFmpeg'in yüklü olup olmadığını kontrol et
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // ffmpeg komutunu çalıştırmayı dene
    if (CreateProcess(NULL, "ffmpeg -version", NULL, NULL, FALSE, 
                     CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        // Sürecin tamamlanmasını bekle
        WaitForSingleObject(pi.hProcess, 1000);
        
        // Çıkış kodunu al
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        // Handle'ları kapat
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        return (exitCode == 0);
    }
    return false;
#else
    // Mac/Linux'ta FFmpeg'in yüklü olup olmadığını kontrol et
    int status = system("which ffmpeg > /dev/null 2>&1");
    return (status == 0);
#endif
}

// Show FFmpeg installation instructions
void show_ffmpeg_installation_instructions() {
    printf("\n===== FFmpeg Installation Instructions =====\n");
    
#ifdef _WIN32
    printf("FFmpeg installation steps for Windows:\n");
    printf("1. Download and install MSYS2: https://www.msys2.org/\n");
    printf("2. Open the MSYS2 MinGW64 terminal\n");
    printf("3. Run this command: pacman -S mingw-w64-x86_64-ffmpeg\n");
    printf("4. Restart the program after installation is complete\n");
#elif defined(__APPLE__)
    printf("FFmpeg installation steps for macOS:\n");
    printf("1. Install Homebrew (if not already installed): /bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"\n");
    printf("2. Run this command: brew install ffmpeg\n");
    printf("3. Restart the program after installation is complete\n");
#else
    printf("FFmpeg installation steps for Linux:\n");
    printf("1. Ubuntu/Debian: sudo apt-get install ffmpeg\n");
    printf("2. Fedora: sudo dnf install ffmpeg\n");
    printf("3. Arch Linux: sudo pacman -S ffmpeg\n");
    printf("4. Restart the program after installation is complete\n");
#endif

    printf("\nNote: Without FFmpeg, the video playback feature will not work.\n");
    printf("Press any key to continue...\n");
    getchar();
}
