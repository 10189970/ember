; fact.asm — fact(5) з ISA.uk.md §6, але текстом.
; Без міток: fact = 0x0007, base = 0x001B — адреси пораховані руками.
        LOADI A, 5      ; 0000  n = 5
        CALL 0x0007     ; 0002  fact
        OUTN            ; 0005  надрукує 120
        HALT            ; 0006
; fact(n): аргумент в A, результат в A. Псує B.
        LOADI B, 0      ; 0007  fact:
        CMP A, B        ; 0009  n == 0 ?
        JZ 0x001B       ; 000A  -> base
        LOADI B, 1      ; 000D
        CMP A, B        ; 000F  n == 1 ?
        JZ 0x001B       ; 0010  -> base
        PUSH A          ; 0013  зберегти n
        DEC A           ; 0014
        CALL 0x0007     ; 0015  A = fact(n-1)
        POP B           ; 0018  B = n
        MUL A, B        ; 0019
        RET             ; 001A
        LOADI A, 1      ; 001B  base:
        RET             ; 001D
