// cpu.hpp — ДАНО. Прочитайте кожен рядок, як memory.hpp у першій лабі.
#pragma once
#include <cstdint>
#include "memory.hpp"

// Коди операцій — саме з ISA.uk.md, свого не вигадуйте.
// Lab 6: замість окремих `const Byte OP_*` — перелік. `: std::uint8_t` каже,
// що під кожним значенням лежить рівно один байт, як і в пам'яті ember.
// `enum class` не змішується з цілими без явного приведення: Op::Add не можна
// випадково додати до числа чи порівняти з сирим байтом.
enum class Op : std::uint8_t {
    // 0x0_ — керування й вивід
    Halt = 0x00,
    Nop  = 0x01,
    Out  = 0x02,   // надрукувати A як символ
    OutN = 0x03,   // надрукувати A як десяткове число і пробіл

    // 0x1_ — ALU
    Add = 0x10,
    Sub = 0x11,
    And = 0x12,
    Or  = 0x13,
    Xor = 0x14,
    Not = 0x15,
    Shl = 0x16,
    Shr = 0x17,
    Inc = 0x18,
    Dec = 0x19,
    Cmp = 0x1A,    // A - B лише заради прапорців, результат викидається
    Mul = 0x1B,    // A = A * B, молодші 8 бітів; C — добуток не вліз (Lab 7)

    // 0x2_ — пересилання даних
    LoadiA = 0x20,  // LOADI A, imm8       розмір 2
    LoadiB = 0x21,  // LOADI B, imm8       розмір 2
    LoadA  = 0x22,  // LOAD A, [addr16]    розмір 3
    LoadB  = 0x23,  // LOAD B, [addr16]    розмір 3
    StoreA = 0x24,  // STORE [addr16], A   розмір 3
    StoreB = 0x25,  // STORE [addr16], B   розмір 3

    // 0x3_ — стрибки, усі по 3 байти: опкод + addr16
    Jmp = 0x30,    // завжди
    Jz  = 0x31,    // якщо Z
    Jnz = 0x32,    // якщо не Z

    // 0x4_ — екран, усі по 1 байту
    Cls  = 0x40,   // занулити відеопам'ять
    Plot = 0x41,   // піксель x = A, y = B; за межами — C = 1
    Show = 0x43,   // намалювати екран у терміналі

    // 0x5_ — стек і виклики (Lab 7)
    PushA = 0x50,  // mem[SP] = A; SP = SP - 1
    PopA  = 0x51,  // SP = SP + 1; A = mem[SP]
    PushB = 0x52,
    PopB  = 0x53,
    Call  = 0x54,  // покласти адресу повернення (2 байти), PC = addr16. Розмір 3
    Ret   = 0x55,  // зняти 2 байти в PC
};

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
    std::uint16_t sp = STACK_HI;  // Lab 7: вказівник стека, наступний ВІЛЬНИЙ байт
    Byte a = 0;
    Byte b = 0;
    Flags f;
    bool halted = false;     // після HALT наступні step відмовляють
    bool error = false;      // Lab 3: зупинились не через HALT, а через помилку
                             // (наприклад, адреса за межами пам'яті)
};

void step(CPU& cpu);

// Lab 6: надрукувати sizeof(CPU) і зсув кожного поля (offsetof).
void print_layout();
void dump_regs(const CPU& cpu);
void dump_stack(const CPU& cpu);  // Lab 7: SP і кілька байтів з верхівки

// Крокувати, поки машина не зупиниться (HALT або помилка), але не більше
// MAX_STEPS кроків (Lab 4). Повертає кількість виконаних кроків.
long run(CPU& cpu);
