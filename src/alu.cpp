// alu.cpp — ALU над uint8_t (Lab 2).
#include "alu.hpp"

#include <cassert>

// Z і N виставляються однаково для всіх операцій групи 0x1_.
static void set_zn(Byte r, Flags& f) {
    f.z = (r == 0);
    f.n = (r & 0x80) != 0;  // дужки обов'язкові: `r & 0x80 != 0` це `r & 1`
}

// ADD, зібраний із бітів: повний суматор на кожен з восьми розрядів.
//   сума  = a ^ b ^ carry
//   carry = (a & b) | (carry & (a ^ b))
// Перенесення, що лишилось після 7-го біта, і є прапорець C.
Byte alu_add(Byte a, Byte b, Flags& f) {
    unsigned result = 0;
    unsigned carry = 0;
    for (unsigned i = 0; i < 8; ++i) {
        unsigned x = (a >> i) & 1u;
        unsigned y = (b >> i) & 1u;
        unsigned s = x ^ y ^ carry;
        carry = (x & y) | (carry & (x ^ y));
        result |= (s << i);
    }
    Byte r = (Byte)result;
    assert(r == (Byte)(a + b));  // у Debug звіряємось зі звичайним `+`

    set_zn(r, f);
    f.c = (carry != 0);
    return r;
}

Byte alu_sub(Byte a, Byte b, Flags& f) {
    Byte r = (Byte)(a - b);
    set_zn(r, f);
    f.c = (a < b);  // позика: від меншого відняли більше
    return r;
}

Byte alu_and(Byte a, Byte b, Flags& f) {
    Byte r = a & b;
    set_zn(r, f);
    f.c = false;
    return r;
}

Byte alu_or(Byte a, Byte b, Flags& f) {
    Byte r = a | b;
    set_zn(r, f);
    f.c = false;
    return r;
}

Byte alu_xor(Byte a, Byte b, Flags& f) {
    Byte r = a ^ b;
    set_zn(r, f);
    f.c = false;
    return r;
}

Byte alu_not(Byte a, Flags& f) {
    Byte r = (Byte)~a;  // ~ підносить до int; (Byte) лишає молодші 8 бітів
    set_zn(r, f);
    f.c = false;
    return r;
}

Byte alu_shl(Byte a, Flags& f) {
    Byte r = (Byte)(a << 1);
    set_zn(r, f);
    f.c = (a & 0x80) != 0;
    return r;
}

Byte alu_shr(Byte a, Flags& f) {
    Byte r = (Byte)(a >> 1);
    set_zn(r, f);
    f.c = (a & 0x01) != 0;
    return r;
}

Byte alu_inc(Byte a, Flags& f) {
    Byte r = (Byte)(a + 1);
    set_zn(r, f);
    return r;
}

Byte alu_dec(Byte a, Flags& f) {
    Byte r = (Byte)(a - 1);
    set_zn(r, f);
    return r;
}

// Lab 7. Byte * Byte підноситься до int, тож справжній добуток (до 255*255)
// не губиться — з нього й видно, чи вліз він у байт.
Byte alu_mul(Byte a, Byte b, Flags& f) {
    unsigned full = (unsigned)a * (unsigned)b;
    Byte r = (Byte)full;  // молодші 8 бітів
    set_zn(r, f);
    f.c = (full > 0xFF);
    return r;
}
