// assembler.cpp — моє (Lab 8, рівень Basic).
#include "assembler.hpp"

#include <cctype>

#include "../cpu.hpp"

// Ідея: кожен рядок перетворюємо на «форму» — рядок, де мнемоніка й регістри
// написані великими літерами, а кожне число замінене на `n`:
//
//   LOADI A, 65        ->  "LOADI A,n"
//   store [0x800], b   ->  "STORE [n],B"
//   JNZ 2              ->  "JNZ n"
//
// А потім шукаємо цю форму в таблиці, переписаній з ISA.uk.md.
// Розмір з таблиці каже, скільки байтів займає операнд: 1 — немає,
// 2 — один байт imm8, 3 — два байти addr16 (молодший уперед).
struct Form {
    const char* shape;
    Op op;
    int size;
};

// Лише ті інструкції, які мій step() уміє виконувати (рівень Basic у кожній лабі).
static const Form FORMS[] = {
    {"HALT", Op::Halt, 1},          {"NOP", Op::Nop, 1},
    {"OUT", Op::Out, 1},            {"OUTN", Op::OutN, 1},

    {"ADD A,B", Op::Add, 1},        {"SUB A,B", Op::Sub, 1},
    {"AND A,B", Op::And, 1},        {"OR A,B", Op::Or, 1},
    {"XOR A,B", Op::Xor, 1},        {"NOT A", Op::Not, 1},
    {"SHL A", Op::Shl, 1},          {"SHR A", Op::Shr, 1},
    {"INC A", Op::Inc, 1},          {"DEC A", Op::Dec, 1},
    {"CMP A,B", Op::Cmp, 1},        {"MUL A,B", Op::Mul, 1},

    {"LOADI A,n", Op::LoadiA, 2},   {"LOADI B,n", Op::LoadiB, 2},
    {"LOAD A,[n]", Op::LoadA, 3},   {"LOAD B,[n]", Op::LoadB, 3},
    {"STORE [n],A", Op::StoreA, 3}, {"STORE [n],B", Op::StoreB, 3},

    {"JMP n", Op::Jmp, 3},          {"JZ n", Op::Jz, 3},
    {"JNZ n", Op::Jnz, 3},

    {"CLS", Op::Cls, 1},            {"PLOT", Op::Plot, 1},
    {"SHOW", Op::Show, 1},

    {"PUSH A", Op::PushA, 1},       {"POP A", Op::PopA, 1},
    {"PUSH B", Op::PushB, 1},       {"POP B", Op::PopB, 1},
    {"CALL n", Op::Call, 3},        {"RET", Op::Ret, 1},
};

static std::string upper(const std::string& s) {
    std::string r = s;
    for (char& c : r) c = (char)std::toupper((unsigned char)c);
    return r;
}

// Текст числа (форму вже перевірив лексер) -> значення.
// Рахуємо самі, цифра за цифрою, і зупиняємось, щойно перевалило за 0xFFFF:
// більшого в ember не буває (найширший операнд — 16 бітів).
static bool number_value(const std::string& text, unsigned long& out) {
    unsigned base = 10;
    std::size_t i = 0;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        i = 2;
    } else if (text.size() > 2 && text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
        base = 2;
        i = 2;
    }
    out = 0;
    for (; i < text.size(); ++i) {
        char c = (char)std::tolower((unsigned char)text[i]);
        unsigned digit = (c >= 'a') ? (unsigned)(c - 'a' + 10) : (unsigned)(c - '0');
        out = out * base + digit;
        if (out > 0xFFFF) return false;
    }
    return true;
}

static bool fail(std::string& error, int line, const std::string& msg) {
    error = "line " + std::to_string(line) + ": " + msg;
    return false;
}

bool assemble(const Token* head, Memory& mem, std::size_t& size, std::string& error) {
    std::size_t addr = CODE_LO;  // куди класти наступний байт
    const Token* t = head;

    while (t && t->kind != Token::Kind::Eof) {
        if (t->kind == Token::Kind::Newline) {  // порожній рядок або рядок-коментар
            t = t->next;
            continue;
        }
        int line = t->line;
        if (t->kind != Token::Kind::Ident) {
            return fail(error, line, "expected an instruction, got '" + t->text + "'");
        }

        // Зібрати форму рядка й числа в ньому.
        std::string shape = upper(t->text);
        unsigned long value = 0;  // число з рядка (у кожної форми їх не більше одного)
        bool first_operand = true;
        for (t = t->next; t->kind != Token::Kind::Newline && t->kind != Token::Kind::Eof;
             t = t->next) {
            if (first_operand) {
                shape += ' ';  // пробіл між мнемонікою й операндами
                first_operand = false;
            }
            switch (t->kind) {
            case Token::Kind::Ident:    shape += upper(t->text); break;
            case Token::Kind::Comma:    shape += ',';            break;
            case Token::Kind::LBracket: shape += '[';            break;
            case Token::Kind::RBracket: shape += ']';            break;
            case Token::Kind::Number:
                if (!number_value(t->text, value)) {
                    return fail(error, line, "number " + t->text + " does not fit in 16 bits");
                }
                shape += 'n';
                break;
            case Token::Kind::Colon:
                return fail(error, line, "labels are not supported (one-pass assembler, Basic)");
            default:
                break;
            }
        }

        // Знайти форму в таблиці.
        const Form* form = nullptr;
        for (const Form& f : FORMS) {
            if (shape == f.shape) {
                form = &f;
                break;
            }
        }
        if (!form) {
            return fail(error, line, "unknown instruction '" + shape + "'");
        }
        if (addr + form->size - 1 > CODE_HI) {
            return fail(error, line, "program does not fit in the code region 0x000..0x7FF");
        }

        // Записати байти: опкод, потім операнд.
        mem_set(mem, addr, (Byte)form->op);
        if (form->size == 2) {
            if (value > 0xFF) {
                return fail(error, line, "immediate " + std::to_string(value) +
                                             " does not fit in a byte (0..255)");
            }
            mem_set(mem, addr + 1, (Byte)value);
        } else if (form->size == 3) {
            if (value >= MEM_SIZE) {
                return fail(error, line, "address " + std::to_string(value) +
                                             " is outside memory 0..4095");
            }
            set16(mem, addr + 1, (std::uint16_t)value);  // молодший уперед, Lab 3
        }
        addr += form->size;
        // t зараз на Newline або Eof — наступна ітерація розбереться
    }
    size = addr - CODE_LO;
    return true;
}
