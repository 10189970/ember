// cpu.cpp — ДАНО форму. Ваші — case для групи 0x1_ і dump_regs() (Lab 2),
//           група 0x2_, OUT/OUTN, перевірка меж і run() (Lab 3),
//           повний декодер, CMP, група 0x3_ і ліміт кроків (Lab 4),
//           група 0x4_ — екран (Lab 5), перехід на enum class Op (Lab 6),
//           MUL, стек, CALL/RET (Lab 7).
#include "cpu.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>

#include "alu.hpp"
#include "display.hpp"

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

// Зупинити машину з помилкою. Далі step відмовляє, як після HALT.
static void fail(CPU& cpu, const char* what, std::size_t addr) {
    std::cout << "error: " << what << " 0x" << std::hex << addr << std::dec
              << " (memory is 0x000..0x" << std::hex << MEM_SIZE - 1 << std::dec << ")\n";
    cpu.error = true;
    cpu.halted = true;
}

// Невідомий опкод — це помилка, а не тихий NOP (Lab 4, M1).
// Байт-сміття в пам'яті має зупинити машину, а не «якось виконатись».
// PC не рухаємо: у regs видно, де саме лежить цей байт.
static void unknown(CPU& cpu, Op op) {
    std::cout << "error: unknown opcode 0x" << std::hex << std::setw(2) << std::setfill('0')
              << (int)(Byte)op << " at 0x" << std::setw(3) << cpu.pc << std::dec << std::setfill(' ')
              << '\n';
    cpu.error = true;
    cpu.halted = true;
}

// Група 0x0_: керування.
static void exec_control(CPU& cpu, Op op) {
    switch (op) {
    case Op::Halt:
        cpu.halted = true;
        cpu.pc += 1;  // розмір HALT з таблиці — 1
        break;
    case Op::Nop:
        cpu.pc += 1;
        break;
    case Op::Out:
        // A — це Byte, тобто uint8_t, а cout друкує його як символ. Lab 1 :)
        std::cout << cpu.a << std::flush;
        cpu.pc += 1;
        break;
    case Op::OutN:
        // (int), щоб надрукувалось число, а не символ
        std::cout << (int)cpu.a << ' ' << std::flush;
        cpu.pc += 1;
        break;
    default:
        unknown(cpu, op);  // 0x04 OUTS — Lab 5
        break;
    }
}

// Група 0x1_: ALU. Усі інструкції цієї лаби мають розмір 1.
static void exec_alu(CPU& cpu, Op op) {
    switch (op) {
    case Op::Add: cpu.a = alu_add(cpu.a, cpu.b, cpu.f); break;
    case Op::Sub: cpu.a = alu_sub(cpu.a, cpu.b, cpu.f); break;
    case Op::And: cpu.a = alu_and(cpu.a, cpu.b, cpu.f); break;
    case Op::Or:  cpu.a = alu_or(cpu.a, cpu.b, cpu.f);  break;
    case Op::Xor: cpu.a = alu_xor(cpu.a, cpu.b, cpu.f); break;
    case Op::Not: cpu.a = alu_not(cpu.a, cpu.f);        break;
    case Op::Shl: cpu.a = alu_shl(cpu.a, cpu.f);        break;
    case Op::Shr: cpu.a = alu_shr(cpu.a, cpu.f);        break;
    case Op::Inc: cpu.a = alu_inc(cpu.a, cpu.f);        break;
    case Op::Dec: cpu.a = alu_dec(cpu.a, cpu.f);        break;
    case Op::Cmp:
        // Те саме віднімання, що й SUB, і ті самі прапорці (C = позика),
        // але результат нікуди не пишемо: A лишається, яким був.
        alu_sub(cpu.a, cpu.b, cpu.f);
        break;
    case Op::Mul: cpu.a = alu_mul(cpu.a, cpu.b, cpu.f); break;
    default:
        unknown(cpu, op);
        return;            // PC не рухаємо
    }
    cpu.pc += 1;
}

// Розмір інструкції в байтах — рівно з таблиці ISA.uk.md, а не на око.
// Стільки байтів step читає, починаючи з PC, і на стільки рухає PC.
static std::size_t instr_size(Op op) {
    switch (op) {
    case Op::LoadiA:
    case Op::LoadiB:
        return 2;  // опкод + imm8
    case Op::LoadA:
    case Op::LoadB:
    case Op::StoreA:
    case Op::StoreB:
    case Op::Jmp:
    case Op::Jz:
    case Op::Jnz:
    case Op::Call:
        return 3;  // опкод + addr16 (молодший байт, старший байт)
    default:
        return 1;
    }
}

