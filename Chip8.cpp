#include "Chip8.h"
#include <fstream>
#include <iostream>
#include <iomanip> // For formatting hex output

const unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
{
    pc = START_ADDRESS; // Program counter starts at 0x200
    I = 0;              // Index register
    sp = 0;
    for (unsigned int i = 0; i < sizeof(fontset); i++)
    {
        memory[i] = fontset[i]; // Load fontset into memory
    }
    screen.fill(0); // Clear screen buffer
    V.fill(0);      // Clear registers
    stack.fill(0);  // Clear stack
    keypad.fill(0); // Clear keypad state
    delayTimer = 0;
    soundTimer = 0;
}

const std::array<uint32_t, 64 * 32> &Chip8::getScreen() const
{
    return screen;
}

void Chip8::setKey(uint8_t key, bool pressed)
{
    keypad[key] = pressed;
}

void Chip8::tickTimers()
{
    if (delayTimer > 0)
    {
        --delayTimer;
    }
    if (soundTimer > 0)
    {
        --soundTimer;
    }
}

bool Chip8::loadROM(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return false;
    }
    std::streamsize size = file.tellg();
    if (size > static_cast<std::streamsize>(MEMORY_SIZE - START_ADDRESS))
    {
        std::cerr << "Error: ROM file is too large to fit in memory.\n";
        return false;
    }
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(memory.data() + START_ADDRESS), size);
    file.close();
    std::cout << "Successfully loaded ROM: " << size << " bytes.\n";
    return true;
}

void Chip8::emulateCycle()
{
    if (pc >= MEMORY_SIZE - 1)
    {
        std::cerr << "Fatal Error: PC out of bounds!\n";
        return;
    }
    // Fetch opcode
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

    // Increment program counter
    pc += 2;

    // Extract variables
    uint16_t NNN = opcode & 0x0FFF;
    uint8_t NN = opcode & 0x00FF;
    uint8_t N = opcode & 0x000F;
    uint8_t X = (opcode & 0x0F00) >> 8;
    uint8_t Y = (opcode & 0x00F0) >> 4;

    // Decode and execute opcode
    switch (opcode & 0xF000)
    {
    case 0x0000:
        handle0000(opcode, NN);
        break;
    case 0x1000:
        pc = NNN;
        break;
    case 0x2000:
        stack[sp] = pc;
        sp++;
        pc = NNN;
        break;
    case 0x3000:
        if (V[X] == NN)
        {
            pc += 2;
        }
        break;
    case 0x4000:
        if (V[X] != NN)
        {
            pc += 2;
        }
        break;
    case 0x5000:
        if (V[X] == V[Y])
        {
            pc += 2;
        }
        break;
    case 0x6000:
        V[X] = NN;
        break;
    case 0x7000:
        V[X] += NN;
        break;
    case 0x8000:
        handle8000(opcode, X, Y, N);
        break;
    case 0x9000:
        if (V[X] != V[Y])
        {
            pc += 2;
        }
        break;
    case 0xA000:
        I = NNN;
        break;
    case 0xB000:
        pc = NNN + V[0];
        break;
    case 0xC000:
        V[X] = (rand() % 256) & NN;
        break;
    case 0xD000:
        handleD000(X, Y, N);
        break;
    case 0xE000:
        handleE000(opcode, X, NN);
        break;
    case 0xF000:
        handleF000(opcode, X, NN);
        break;
    default:
        std::cout << "Unknown opcode: 0x"
                  << std::setfill('0') << std::setw(4) << std::hex << opcode << "\n";
        break;
    }
}

void Chip8::handle0000(uint16_t opcode, uint8_t NN)
{
    switch (NN)
    {
    case 0xE0:
        screen.fill(0); // Clear the display
        break;
    case 0xEE:
        if (sp == 0)
        {
            std::cerr << "Stack underflow error!\n";
            return;
        }
        sp--;
        pc = stack[sp]; // Return to the address at the top of the stack
        break;
    default:
        std::cout << "Unknown opcode: 0x"
                  << std::setfill('0') << std::setw(4) << std::hex << opcode << "\n";
        break;
    }
}

