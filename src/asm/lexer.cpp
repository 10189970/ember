// lexer.cpp — ДАНО, крім number(). Прочитайте цикл у lex() згори донизу:
// це псевдокод з теорії §1, записаний на C++.
#include "lexer.hpp"

#include <cctype>

// Стан лексера: текст, курсор, номер рядка і список, який ми будуємо.
struct Lexer {
    const std::string& src;
    std::size_t i = 0;
    int line = 1;
    Token* head = nullptr;
    Token* tail = nullptr;
    std::string error;  // непорожній, щойно лексер щось відхилив
};

// Додати вузол у кінець списку (Notes 08 §3).
static void emit(Lexer& lx, Token::Kind kind, const std::string& text) {
    Token* t = new Token{kind, text, lx.line, nullptr};
    if (!lx.head) {
        lx.head = lx.tail = t;
    } else {
        lx.tail->next = t;
        lx.tail = t;
    }
}

// Записати помилку з номером рядка. Повертає false, щоб зручно писати
// `return fail(lx, "...");`
static bool fail(Lexer& lx, const std::string& msg) {
    lx.error = "line " + std::to_string(lx.line) + ": " + msg;
    return false;
}

void destroy(Token* head) {
    while (head) {
        Token* next = head->next;
        delete head;
        head = next;
    }
}

const char* kind_name(Token::Kind kind) {
    switch (kind) {
    case Token::Kind::Ident:    return "Ident";
    case Token::Kind::Number:   return "Number";
    case Token::Kind::Comma:    return "Comma";
    case Token::Kind::Colon:    return "Colon";
    case Token::Kind::LBracket: return "LBracket";
    case Token::Kind::RBracket: return "RBracket";
    case Token::Kind::Newline:  return "Newline";
    case Token::Kind::Eof:      return "Eof";
    }
    return "?";
}

static bool is_hex_digit(char c) { return std::isxdigit((unsigned char)c) != 0; }
static bool is_bin_digit(char c) { return c == '0' || c == '1'; }
static bool is_dec_digit(char c) { return std::isdigit((unsigned char)c) != 0; }

// МОЄ (Lab 8, M1). Курсор lx.i стоїть на цифрі.
// Три форми: 0x41 (шістнадцяткове), 0b101 (двійкове), 65 (десяткове).
// Зіпсоване число — помилка з номером рядка, а не тихий нуль:
//   0xGG — після 0x немає жодної шістнадцяткової цифри
//   0x   — те саме
//   12ab — одразу після цифр іде буква: це не число і не ідентифікатор
// Значення тут не рахуємо — лише перевіряємо форму. Рахує асемблер.
static bool number(Lexer& lx) {
    const std::string& s = lx.src;
    std::size_t start = lx.i;

    // Подивитись на символ після нуля, не вилізши за кінець рядка.
    char second = (lx.i + 1 < s.size()) ? s[lx.i + 1] : '\0';

    if (s[lx.i] == '0' && (second == 'x' || second == 'X')) {
        lx.i += 2;  // з'їли "0x"
        std::size_t digits = lx.i;
        while (lx.i < s.size() && is_hex_digit(s[lx.i])) ++lx.i;
        if (lx.i == digits) return fail(lx, "bad number: no hex digits after 0x");
    } else if (s[lx.i] == '0' && (second == 'b' || second == 'B')) {
        lx.i += 2;  // з'їли "0b"
        std::size_t digits = lx.i;
        while (lx.i < s.size() && is_bin_digit(s[lx.i])) ++lx.i;
        if (lx.i == digits) return fail(lx, "bad number: no binary digits after 0b");
    } else {
        while (lx.i < s.size() && is_dec_digit(s[lx.i])) ++lx.i;
    }

    // Число має закінчитись. Якщо одразу за ним буква, цифра чи '_' —
    // це щось на кшталт 12ab, 0x1G або 0b102: відхиляємо.
    if (lx.i < s.size() && (std::isalnum((unsigned char)s[lx.i]) || s[lx.i] == '_')) {
        // показати в повідомленні все «слово», а не лише його початок
        std::size_t end = lx.i;
        while (end < s.size() && (std::isalnum((unsigned char)s[end]) || s[end] == '_')) ++end;
        return fail(lx, "bad number '" + s.substr(start, end - start) + "'");
    }

    emit(lx, Token::Kind::Number, s.substr(start, lx.i - start));
    return true;
}

Token* lex(const std::string& src, std::string& error) {
    Lexer lx{src, 0, 1, nullptr, nullptr, ""};
    while (lx.i < src.size()) {
        char c = src[lx.i];
        if (c == ' ' || c == '\t' || c == '\r') {
            ++lx.i;                                   // пробіли не токени
        } else if (c == ';') {
            while (lx.i < src.size() && src[lx.i] != '\n') ++lx.i;  // коментар
        } else if (c == '\n') {
            emit(lx, Token::Kind::Newline, "\\n");
            ++lx.line;
            ++lx.i;
        } else if (std::isalpha((unsigned char)c) || c == '_') {
            std::size_t start = lx.i;
            while (lx.i < src.size() &&
                   (std::isalnum((unsigned char)src[lx.i]) || src[lx.i] == '_')) {
                ++lx.i;
            }
            emit(lx, Token::Kind::Ident, src.substr(start, lx.i - start));
        } else if (std::isdigit((unsigned char)c)) {
            if (!number(lx)) break;
        } else if (c == ',') { emit(lx, Token::Kind::Comma, ",");    ++lx.i; }
        else if (c == ':')   { emit(lx, Token::Kind::Colon, ":");    ++lx.i; }
        else if (c == '[')   { emit(lx, Token::Kind::LBracket, "["); ++lx.i; }
        else if (c == ']')   { emit(lx, Token::Kind::RBracket, "]"); ++lx.i; }
        else {
            fail(lx, std::string("unexpected character '") + c + "'");
            break;
        }
    }
    if (!lx.error.empty()) {
        destroy(lx.head);
        error = lx.error;
        return nullptr;
    }
    emit(lx, Token::Kind::Eof, "");
    return lx.head;
}
