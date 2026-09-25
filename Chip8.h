#pragma once
#include <cstdint>
#include <array>

class Chip8
{
public:
    Chip8();
    bool loadROM(const char *filename);
    void emulateCycle();
    uint16_t getPC() const;
    const std::array<uint32_t, 64 * 32> &getScreen() const;
    void setKey(uint8_t key, bool pressed);
    void tickTimers(); // Update delay and sound timers
private:
    static constexpr uint16_t START_ADDRESS = 0x200;
    static constexpr unsigned int MEMORY_SIZE = 4096;
    static constexpr unsigned int NUM_REGISTERS = 16;
    static constexpr unsigned int VIDEO_WIDTH = 64;
    static constexpr unsigned int VIDEO_HEIGHT = 32;
    static constexpr unsigned int NUM_KEYS = 16;
    static constexpr unsigned int FONT_CHAR_SIZE = 5;

    std::array<uint8_t, MEMORY_SIZE> memory;
    uint16_t pc;                          // Program counter
    std::array<uint32_t, VIDEO_WIDTH * VIDEO_HEIGHT> screen; // Screen buffer (64x32 pixels)
    std::array<uint8_t, NUM_REGISTERS> V; // General-purpose registers
    uint16_t I;                           // Index register
    uint8_t sp;                           // Stack pointer
    std::array<uint16_t, NUM_REGISTERS> stack;       // Call stack
    std::array<uint8_t, NUM_KEYS> keypad;
    uint8_t delayTimer;
    uint8_t soundTimer;

    void handle0000(uint16_t opcode, uint8_t NN);
    void handle8000(uint16_t opcode, uint8_t X, uint8_t Y, uint8_t N);
    void handleD000(uint8_t X, uint8_t Y, uint8_t N);
    void handleE000(uint16_t opcode, uint8_t X, uint8_t NN);
    void handleF000(uint16_t opcode, uint8_t X, uint8_t NN);
};
