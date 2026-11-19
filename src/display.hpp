// display.hpp — екран 64x32, по біту на піксель (Lab 5).
//
// Екран живе ВСЕРЕДИНІ пам'яті ember: 256 байтів з VRAM_LO = 0xA00.
// Окремого масиву пікселів на хості немає — є mem.data[0xA00 .. 0xAFF].
#pragma once

#include "memory.hpp"

const int SCREEN_W = 64;
const int SCREEN_H = 32;
const int BYTES_PER_ROW = SCREEN_W / 8;  // 8 байтів на рядок

// ДАНО. Намалювати відеопам'ять у терміналі.
void show(const Memory& mem);

// Засвітити піксель (x, y). Повертає false і нічого не міняє, якщо
// x >= 64 або y >= 32 (чи від'ємні) — з цього PLOT виставляє прапорець C.
bool plot(Memory& mem, int x, int y);

// Занулити всі 256 байтів відеопам'яті.
void clear(Memory& mem);
