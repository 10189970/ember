// main.cpp — the prompt.
//
// GIVEN, in full. This file is scaffolding: it reads a line, splits it into
// words, and calls one of your functions. It uses `while`, `if` and functions,
// which the course only explains properly in Labs 4 and 7. That is on purpose —
// in Lab 1 you read this file, you do not write it.
//
// What you add in later labs is one more `else if` branch per command.
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#include "alu.hpp"
#include "cpu.hpp"
#include "display.hpp"
#include "dump.hpp"
#include "memory.hpp"

// Turn a word into a number. Accepts decimal (65) and hex (0x41).
// Returns false if the word is not a number at all.
static bool parse_number(const std::string& word, long& out) {
    try {
        std::size_t used = 0;
        // base 0 means: look at the prefix. "0x41" is hex, "65" is decimal.
        out = std::stol(word, &used, 0);
        return used == word.size(); // reject things like "12abc"
    } catch (...) {
        return false;
    }
}

static void print_help() {
    std::cout << "commands:\n"
              << "  dump              print all " << MEM_SIZE << " bytes\n"
              << "  dump <addr> <len> print only the rows covering addr..addr+len-1\n"
              << "  get <addr>        show one byte four ways\n"
              << "  set <addr> <val>  write one byte (dec or 0x hex)\n"
              << "  help              this list\n"
              << "  inc <addr>        increment value\n"
              << "  reg <a|b> <val>   put a value into a CPU register\n"
              << "  regs              print PC A B Z N C\n"
              << "  step              execute one instruction at PC\n"
              << "  run               step until HALT or error\n"
              << "  get16 <addr>      read 2 bytes as one 16-bit value (low byte first)\n"
              << "  set16 <addr> <v>  write a 16-bit value as 2 bytes (low byte first)\n"
              << "  show              draw the screen (VRAM at 0xA00)\n"
              << "  cls               clear the screen\n"
              << "  plot <x> <y>      light one pixel from C++\n"
              << "  frame             draw a 64x32 border from C++\n"
              << "  quit              leave\n";
}

