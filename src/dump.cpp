// dump.cpp — hex + ASCII, like the Unix tool `hexdump -C`.
//
// The `dump` function below is GIVEN and working, except for one TODO.
// Read it before you change it: it is the shape of nearly every loop you will
// write this semester (an outer loop over rows, an inner loop over columns).
//
// `show_byte` at the bottom is YOUR WORK.
#include "dump.hpp"

#include <iomanip>
#include <iostream>

// How many bytes we print on one line. Try changing it to 8 and rebuilding.
const std::size_t BYTES_PER_LINE = 16;

// Is this byte something a terminal can print as a glyph?
// 0x20 is space, 0x7E is '~'. Everything outside that range we show as '.'.
static bool is_printable(Byte b) { return b >= 0x20 && b <= 0x7E; }

void dump(const Memory& mem) { dump_range(mem, 0, MEM_SIZE); }

// Lab 3: дамп лише частини пам'яті, щоб показати регіон коду (0x000)
// і регіон даних (0x800) без 256 рядків нулів. Рядки вирівнюємо по 16.
void dump_range(const Memory& mem, std::size_t from, std::size_t len) {
    if (from >= MEM_SIZE) {
        return;
    }
    std::size_t to = from + len;  // перша адреса, яку вже НЕ друкуємо
    if (to > MEM_SIZE) {
        to = MEM_SIZE;
    }
    std::size_t first_row = from - from % BYTES_PER_LINE;

    // std::hex switches the stream to hexadecimal; setfill/setw pad with zeros
    // so every number is the same width and the columns line up.
    for (std::size_t row = first_row; row < to; row += BYTES_PER_LINE) {

        // The address column: 0000, 0010, 0020, ...
        std::cout << std::hex << std::setfill('0') << std::setw(4) << row << "  ";

        // The hex column: 16 bytes, two digits each.
        for (std::size_t col = 0; col < BYTES_PER_LINE; ++col) {
            Byte b = mem.data[row + col];
            std::cout << std::setw(2) << (int)b << ' ';
        }

        std::cout << " |";

        // The ASCII gutter.
        for (std::size_t col = 0; col < BYTES_PER_LINE; ++col) {
            Byte b = mem.data[row + col];
            // TODO(lab-01, M2): when the byte IS printable, print the byte
            //                   itself instead of the dot. One token changes.
            //                   Hint: a Byte sent to std::cout prints as a
            //                   character already - that is the whole joke of
            //                   Lab 1. Right now every byte looks unprintable.
            if (is_printable(b)) {
                std::cout << b;
            } else {
                std::cout << '.';
            }
        }

        std::cout << "|\n";
    }

    // Put the stream back to decimal, or every number you print later is hex.
    std::cout << std::dec << std::setfill(' ');
}

void show_byte(Byte b) {

	std::cout << (int)b << " "; 

	std::cout << "0x" 
              << std::hex << std::setw(2) << std::setfill('0') 
              << static_cast<int>(static_cast<unsigned char>(b)) 
              << std::dec << " ";	


	std::cout << "0b";
    	for (int i = 7; i >= 0; --i) {
        	std::cout << ((b >> i) & 1);
    	}
	std::cout << " ";

	std::cout << "'";
	if (is_printable(b)){
		std::cout << b;
	}
	else{
		 std::cout << ".";
	}
	std::cout << "'";

	 std::cout << "\n";
}
