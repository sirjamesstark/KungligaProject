#ifndef FFMPEG_CHECKER_H
#define FFMPEG_CHECKER_H

#include <stdbool.h>

// FFmpeg kurulu olup olmadığını kontrol eder
bool is_ffmpeg_installed();

// FFmpeg kurulumu için talimatları göster
void show_ffmpeg_installation_instructions();

#endif // FFMPEG_CHECKER_H
