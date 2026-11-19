// cpu.cpp — ДАНО форму. Ваші — case для групи 0x1_ і dump_regs() (Lab 2),
//           група 0x2_, OUT/OUTN, перевірка меж і run() (Lab 3).
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
    case OP_OUT:
        // A — це Byte, тобто uint8_t, а cout друкує його як символ. Lab 1 :)
        std::cout << cpu.a << std::flush;
        cpu.pc += 1;
        break;
    case OP_OUTN:
        // (int), щоб надрукувалось число, а не символ
        std::cout << (int)cpu.a << ' ' << std::flush;
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

// Зупинити машину з помилкою. Далі step відмовляє, як після HALT.
static void fail(CPU& cpu, const char* what, std::size_t addr) {
    std::cout << "error: " << what << " 0x" << std::hex << addr << std::dec
              << " (memory is 0x000..0x" << std::hex << MEM_SIZE - 1 << std::dec << ")\n";
    cpu.error = true;
    cpu.halted = true;
}

// Розмір інструкції в байтах — рівно з таблиці ISA.uk.md, а не на око.
// Стільки байтів step читає, починаючи з PC, і на стільки рухає PC.
static std::size_t instr_size(Byte op) {
    switch (op) {
    case OP_LOADI_A:
    case OP_LOADI_B:
        return 2;  // опкод + imm8
    case OP_LOAD_A:
    case OP_LOAD_B:
    case OP_STORE_A:
    case OP_STORE_B:
        return 3;  // опкод + addr16 (молодший байт, старший байт)
    default:
        return 1;
    }
}

// Група 0x2_: пересилання даних.
// Операнд лежить у пам'яті ОДРАЗУ ПІСЛЯ опкода: imm8 за адресою pc + 1,
// addr16 — у pc + 1 (молодший) і pc + 2 (старший). Тому get16(pc + 1).
static void exec_data(CPU& cpu, Byte op) {
    Memory& mem = *cpu.mem;  // `*` — піти за вказівником до самої пам'яті

    switch (op) {
    case OP_LOADI_A:
        cpu.a = mem_get(mem, cpu.pc + 1);
        break;
    case OP_LOADI_B:
        cpu.b = mem_get(mem, cpu.pc + 1);
        break;
    case OP_LOAD_A:
    case OP_LOAD_B:
    case OP_STORE_A:
    case OP_STORE_B: {
        std::uint16_t addr = get16(mem, cpu.pc + 1);
        // addr16 може бути аж до 0xFFFF, а пам'ять — лише до 0xFFF.
        // Не загортаємо й не читаємо тихий нуль: це помилка програми.
        if (addr >= MEM_SIZE) {
            fail(cpu, "address out of memory:", addr);
            return;  // PC не рухаємо, щоб у regs було видно, ДЕ впало
        }
        if (op == OP_LOAD_A) {
            cpu.a = mem_get(mem, addr);
        } else if (op == OP_LOAD_B) {
            cpu.b = mem_get(mem, addr);
        } else if (op == OP_STORE_A) {
            mem_set(mem, addr, cpu.a);
        } else {
            mem_set(mem, addr, cpu.b);
        }
        break;
    }
    default:
        break;  // MOV, LOADH, [H], INCH, DECH — рівень Standard; поки як NOP
    }
    // LOAD і STORE прапорці НЕ чіпають (ISA.uk.md §3).
    cpu.pc += instr_size(op);
}

void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    // Перевірка меж: PC — це адреса, і вона теж може вибігти за коробку.
    if (cpu.pc >= MEM_SIZE) {
        fail(cpu, "PC is outside memory:", cpu.pc);
        return;
    }
    // *cpu.mem — «пам'ять, на яку вказує cpu.mem».
    Byte op = mem_get(*cpu.mem, cpu.pc);

    // Уся інструкція (опкод + операнди) має влізти в пам'ять. Інакше
    // ми б прочитали операнд за MEM_SIZE.
    if (cpu.pc + instr_size(op) > MEM_SIZE) {
        fail(cpu, "instruction runs past end of memory at", cpu.pc);
        return;
    }

    // Розводимо за полем «група», а не за магічним числом на кожну інструкцію.
    switch (decode(op).group) {
    case 0x0:
        exec_control(cpu, op);
        break;
    case 0x1:
        exec_alu(cpu, op);
        break;
    case 0x2:
        exec_data(cpu, op);
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
              << (cpu.error ? "  [error]" : (cpu.halted ? "  [halted]" : "")) << '\n';
}

long run(CPU& cpu) {
    // Стрибків ще немає (Lab 4), тож PC тільки росте і рано чи пізно
    // дійде або до HALT, або до кінця пам'яті. Ліміт — про всяк випадок.
    const long MAX_STEPS = 100000;
    long steps = 0;
    while (!cpu.halted && steps < MAX_STEPS) {
        step(cpu);
        ++steps;
    }
    if (!cpu.halted) {
        std::cout << "stopped after " << MAX_STEPS << " steps without HALT\n";
    }
    return steps;
}