// Група 0x2_: пересилання даних.
// Операнд лежить у пам'яті ОДРАЗУ ПІСЛЯ опкода: imm8 за адресою pc + 1,
// addr16 — у pc + 1 (молодший) і pc + 2 (старший). Тому get16(pc + 1).
static void exec_data(CPU& cpu, Op op) {
    Memory& mem = *cpu.mem;  // `*` — піти за вказівником до самої пам'яті

    switch (op) {
    case Op::LoadiA:
        cpu.a = mem_get(mem, cpu.pc + 1);
        break;
    case Op::LoadiB:
        cpu.b = mem_get(mem, cpu.pc + 1);
        break;
    case Op::LoadA:
    case Op::LoadB:
    case Op::StoreA:
    case Op::StoreB: {
        std::uint16_t addr = get16(mem, cpu.pc + 1);
        // addr16 може бути аж до 0xFFFF, а пам'ять — лише до 0xFFF.
        // Не загортаємо й не читаємо тихий нуль: це помилка програми.
        if (addr >= MEM_SIZE) {
            fail(cpu, "address out of memory:", addr);
            return;  // PC не рухаємо, щоб у regs було видно, ДЕ впало
        }
        if (op == Op::LoadA) {
            cpu.a = mem_get(mem, addr);
        } else if (op == Op::LoadB) {
            cpu.b = mem_get(mem, addr);
        } else if (op == Op::StoreA) {
            mem_set(mem, addr, cpu.a);
        } else {
            mem_set(mem, addr, cpu.b);
        }
        break;
    }
    default:
        unknown(cpu, op);  // MOV, LOADH, [H], INCH, DECH — у мене рівень Basic, їх немає
        return;
    }
    // LOAD і STORE прапорці НЕ чіпають (ISA.uk.md §3).
    cpu.pc += instr_size(op);
}

// Група 0x3_: стрибки. Стрибок — це просто запис нової адреси в PC.
static void exec_jump(CPU& cpu, Op op) {
    std::uint16_t target = get16(*cpu.mem, cpu.pc + 1);

    bool take = false;  // чи спрацьовує стрибок
    switch (op) {
    case Op::Jmp: take = true;        break;
    case Op::Jz:  take = cpu.f.z;     break;
    case Op::Jnz: take = !cpu.f.z;    break;
    default:
        unknown(cpu, op);  // 0x33 JC, 0x34 JNC — opt
        return;
    }

    if (take) {
        if (target >= MEM_SIZE) {
            fail(cpu, "jump target out of memory:", target);
            return;
        }
        cpu.pc = target;  // і НЕ додаємо 3: PC уже там, куди треба
    } else {
        cpu.pc += 3;      // не спрацював — іти до наступної інструкції
    }
}

// Група 0x4_: екран. Відеопам'ять — це звичайні байти пам'яті на 0xA00,
// тож ці інструкції лише зручніші за ручні STORE.
static void exec_screen(CPU& cpu, Op op) {
    switch (op) {
    case Op::Cls:
        clear(*cpu.mem);
        break;
    case Op::Plot:
        // C = 1, якщо (A, B) поза екраном; Z і N не чіпаємо
        cpu.f.c = !plot(*cpu.mem, cpu.a, cpu.b);
        break;
    case Op::Show:
        show(*cpu.mem);
        break;
    default:
        unknown(cpu, op);  // 0x42 UNPLOT — opt
        return;
    }
    cpu.pc += 1;
}

// ---- Група 0x5_: стек і виклики (Lab 7) ----
//
// Стек «порожній спадний»: SP вказує на наступний ВІЛЬНИЙ байт.
//   PUSH: mem[SP] = v; SP = SP - 1
//   POP:  SP = SP + 1; v = mem[SP]
// Регіон стека 0xF00..0xFFF. Вийти за нього — помилка машини з моїм
// повідомленням, а не падіння процесу і не тихе загортання.

