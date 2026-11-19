// lexer.hpp — ДАНО (Lab 8).
#pragma once

#include <string>

#include "token.hpp"

// Текст -> список токенів. При помилці повертає nullptr, а в `error`
// кладе "line N: що не так". Список звільняє той, хто викликав: destroy(head).
Token* lex(const std::string& src, std::string& error);
