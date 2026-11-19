// alu.hpp — арифметико-логічний пристрій (Lab 2).
//
// Кожна функція отримує байти, повертає результат (загорнутий на 8 бітах)
// і виставляє прапорці Z/N/C у `f` так, як написано в ISA.uk.md §5, група 0x1_.
#pragma once

#include "cpu.hpp"

Byte alu_add(Byte a, Byte b, Flags& f);  // Z N C   (C = перенесення з 7-го біта)
Byte alu_sub(Byte a, Byte b, Flags& f);  // Z N C   (C = позика: a < b)
Byte alu_and(Byte a, Byte b, Flags& f);  // Z N, C=0
Byte alu_or(Byte a, Byte b, Flags& f);   // Z N, C=0
Byte alu_xor(Byte a, Byte b, Flags& f);  // Z N, C=0
Byte alu_not(Byte a, Flags& f);          // Z N, C=0
Byte alu_shl(Byte a, Flags& f);          // Z N C   (C = біт, що виїхав із 7-ї позиції)
Byte alu_shr(Byte a, Flags& f);          // Z N C   (C = біт, що виїхав із 0-ї позиції)
Byte alu_inc(Byte a, Flags& f);          // Z N     (C не чіпає)
Byte alu_dec(Byte a, Flags& f);          // Z N     (C не чіпає)