void Chip8::handle8000(uint16_t opcode, uint8_t X, uint8_t Y, uint8_t N)
{
    switch (N)
    {
    case 0x0:
        V[X] = V[Y];
        break;
    case 0x1:
        V[X] |= V[Y];
        break;
    case 0x2:
        V[X] &= V[Y];
        break;
    case 0x3:
        V[X] ^= V[Y];
        break;
    case 0x4:
    {
        uint16_t sum = V[X] + V[Y];
        V[0xF] = (sum > 255) ? 1 : 0; // Set carry flag
        V[X] = sum & 0xFF;            // Keep only the lower 8 bits
        break;
    }
    case 0x5:
        V[0xF] = (V[X] > V[Y]) ? 1 : 0; // Set borrow flag
        V[X] -= V[Y];
        break;
    case 0x6:
        V[0xF] = V[X] & 0x1; // Store LSB in VF
        V[X] >>= 1;          // Shift right
        break;
    case 0x7:
        V[0xF] = (V[Y] > V[X]) ? 1 : 0; // Set borrow flag
        V[X] = V[Y] - V[X];
        break;
    case 0xE:
        V[0xF] = (V[X] & 0x80) >> 7; // Store MSB in VF
        V[X] <<= 1;                  // Shift left
        break;
    default:
        std::cout << "Unknown opcode: 0x"
                  << std::setfill('0') << std::setw(4) << std::hex << opcode << "\n";
        break;
    }
}

void Chip8::handleD000(uint8_t X, uint8_t Y, uint8_t N)
{
    uint8_t xPos = V[X] % VIDEO_WIDTH;
    uint8_t yPos = V[Y] % VIDEO_HEIGHT;
    V[0xF] = 0; // Reset collision flag
    for (unsigned int row = 0; row < N; row++)
    {
        uint8_t spriteByte = memory[I + row];
        for (unsigned int col = 0; col < 8; col++)
        {
            uint8_t spritePixel = spriteByte & (0x80 >> col);
            if (spritePixel != 0)
            {
                uint32_t screenIndex = ((yPos + row) % VIDEO_HEIGHT) * VIDEO_WIDTH + ((xPos + col) % VIDEO_WIDTH);
                if (screen[screenIndex] == 1)
                {
                    V[0xF] = 1; // Collision detected
                }
                screen[screenIndex] ^= 1; // XOR the pixel
            }
        }
    }
}

void Chip8::handleE000(uint16_t opcode, uint8_t X, uint8_t NN)
{
    switch (NN)
    {
    case 0x9E:
        if (keypad[V[X]] != 0)
        {
            pc += 2; // Skip next instruction if key in VX is pressed
        }
        break;
    case 0xA1:
        if (keypad[V[X]] == 0)
        {
            pc += 2; // Skip next instruction if key in VX is not pressed
        }
        break;
    default:
        std::cout << "Unknown opcode: 0x"
                  << std::setfill('0') << std::setw(4) << std::hex << opcode << "\n";
    }
}

void Chip8::handleF000(uint16_t opcode, uint8_t X, uint8_t NN)
{
    switch (NN)
    {
    case 0x07:
        V[X] = delayTimer; // Set VX to the value of the delay timer
        break;
    case 0x0A:
    {
        bool keyPressed = false;
        for (uint8_t i = 0; i < NUM_KEYS; i++)
        {
            if (keypad[i] != 0)
            {
                V[X] = i; // Store the key in VX
                keyPressed = true;
                break;
            }
        }
        if (!keyPressed)
        {
            pc -= 2; // Repeat this instruction until a key is pressed
        }
        break;
    }
    case 0x15:
        delayTimer = V[X]; // Set the delay timer to VX
        break;
    case 0x18:
        soundTimer = V[X]; // Set the sound timer to VX
        break;
    case 0x1E:
        I += V[X]; // Add VX to I
        break;
    case 0x29:
        I = V[X] * FONT_CHAR_SIZE; // Set I to the location of the sprite for the character in VX
        break;
    case 0x33:
        memory[I] = V[X] / 100;           // Hundreds digit
        memory[I + 1] = (V[X] / 10) % 10; // Tens digit
        memory[I + 2] = V[X] % 10;        // Ones digit
        break;
    case 0x55:
        for (uint8_t i = 0; i <= X; i++)
        {
            memory[I + i] = V[i]; // Store registers V0 through VX in memory starting at I
        }
        break;
    case 0x65:
        for (uint8_t i = 0; i <= X; i++)
        {
            V[i] = memory[I + i]; // Read registers V0 through VX from memory starting at I
        }
        break;
    default:
        std::cout << "Unknown opcode: 0x"
                  << std::setfill('0') << std::setw(4) << std::hex << opcode << "\n";
    }
}

uint16_t Chip8::getPC() const
{
    return pc;
}
