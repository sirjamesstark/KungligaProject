# KungligaProjekt
Top Secret
TEST Detta är ett spel

## Setup Instructions

This project now includes video playback functionality using FFmpeg. Please follow the setup guide for your operating system:

- [Windows Setup Guide](WINDOWS_SETUP.md)
- [macOS Setup Guide](MACOS_SETUP.md)

### Requirements

- SDL2 libraries (SDL2, SDL2_image, SDL2_mixer, SDL2_ttf, SDL2_net)
- FFmpeg (for video playback)
- GCC compiler

Fixade rörelser genom att ändra Nicklas kod smo styrde åt sidor.
    Tog bort ner knappen och applicerade upCounter och downCounter när man hoppar

Fixade meny där den har bara start och exit knappar där man kan välja menyn antingen mus eller med knappar.

Fixade plattformar med bara lådor genom att använda 2d arrays.

Fixade kollision
	Använde Nicklas kod för änden av skärmen
		Implementerade för lådor
    Hade bytt plats av x,y för matrisen och kunde inte förstå logiken för 4+ timmar...
    Tog bort upCounter och downCounter för att man vlil att spelaren alltid skulle ramla ner med gravitation med eller utan counter annars stannade spelaren vid nästa ruta utan block.
    Spelaren gick inte vänster och höger för att spelaren drogs ner hela tiden även på lådan. Tog bort gravitation när spelaren är på lådan för att fixa det. Gravitation implementeras bara när spelaren är inte på lådan.
    Lade tillbaka upCounter för att gravitation implementerades inte tills spelaren nådde taket.

Tog bort gamerunning variabel från strukten genom att anropa funktionen med int och returna 0 eller 1 beroende på errors.
