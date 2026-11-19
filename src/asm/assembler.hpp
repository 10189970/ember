// assembler.hpp — асемблер ember, рівень Basic (Lab 8).
//
// Один прохід, БЕЗ міток: кожен рядок — одна інструкція, мнемоніка стає
// опкодом, число — байтом (imm8) або двома байтами молодшим уперед (addr16).
// Адреси для JMP/CALL пишуться числами.
#pragma once

#include <string>

#include "../memory.hpp"
#include "token.hpp"

// Зібрати список токенів у байти, починаючи з адреси 0x000 (регіон коду).
// Успіх: true, у `size` — скільки байтів записано.
// Помилка: false, у `error` — "line N: що не так"; пам'ять могла змінитись частково.
bool assemble(const Token* head, Memory& mem, std::size_t& size, std::string& error);