static void stack_error(CPU& cpu, const char* what) {
    std::cout << "error: stack " << what << " at PC=0x" << std::hex << cpu.pc << " (SP=0x"
              << cpu.sp << ", stack is 0x" << STACK_LO << "..0x" << STACK_HI << ")" << std::dec
              << '\n';
    cpu.error = true;
    cpu.halted = true;
}

// Чи можна покласти ще `n` байтів / зняти `n` байтів.
// Покласти n: найнижчий записаний байт — SP - (n - 1), він має бути >= STACK_LO.
// Зняти n: найвищий прочитаний байт — SP + n, він має бути <= STACK_HI.
static bool can_push(const CPU& cpu, unsigned n) { return cpu.sp >= STACK_LO + (n - 1); }
static bool can_pop(const CPU& cpu, unsigned n) { return cpu.sp + n <= STACK_HI; }

// Без перевірок — їх робить exec_stack ДО того, як щось змінити,
// щоб CALL не поклав половину адреси і не впав посередині.
static void push(CPU& cpu, Byte v) {
    mem_set(*cpu.mem, cpu.sp, v);
    cpu.sp -= 1;
}
static Byte pop(CPU& cpu) {
    cpu.sp += 1;
    return mem_get(*cpu.mem, cpu.sp);
}

static void exec_stack(CPU& cpu, Op op) {
    unsigned need_push = 0;  // скільки байтів інструкція покладе
    unsigned need_pop = 0;   // скільки зніме
    switch (op) {
    case Op::PushA: case Op::PushB: need_push = 1; break;
    case Op::PopA:  case Op::PopB:  need_pop = 1;  break;
    case Op::Call:                  need_push = 2; break;
    case Op::Ret:                   need_pop = 2;  break;
    default:
        unknown(cpu, op);  // 0x56 PUSH H, 0x57 POP H — opt, H у мене немає
        return;
    }
    if (need_push > 0 && !can_push(cpu, need_push)) {
        stack_error(cpu, "overflow");
        return;  // PC і SP не рухаємо
    }
    if (need_pop > 0 && !can_pop(cpu, need_pop)) {
        stack_error(cpu, "underflow");
        return;
    }

    switch (op) {
    case Op::PushA: push(cpu, cpu.a);  cpu.pc += 1; break;
    case Op::PushB: push(cpu, cpu.b);  cpu.pc += 1; break;
    case Op::PopA:  cpu.a = pop(cpu);  cpu.pc += 1; break;
    case Op::PopB:  cpu.b = pop(cpu);  cpu.pc += 1; break;
    case Op::Call: {
        std::uint16_t target = get16(*cpu.mem, cpu.pc + 1);
        if (target >= MEM_SIZE) {
            fail(cpu, "call target out of memory:", target);
            return;
        }
        // Адреса повернення — інструкція ПІСЛЯ CALL, тобто PC + 3.
        // Якби клали сам PC, RET повернувся б знову на CALL — і так вічно.
        std::uint16_t ret = cpu.pc + 3;
        push(cpu, (Byte)(ret & 0x00FF));  // спершу молодший
        push(cpu, (Byte)(ret >> 8));      // потім старший
        cpu.pc = target;                  // і, як у JMP, нічого не додаємо
        break;
    }
    case Op::Ret: {
        Byte hi = pop(cpu);  // знімаємо у зворотному порядку: старший перший
        Byte lo = pop(cpu);
        cpu.pc = (std::uint16_t)((hi << 8) | lo);
        break;
    }
    default:
        break;  // сюди не потрапляємо: невідомі відсіяв перший switch
    }
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
    Byte raw = mem_get(*cpu.mem, cpu.pc);
    // Lab 6: сирий байт -> Op. Приведення до enum із фіксованим типом uint8_t
    // визначене для БУДЬ-ЯКОГО байта, навіть якого немає в переліку, — тому
    // сміття не стає UB, а потрапляє в `default:` і дає «unknown opcode».
    Op op = (Op)raw;

    // Уся інструкція (опкод + операнди) має влізти в пам'ять. Інакше
    // ми б прочитали операнд за MEM_SIZE.
    if (cpu.pc + instr_size(op) > MEM_SIZE) {
        fail(cpu, "instruction runs past end of memory at", cpu.pc);
        return;
    }

    // Розводимо за полем «група», а не за магічним числом на кожну інструкцію.
    switch (decode(raw).group) {
    case 0x0:
        exec_control(cpu, op);
        break;
    case 0x1:
        exec_alu(cpu, op);
        break;
    case 0x2:
        exec_data(cpu, op);
        break;
    case 0x3:
        exec_jump(cpu, op);
        break;
    case 0x4:
        exec_screen(cpu, op);
        break;
    case 0x5:
        exec_stack(cpu, op);
        break;
    default:
        unknown(cpu, op);  // 0x6_ ALLOC — Advanced у Lab 6, у мене немає
        break;
    }
}

