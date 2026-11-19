// display.cpp — show() ДАНО; plot() і clear() — моє (Lab 5).
//
// Формула з ISA.uk.md §7:
//   адреса байта = VRAM_LO + y * 8 + (x / 8)
//   номер біта   = 7 - (x % 8)            ; біт 7 — найлівіший піксель у байті
#include "display.hpp"

#include <iostream>

// ДАНО. Намалювати відеопам'ять у терміналі.
void show(const Memory& mem) {
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            Byte cell = mem.data[0xA00 + y * 8 + (x / 8)];
            bool lit  = (cell >> (7 - (x % 8))) & 1;
            std::cout << (lit ? '#' : '.');
        }
        std::cout << '\n';
    }
}

// Та сама формула, що в show, тільки навпаки: не читати біт, а виставити його.
bool plot(Memory& mem, int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) {
        return false;  // за межами екрана — нічого не чіпаємо
    }
    std::size_t addr = VRAM_LO + y * BYTES_PER_ROW + x / 8;  // який байт
    int bit = 7 - x % 8;                                     // який біт у ньому
    mem.data[addr] = (Byte)(mem.data[addr] | (1 << bit));    // OR вмикає один біт
    return true;
}

void clear(Memory& mem) {
    for (std::size_t addr = VRAM_LO; addr <= VRAM_HI; ++addr) {
        mem.data[addr] = 0;
    }
}
