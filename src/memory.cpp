// memory.cpp — YOUR WORK (Lab 1, M3).
//
// Two functions, a few lines each. Both need exactly one `if`: an address is
// valid when it is smaller than MEM_SIZE. Addresses are counted from 0, so the
// last valid address is MEM_SIZE - 1, not MEM_SIZE. That off-by-one is the
// whole subject of Lab 3.
#include "memory.hpp"

Byte mem_get(const Memory& mem, std::size_t addr) {
    
	if (addr < MEM_SIZE){
	return mem.data[addr];
	}else{
	return 0;
	}
}

bool mem_set(Memory& mem, std::size_t addr, Byte value) {
        if (addr < MEM_SIZE){
        	mem.data[addr] = value;
		return true;
        }else{
        	return false;
        }
}

// ---- Lab 3, M1 ----

// ДАНО. Прочитати два байти як одне 16-бітне значення, молодший перший.
std::uint16_t get16(const Memory& mem, std::size_t addr) {
    Byte lo = mem_get(mem, addr);
    Byte hi = mem_get(mem, addr + 1);
    return (std::uint16_t)(lo | (hi << 8));
}

// Дзеркало get16: розрізати значення на два байти і покласти молодший першим.
//   0x1234 -> lo = 0x34 (маска молодших 8 бітів), hi = 0x12 (зсув на 8 праворуч)
// Якщо другий байт не влазить у пам'ять — нічого не пишемо взагалі,
// щоб не лишити в пам'яті половину числа.
bool set16(Memory& mem, std::size_t addr, std::uint16_t value) {
    if (addr + 1 >= MEM_SIZE) {
        return false;
    }
    Byte lo = (Byte)(value & 0xFF);
    Byte hi = (Byte)((value >> 8) & 0xFF);
    mem_set(mem, addr, lo);
    mem_set(mem, addr + 1, hi);
    return true;
}