int main() {
    Memory mem; // 4096 bytes, on the stack, zeroed by the {} in memory.hpp
    CPU cpu;
    cpu.mem = &mem; // «ось твоя пам'ять» — `&` теж розбирає Lab 3

    std::cout << "ember 0.1 - 4096 bytes of memory you can see. Type `help`.\n";

    std::string line;
    while (true) {
        std::cout << "ember> ";

        // getline reads one whole line. It returns false at end of input
        // (Ctrl-D), which is how the loop ends if you never type `quit`.
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        // Split the line into words: "set 0 65" -> cmd="set", args "0" and "65".
        std::istringstream words(line);
        std::string cmd;
        words >> cmd;

        if (cmd.empty()) {
            continue; // the user just pressed Enter
        } else if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "dump") {
            std::string a, n;
            long addr = 0, len = 0;
            if (!(words >> a)) {
                dump(mem);  // без аргументів — уся пам'ять, як раніше
            } else if (!(words >> n) || !parse_number(a, addr) || !parse_number(n, len) ||
                       addr < 0 || len <= 0) {
                std::cout << "usage: dump [<addr> <len>]\n";
            } else {
                dump_range(mem, (std::size_t)addr, (std::size_t)len);
            }
        } else if (cmd == "get") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr)) {
                std::cout << "usage: get <addr>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else {
                show_byte(mem_get(mem, (std::size_t)addr));
            }
        } else if (cmd == "set") {
            std::string a, v;
            long addr = 0, value = 0;
            if (!(words >> a) || !(words >> v) || !parse_number(a, addr) ||
                !parse_number(v, value)) {
                std::cout << "usage: set <addr> <value>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else if (value < 0 || value > 255) {
                // A cell holds ONE byte. 256 does not fit. Lab 1, theory 3.
                std::cout << "a byte is 0..255, got " << value << '\n';
            } else if (!mem_set(mem, (std::size_t)addr, (Byte)value)) {
                std::cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
            }
        } else if (cmd == "inc") {
            std::string a;
            long addr = 0, value = 0;
            if (!(words >> a) || !parse_number(a, addr)) {
                std::cout << "usage: inc <addr>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else {
                value = mem_get(mem, (std::size_t)addr);
                value++;
                mem_set(mem, (std::size_t)addr, value);
            }
        } else if (cmd == "reg") {
            std::string r, v;
            long value = 0;
            if (!(words >> r) || !(words >> v) || !parse_number(v, value) || value < 0 ||
                value > 255) {
                std::cout << "usage: reg <a|b> <0..255>\n";
            } else if (r == "a") {
                cpu.a = (Byte)value;
            } else if (r == "b") {
                cpu.b = (Byte)value;
            } else {
                std::cout << "unknown register: " << r << '\n';
            }
        } else if (cmd == "get16") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr) || addr < 0) {
                std::cout << "usage: get16 <addr>\n";
            } else if ((std::size_t)addr + 1 >= MEM_SIZE) {
                std::cout << "address " << addr << " + 1 is outside 0.." << MEM_SIZE - 1 << '\n';
            } else {
                std::uint16_t v = get16(mem, (std::size_t)addr);
                std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << v
                          << std::dec << std::setfill(' ') << " = " << v << '\n';
            }
        } else if (cmd == "set16") {
            std::string a, v;
            long addr = 0, value = 0;
            if (!(words >> a) || !(words >> v) || !parse_number(a, addr) ||
                !parse_number(v, value)) {
                std::cout << "usage: set16 <addr> <value>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else if (value < 0 || value > 0xFFFF) {
                std::cout << "a 16-bit value is 0..65535, got " << value << '\n';
            } else if (!set16(mem, (std::size_t)addr, (std::uint16_t)value)) {
                std::cout << "address " << addr << " + 1 is outside 0.." << MEM_SIZE - 1 << '\n';
            }
        } else if (cmd == "run") {
            if (cpu.halted) {
                std::cout << "halted\n";
            } else {
                long steps = run(cpu);
                // OUT друкує без переводу рядка, тож закриваємо рядок тут
                std::cout << "\n" << steps << " steps\n";
                dump_regs(cpu);
            }
        } else if (cmd == "show") {
            show(mem);
        } else if (cmd == "cls") {
            clear(mem);
        } else if (cmd == "plot") {
            std::string xs, ys;
            long x = 0, y = 0;
            if (!(words >> xs) || !(words >> ys) || !parse_number(xs, x) || !parse_number(ys, y)) {
                std::cout << "usage: plot <x> <y>\n";
            } else if (!plot(mem, (int)x, (int)y)) {
                std::cout << "pixel (" << x << ", " << y << ") is outside 0..63 x 0..31\n";
            }
        } else if (cmd == "frame") {
            // Рамка: верхній і нижній рядок, лівий і правий стовпчик.
            for (int x = 0; x < SCREEN_W; ++x) {
                plot(mem, x, 0);
                plot(mem, x, SCREEN_H - 1);
            }
            for (int y = 0; y < SCREEN_H; ++y) {
                plot(mem, 0, y);
                plot(mem, SCREEN_W - 1, y);
            }
        } else if (cmd == "regs") {
            dump_regs(cpu);
        } else if (cmd == "step") {
            bool was_halted = cpu.halted;
            if (!was_halted) {
                Byte op = mem_get(mem, cpu.pc);
                std::cout << "mem[0x" << std::hex << std::setw(3) << std::setfill('0') << cpu.pc
                          << "]=0x" << std::setw(2) << (int)op << std::dec << std::setfill(' ')
                          << "  ->  ";
            }
            step(cpu); // якщо машина вже зупинена, step сам відмовить і надрукує "halted"
            if (!was_halted) {
                dump_regs(cpu);
            }
        } else {
            std::cout << "unknown command: " << cmd << " (try `help`)\n";
        }
    }

    return 0; // 0 means "success" to the operating system
}
