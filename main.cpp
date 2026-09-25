#include "Chip8.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int WINDOW_SCALE = 10;
const unsigned int FPS = 60;
const unsigned int CYCLES_PER_FRAME = 10;
const unsigned int ARG_NUM = 2;

int main(int argc, char **argv)
{
    if (argc != ARG_NUM)
    {
        std::cerr << "Usage: " << argv[0] << " <ROM file>\n";
        return -1;
    }
    Chip8 cpu;
    if (!cpu.loadROM(argv[1]))
    {
        return -1; // Exit if ROM loading fails
    }
    sf::RenderWindow window(sf::VideoMode({VIDEO_WIDTH * WINDOW_SCALE, VIDEO_HEIGHT * WINDOW_SCALE}), "CHIP-8 Emulator");
    window.setFramerateLimit(FPS); // Limit to 60 FPS
    sf::RectangleShape pixel(sf::Vector2f({(float)WINDOW_SCALE, (float)WINDOW_SCALE}));
    pixel.setFillColor(sf::Color::White);
    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        // --- KEYBOARD MAPPING ---
        // Maps the SFML keys to the internal CHIP-8 keypad array
        cpu.setKey(0x1, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1));
        cpu.setKey(0x2, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2));
        cpu.setKey(0x3, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3));
        cpu.setKey(0xC, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4));

        cpu.setKey(0x4, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q));
        cpu.setKey(0x5, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W));
        cpu.setKey(0x6, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E));
        cpu.setKey(0xD, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R));

        cpu.setKey(0x7, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A));
        cpu.setKey(0x8, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S));
        cpu.setKey(0x9, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D));
        cpu.setKey(0xE, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F));

        cpu.setKey(0xA, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z));
        cpu.setKey(0x0, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X));
        cpu.setKey(0xB, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C));
        cpu.setKey(0xF, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::V));

        cpu.tickTimers(); // Update delay and sound timers

        for (int i = 0; i < CYCLES_PER_FRAME; i++)
        { // Execute multiple cycles per frame for better performance
            cpu.emulateCycle();
        }
        window.clear(sf::Color::Black);
        const auto &screen = cpu.getScreen();
        for (int i = 0; i < (VIDEO_WIDTH * VIDEO_HEIGHT); i++)
        {
            if (screen[i] == 1)
            {
                pixel.setPosition({(float)(i % VIDEO_WIDTH) * WINDOW_SCALE, (float)(i / VIDEO_WIDTH) * WINDOW_SCALE});
                window.draw(pixel);
            }
        }
        window.display();
    }
    return 0;
}
