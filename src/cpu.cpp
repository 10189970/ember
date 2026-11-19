// cpu.cpp — ДАНО форму. Ваші — case для групи 0x1_ і dump_regs().
#include "cpu.hpp"

#include <iomanip>
#include <iostream>

#include "alu.hpp"

// Опкод — це два бітові поля: старший півбайт — група, молодший — індекс.
struct Decoded {
    Byte group;  // 0x0 — керування, 0x1 — ALU, ...
    Byte index;  // номер інструкції всередині групи
};

static Decoded decode(Byte op) {
    Decoded d;
    d.group = (op >> 4) & 0x0F;  // зсунути поле до нульового біта, потім маска
    d.index = op & 0x0F;
    return d;
}

// Група 0x0_: керування.
static void exec_control(CPU& cpu, Byte op) {
    switch (op) {
    case OP_HALT:
        cpu.halted = true;
        cpu.pc += 1;  // розмір HALT з таблиці — 1
        break;
    case OP_NOP:
        cpu.pc += 1;
        break;
    default:
        cpu.pc += 1;  // поки що як NOP; у Lab 4 це стане помилкою
        break;
    }
}

// Група 0x1_: ALU. Усі інструкції цієї лаби мають розмір 1.
static void exec_alu(CPU& cpu, Byte op) {
    switch (op) {
    case OP_ADD: cpu.a = alu_add(cpu.a, cpu.b, cpu.f); break;
    case OP_SUB: cpu.a = alu_sub(cpu.a, cpu.b, cpu.f); break;
    case OP_AND: cpu.a = alu_and(cpu.a, cpu.b, cpu.f); break;
    case OP_OR:  cpu.a = alu_or(cpu.a, cpu.b, cpu.f);  break;
    case OP_XOR: cpu.a = alu_xor(cpu.a, cpu.b, cpu.f); break;
    case OP_NOT: cpu.a = alu_not(cpu.a, cpu.f);        break;
    case OP_SHL: cpu.a = alu_shl(cpu.a, cpu.f);        break;
    case OP_SHR: cpu.a = alu_shr(cpu.a, cpu.f);        break;
    case OP_INC: cpu.a = alu_inc(cpu.a, cpu.f);        break;
    case OP_DEC: cpu.a = alu_dec(cpu.a, cpu.f);        break;
    default:     break;  // 0x1A CMP, 0x1B MUL — наступні лаби; поки як NOP
    }
    cpu.pc += 1;
}

void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    // *cpu.mem — «пам'ять, на яку вказує cpu.mem». Докладно — Lab 3.
    Byte op = mem_get(*cpu.mem, cpu.pc);

    // Розводимо за полем «група», а не за магічним числом на кожну інструкцію.
    switch (decode(op).group) {
    case 0x0:
        exec_control(cpu, op);
        break;
    case 0x1:
        exec_alu(cpu, op);
        break;
    default:
        cpu.pc += 1;  // поки що як NOP; у Lab 4 це стане помилкою
        break;
    }
}

void dump_regs(const CPU& cpu) {
    std::cout << "PC=" << std::hex << std::setfill('0') << std::setw(4) << cpu.pc
              << "  A=" << std::setw(2) << (int)cpu.a
              << "  B=" << std::setw(2) << (int)cpu.b << std::dec << std::setfill(' ')
              << "  (A=" << (int)cpu.a << " B=" << (int)cpu.b << ")"
              << "  Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c
              << (cpu.halted ? "  [halted]" : "") << '\n';
}