void print_layout() {
    std::cout << "sizeof(Flags) = " << sizeof(Flags) << ", sizeof(CPU) = " << sizeof(CPU)
              << ", sizeof(Op) = " << sizeof(Op) << '\n'
              << "  offset  size  field\n";
    // offsetof(T, поле) — на скільки байтів поле відстоїть від початку структури
    struct Row {
        const char* name;
        std::size_t offset;
        std::size_t size;
    };
    const Row rows[] = {
        {"Memory* mem", offsetof(CPU, mem), sizeof(CPU::mem)},
        {"uint16_t pc", offsetof(CPU, pc), sizeof(CPU::pc)},
        {"uint16_t sp", offsetof(CPU, sp), sizeof(CPU::sp)},
        {"Byte a", offsetof(CPU, a), sizeof(CPU::a)},
        {"Byte b", offsetof(CPU, b), sizeof(CPU::b)},
        {"Flags f", offsetof(CPU, f), sizeof(CPU::f)},
        {"bool halted", offsetof(CPU, halted), sizeof(CPU::halted)},
        {"bool error", offsetof(CPU, error), sizeof(CPU::error)},
    };
    std::size_t expected = 0;  // де мало б початись наступне поле без дірок
    for (const Row& r : rows) {
        if (r.offset > expected) {
            std::cout << "  " << std::setw(6) << expected << "  " << std::setw(4)
                      << r.offset - expected << "  (padding)\n";
        }
        std::cout << "  " << std::setw(6) << r.offset << "  " << std::setw(4) << r.size << "  "
                  << r.name << '\n';
        expected = r.offset + r.size;
    }
    if (sizeof(CPU) > expected) {
        std::cout << "  " << std::setw(6) << expected << "  " << std::setw(4)
                  << sizeof(CPU) - expected << "  (padding at the end)\n";
    }
}

void dump_stack(const CPU& cpu) {
    std::cout << "SP=0x" << std::hex << cpu.sp;
    if (cpu.sp >= STACK_HI) {
        std::cout << "  (empty)" << std::dec << '\n';
        return;
    }
    // Верхівка стека — SP + 1 (останній покладений байт), далі вглиб до 0xFFF.
    std::cout << "  top ->";
    int shown = 0;
    for (std::size_t addr = cpu.sp + 1; addr <= STACK_HI && shown < 8; ++addr, ++shown) {
        std::cout << "  [" << addr << "]=" << std::setw(2) << std::setfill('0')
                  << (int)mem_get(*cpu.mem, addr) << std::setfill(' ');
    }
    if ((std::size_t)cpu.sp + 1 + 8 <= STACK_HI) {
        std::cout << "  ...";  // глибше ще є байти, але вісім — досить
    }
    std::cout << std::dec << '\n';
}

void dump_regs(const CPU& cpu) {
    std::cout << "PC=" << std::hex << std::setfill('0') << std::setw(4) << cpu.pc
              << "  SP=" << std::setw(3) << cpu.sp
              << "  A=" << std::setw(2) << (int)cpu.a
              << "  B=" << std::setw(2) << (int)cpu.b << std::dec << std::setfill(' ')
              << "  (A=" << (int)cpu.a << " B=" << (int)cpu.b << ")"
              << "  Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c
              << (cpu.error ? "  [error]" : (cpu.halted ? "  [halted]" : "")) << '\n';
}

long run(CPU& cpu) {
    // Зі стрибками програма може крутитись вічно (JMP сам на себе).
    // Тому не більше MAX_STEPS кроків; потім зупиняємось і кажемо про це.
    // Машина при цьому НЕ halted: можна подивитись regs і зробити run ще раз.
    long steps = 0;
    while (!cpu.halted && steps < MAX_STEPS) {
        step(cpu);
        ++steps;
    }
    if (!cpu.halted) {
        std::cout << "\nstep limit: stopped after " << MAX_STEPS << " steps without HALT\n";
    }
    return steps;
}
