// token.hpp — ДАНО (Lab 8). Вузол однозв'язного списку токенів.
//
// Список, а не масив, бо довжину файлу наперед не знаємо:
// він росте вузол за вузлом, а потім destroy проходить і звільняє все.
#pragma once

#include <string>

struct Token {
    enum class Kind { Ident, Number, Comma, Colon, LBracket, RBracket, Newline, Eof };
    Kind kind;
    std::string text;  // як саме воно було написано в тексті
    int line;          // номер рядка — для повідомлень про помилки
    Token* next;       // nullptr у кінці
};

// Пройтись по списку й зробити delete кожному вузлу.
void destroy(Token* head);

// Ім'я виду токена для друку (моє, для команди `lex`).
const char* kind_name(Token::Kind kind);
