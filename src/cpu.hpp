// cpu.hpp — ДАНО. Прочитайте кожен рядок, як memory.hpp у першій лабі.
#pragma once
#include <cstdint>
#include "memory.hpp"

// Коди операцій — саме з ISA.uk.md, свого не вигадуйте.
// Поки це звичайні константи (Lab 1, `const`). У Lab 6 вони стануть `enum class Op`.
const Byte OP_HALT = 0x00;
const Byte OP_NOP  = 0x01;
const Byte OP_OUT  = 0x02;   // Lab 3: надрукувати A як символ
const Byte OP_OUTN = 0x03;   // Lab 3: надрукувати A як десяткове число і пробіл
const Byte OP_ADD  = 0x10;
const Byte OP_SUB  = 0x11;
const Byte OP_AND  = 0x12;
const Byte OP_OR   = 0x13;
const Byte OP_XOR  = 0x14;
const Byte OP_NOT  = 0x15;
const Byte OP_SHL  = 0x16;
const Byte OP_SHR  = 0x17;
const Byte OP_INC  = 0x18;
const Byte OP_DEC  = 0x19;
const Byte OP_CMP  = 0x1A;   // Lab 4: A - B лише заради прапорців, результат викидається

// Група 0x2_ — пересилання даних (Lab 3, рівень Basic).
const Byte OP_LOADI_A  = 0x20;  // LOADI A, imm8       розмір 2
const Byte OP_LOADI_B  = 0x21;  // LOADI B, imm8       розмір 2
const Byte OP_LOAD_A   = 0x22;  // LOAD A, [addr16]    розмір 3
const Byte OP_LOAD_B   = 0x23;  // LOAD B, [addr16]    розмір 3
const Byte OP_STORE_A  = 0x24;  // STORE [addr16], A   розмір 3
const Byte OP_STORE_B  = 0x25;  // STORE [addr16], B   розмір 3

// Група 0x3_ — стрибки (Lab 4, рівень Basic). Усі по 3 байти: опкод + addr16.
const Byte OP_JMP = 0x30;  // завжди
const Byte OP_JZ  = 0x31;  // якщо Z
const Byte OP_JNZ = 0x32;  // якщо не Z

// Група 0x4_ — екран (Lab 5, рівень Basic). Усі по 1 байту.
const Byte OP_CLS  = 0x40;  // занулити відеопам'ять
const Byte OP_PLOT = 0x41;  // піксель x = A, y = B; за межами — C = 1
const Byte OP_SHOW = 0x43;  // намалювати екран у терміналі

// Ліміт кроків для `run`: зламаний JMP (наприклад, сам на себе) не має
// повісити процес. 100000 — з запасом для будь-якої програми цього семестру.
const long MAX_STEPS = 100000;

struct Flags {
    bool z = false;   // результат нульовий
    bool n = false;   // старший біт результату — одиниця
    bool c = false;   // біт, який виїхав за межу восьми
};

struct CPU {
    Memory* mem = nullptr;   // де лежить коробка. Що таке `*`, пояснює Lab 3;
                             // поки читайте як «процесор знає, де його пам'ять»
    std::uint16_t pc = 0;    // адреса НАСТУПНОЇ інструкції
    Byte a = 0;
    Byte b = 0;
    Flags f;
    bool halted = false;     // після HALT наступні step відмовляють
    bool error = false;      // Lab 3: зупинились не через HALT, а через помилку
                             // (наприклад, адреса за межами пам'яті)
};

void step(CPU& cpu);
void dump_regs(const CPU& cpu);

// Крокувати, поки машина не зупиниться (HALT або помилка), але не більше
// MAX_STEPS кроків (Lab 4). Повертає кількість виконаних кроків.
long run(CPU& cpu);
