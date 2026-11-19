; hello.asm — Lab 8, Basic. Без міток: кожен символ — LOADI + OUT.
LOADI A, 72     ; 'H'
OUT
LOADI A, 0x45   ; 'E'
OUT
LOADI A, 76     ; 'L'
OUT
OUT             ; ще раз 'L' — A не змінився
LOADI A, 0x4F   ; 'O'
OUT
LOADI A, 0b00100001  ; '!'
OUT
HALT
