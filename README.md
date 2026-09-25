# CHIP-8 Emulator

A custom-built CHIP-8 emulator written in modern C++ using SFML 3.

## Dependencies
* C++ Compiler (GCC / MinGW recommended for Windows)
* [SFML 3](https://www.sfml-dev.org/) (Graphics, Window, and System modules)

## How to Build
Assuming you are compiling on Windows using MinGW with your SFML 3 library extracted to `C:\SFML`, open your terminal in the project directory and run the following command:

```bash
g++ main.cpp Chip8.cpp -I"C:\SFML\include" -L"C:\SFML\lib" -lsfml-graphics -lsfml-window -lsfml-system -o chip8.exe

.\chip8.exe [rom_filename].ch8