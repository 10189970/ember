# `ember` — starter skeleton (Lab 1)

Copy this folder to a repository of your own, `git init`, and start from **M1** of
[Lab 01](../lab-01-a-box-of-bytes.md). It builds and runs as-is.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/ember
```

## Why a skeleton exists

Lab 1 is about **bytes and types**. It is not about `while` loops, splitting a
string into words, or `std::setw` — you meet those properly in Labs 4, 5 and 7.
So the parts that need them are given to you, fully written and commented. You
read those. You write the four small things that *are* Lab 1.

## Given — read it, don't rewrite it

| File | What it does |
|---|---|
| `CMakeLists.txt` | C++17, `-Wall -Wextra -Werror`, ASan + UBSan on Debug |
| `src/main.cpp` | the prompt: read a line, split it into words, call your functions |
| `src/memory.hpp` | `Byte`, `MEM_SIZE`, `struct Memory` — the box |
| `src/dump.hpp` | the two declarations |
| `src/dump.cpp` → `dump()` | the hex dump loop |

## Yours — four `TODO(lab-01)` markers

```bash
grep -rn "TODO(lab-01)" src/
```

| # | Where | The job |
|---|---|---|
| 1 | `memory.cpp` → `mem_get` | return the byte, or 0 if the address is outside the box |
| 2 | `memory.cpp` → `mem_set` | write the byte, or return `false` if the address is outside |
| 3 | `dump.cpp` → the ASCII gutter | print the character when the byte is printable |
| 4 | `dump.cpp` → `show_byte` | one byte, four views |

When all four are done:

```txt
ember> set 0 65
ember> set 1 66
ember> get 0
65  0x41  0b01000001  'A'
ember> dump
0000  41 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |AB..............|
```

That is M2 and M3 of Lab 1. M4 (the three deliberate breakages) is in the lab.

Until you implement `mem_set`, `set` accepts everything and stores nothing, and
`get` prints `show_byte: not implemented yet`. That is the starting state, not a
bug.

## Later labs

You keep this repository for all eight labs. Every lab adds one `else if` branch
to the dispatcher in `main.cpp` and one or two new files next to these.

---

## Українською

Скопіюйте цю теку у свій репозиторій — вона вже збирається й запускається.

Lab 1 — про **байти й типи**, а не про цикли, розбір рядка на слова чи
форматування виводу (це Labs 4, 5, 7). Тому все, що потребує ще не пройденого,
вам **дано** — з коментарями, щоб читати. Ви пишете чотири маленькі речі, які й
є Lab 1: `mem_get`, `mem_set`, ASCII-колонку в дампі та `show_byte`.

Знайти свою роботу: `grep -rn "TODO(lab-01)" src/`.

Якщо C++ бачите вперше — спочатку
[C++ за годину](../cpp-survival-kit.notes.md), потім
[інструменти й git](../setup.notes.md).


M1
ember 0.1 - 4096 bytes of memory you can see. Type `help`.
ember> help

M2
ember> dump
0000  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0030  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|

M3


ember 0.1 - 4096 bytes of memory you can see. Type `help`.
ember> help
commands:
  dump              print all 4096 bytes
  get <addr>        show one byte four ways
  set <addr> <val>  write one byte (dec or 0x hex)
  help              this list
  inc <addr>        increment value
  quit              leave
ember> set 0 65
ember> get 0
65 0x41 0b01000001 'A'
ember> set 5000 1
address 5000 is outside 0..4095
ember> set 0 255
ember> inc 0
ember> get 0
0 0x00 0b00000000 '.'
ember>

---

# Lab 02 — Біти не брешуть


## Що додалось

| Файл | Що там |
|---|---|
| `src/cpu.hpp` | ДАНО: опкоди з ISA, `Flags`, `CPU`, оголошення `step` і `dump_regs` |
| `src/cpu.cpp` | `decode()` (група й індекс маскою і зсувом), `step()`, гілки груп `0x0_` і `0x1_`, `dump_regs()` |
| `src/alu.hpp`, `src/alu.cpp` | ALU: `add sub and or xor not shl shr inc dec` над `uint8_t`, кожна виставляє `Z`/`N`/`C` |
| `src/main.cpp` | процесор `CPU cpu` і нові команди `reg`, `regs`, `step`, `alu` |
| `CMakeLists.txt` | `src/cpu.cpp` і `src/alu.cpp` додані в `add_executable` |

Нові команди:

```txt
reg <a|b> <val>   покласти значення в регістр A або B
regs              показати PC, A, B і прапорці Z N C
step              виконати одну інструкцію за адресою PC
alu <op> <a> [b]  перевірити одну операцію ALU без процесора
```

## Які рядки ISA.uk.md реалізовані

| Опкод | Мнемоніка | Розмір | Прапорці | Що робить |
|---|---|---|---|---|
| `0x00` | `HALT` | 1 | — | зупинитись; наступні `step` відмовляють |
| `0x01` | `NOP` | 1 | — | нічого |
| `0x10` | `ADD A, B` | 1 | Z N C | `A = A + B` |
| `0x11` | `SUB A, B` | 1 | Z N C | `A = A - B` (`C` — позика) |
| `0x12` | `AND A, B` | 1 | Z N, C=0 | `A = A & B` |
| `0x13` | `OR A, B` | 1 | Z N, C=0 | `A = A \| B` |
| `0x14` | `XOR A, B` | 1 | Z N, C=0 | `A = A ^ B` |
| `0x15` | `NOT A` | 1 | Z N, C=0 | `A = ~A` |
| `0x16` | `SHL A` | 1 | Z N C | `A = A << 1`, у `C` біт, що виїхав із сьомої позиції |
| `0x17` | `SHR A` | 1 | Z N C | `A = A >> 1`, у `C` біт, що виїхав із нульової позиції |
| `0x18` | `INC A` | 1 | Z N | `A = A + 1`, `C` не чіпає |
| `0x19` | `DEC A` | 1 | Z N | `A = A - 1`, `C` не чіпає |

Решта опкодів поки поводиться як `NOP` (`PC += 1`); з Lab 4 це стане помилкою.

**Власні розширення:** поки немає.

## M1 — ALU руками: розібрані приклади

### Приклад 1: `200 + 100` — загортання й перенесення

```txt
ember> alu add 200 100
result=44  Z=0 N=0 C=1
```

```txt
    1100 1000   = 200 (0xC8)
  + 0110 0100   = 100 (0x64)
  -----------
  1 0010 1100   = 300 (0x12C) = 256 + 44
  ^
  дев'ятий біт не влазить у байт -> C = 1
    0010 1100   = 44 — те, що лишилось у байті
```

`Z = 0`, бо 44 не нуль; `N = 0`, бо сьомий біт результату нуль.

### Приклад 2: `5 - 10` — позика й доповняльний код

```txt
ember> alu sub 5 10
result=251  Z=0 N=1 C=1
```

```txt
  5 - 10 = -5, а в байт від'ємне не влазить, тож він загортається по модулю 256:
  -5 -> 256 - 5 = 251 = 0xFB = 1111 1011

  це той самий байт, що й -5 у доповняльному коді:
  5        = 0000 0101
  ~5       = 1111 1010
  ~5 + 1   = 1111 1011
```

`C = 1`, бо віднімали більше від меншого (позика). `N = 1`, бо сьомий біт одиниця: як `int8_t` цей байт читається як −5. Байт той самий, тлумачення дає тип.

### AND і SHL

```txt
ember> alu and 7 1
result=1  Z=0 N=0 C=0         0000 0111 & 0000 0001 = 0000 0001
ember> alu and 0xF0 0x0F
result=0  Z=1 N=0 C=0         1111 0000 & 0000 1111 = 0000 0000 -> Z = 1
ember> alu shl 0x81
result=2  Z=0 N=0 C=1         1000 0001 << 1 = (1) 0000 0010 -> одиниця виїхала в C
```

## M2 — `checks/lab-02.txt`

`reg a 200`, `reg b 100`, у пам'яті `ADD A, B` і `HALT`, три `step`:

```txt
ember> regs
PC=0x000  A=200 (0xc8)  B=100 (0x64)  Z=0 N=0 C=0
ember> step
mem[0x000]=0x10  ->  PC=0x001  A= 44 (0x2c)  B=100 (0x64)  Z=0 N=0 C=1
ember> regs
PC=0x001  A= 44 (0x2c)  B=100 (0x64)  Z=0 N=0 C=1
ember> step
mem[0x001]=0x00  ->  PC=0x002  A= 44 (0x2c)  B=100 (0x64)  Z=0 N=0 C=1  [halted]
ember> step
halted
ember> regs
PC=0x002  A= 44 (0x2c)  B=100 (0x64)  Z=0 N=0 C=1  [halted]
```

Усе як у CHECKS.md: `A = 44`, `C = 1`, `Z = N = 0`, `PC` після першого кроку 1, після `HALT` 2, третій `step` відмовляє, і `PC` лишається 2.

## M3 — трасування

### `AND`, `HALT` з `A = 7`, `B = 1`

```txt
reg a 7
reg b 1
set 0 0x12
set 1 0x00
step
step
```

| Крок | PC | mem[PC] | Інструкція | A після | Z N C |
|---|---|---|---|---|---|
| 1 | `0x000` | `0x12` | `AND A, B` | 1 (`0000 0001`) | 0 0 0 |
| 2 | `0x001` | `0x00` | `HALT` | 1 | 0 0 0 |

Після цього `PC = 0x002`, наступний `step` друкує `halted`.

### Моя програма: `SHL`, поки не з'явиться перенесення, і далі

`A = 0x41`, `B = 0x0F`, байти `16 16 19 12 11 00`:

```txt
reg a 0x41
reg b 0x0F
set 0 0x16
set 1 0x16
set 2 0x19
set 3 0x12
set 4 0x11
set 5 0x00
```

| Крок | PC | mem[PC] | Інструкція | A після | Z N C | Що сталось |
|---|---|---|---|---|---|---|
| 1 | `0x000` | `0x16` | `SHL A` | 130 = `1000 0010` | 0 1 0 | `0100 0001` зсунулось ліворуч, виїхав 0; сьомий біт став 1 -> `N = 1` |
| 2 | `0x001` | `0x16` | `SHL A` | 4 = `0000 0100` | 0 0 1 | виїхала одиниця -> `C = 1` |
| 3 | `0x002` | `0x19` | `DEC A` | 3 = `0000 0011` | 0 0 1 | `DEC` не чіпає `C`, тому одиниця з кроку 2 лишилась |
| 4 | `0x003` | `0x12` | `AND A, B` | 3 = `0000 0011` | 0 0 0 | `0000 0011 & 0000 1111`; `AND` скидає `C` |
| 5 | `0x004` | `0x11` | `SUB A, B` | 244 = `1111 0100` | 0 1 1 | 3 − 15 = −12 -> 256 − 12 = 244; позика -> `C = 1` |
| 6 | `0x005` | `0x00` | `HALT` | 244 | 0 1 1 | `PC = 0x006`, далі `step` відмовляє |

## M4 — розбір опкода маскою і зсувом

`decode()` у `src/cpu.cpp` ділить байт опкода на два поля:

```txt
op    = 0x16 = 0001 0110
group = (op >> 4) & 0x0F = 0001        -> група 0x1_ (ALU)
index =  op       & 0x0F =      0110   -> шоста інструкція групи (SHL)
```

`step()` розводить `switch` за полем `group`: `0x0_` іде в `exec_ctrl()`, `0x1_` в `exec_alu()`. Всередині ALU інструкцію визначає `index`, а мітки `case` записані як `OP_ADD & 0x0F`, `OP_SHL & 0x0F` і так далі, тобто це ті самі опкоди з таблиці, з яких маска лишила молодший півбайт.

## Баг із пріоритетом операцій

`==` зв'язує тісніше за `&`. Перевірка «сьомий біт виставлений» без дужок:

```cpp
std::uint8_t r = 0x82;               // 1000 0010: сьомий біт = 1
bool n_bad  = r & 0x80 == 0x80;      // насправді r & (0x80 == 0x80) = r & 1 = 0
bool n_good = (r & 0x80) == 0x80;    // 1
```

З нашими прапорцями `-Wall -Wextra -Werror` це навіть не збирається:

```txt
error: suggest parentheses around comparison in operand of '&' [-Werror=parentheses]
```

А якщо вимкнути попередження, програма друкує `0 1`: перший варіант каже, що сьомий біт не виставлений, хоча він виставлений. Тому в `alu.cpp` прапорець `N` рахується з дужками: `((r >> 7) & 1) != 0`.


---

# Lab 03 — Адреси, а не імена

**Рівень: Basic** — «процесор дістає до пам'яті».

## Що додалось

| Файл | Що там |
|---|---|
| `src/memory.hpp`, `src/memory.cpp` | `get16` (дано) і `set16` (моє, дзеркало `get16`); константи `CODE_LO/HI`, `DATA_LO/HI` з карти пам'яті |
| `src/cpu.hpp` | опкоди `OUT`, `OUTN` і `0x20`–`0x25`; поле `error`; оголошення `run()` |
| `src/cpu.cpp` | `OUT`/`OUTN` у групі `0x0_`, нова група `0x2_` (`exec_data`), `instr_size()` з таблиці ISA, перевірка меж у `step`, `run()` |
| `src/dump.cpp` | `dump_range()` — дамп лише потрібних рядків; ASCII-колонка нарешті друкує літери (TODO з Lab 1) |
| `src/main.cpp` | команди `run`, `get16`, `set16`, `dump <addr> <len>` |
| `programs/lab03-*.txt` | сценарії, якими зроблені всі виводи нижче: `./build/ember < programs/lab03-hello.txt` |

Нові команди:

```txt
dump <addr> <len>  тільки рядки, що покривають addr..addr+len-1
run                step, поки не HALT або помилка
get16 <addr>       два байти як одне 16-бітне число, молодший перший
set16 <addr> <v>   записати 16-бітне число двома байтами, молодший перший
```

## Які рядки ISA.uk.md реалізовані (нові в цій лабі)

| Опкод | Мнемоніка | Розмір | Прапорці | Що робить |
|---|---|---|---|---|
| `0x02` | `OUT` | 1 | — | надрукувати `A` як символ |
| `0x03` | `OUTN` | 1 | — | надрукувати `A` як десяткове число і пробіл |
| `0x20` | `LOADI A, imm8` | 2 | — | `A = imm8` |
| `0x21` | `LOADI B, imm8` | 2 | — | `B = imm8` |
| `0x22` | `LOAD A, [addr16]` | 3 | — | `A = mem[addr]` |
| `0x23` | `LOAD B, [addr16]` | 3 | — | `B = mem[addr]` |
| `0x24` | `STORE [addr16], A` | 3 | — | `mem[addr] = A` |
| `0x25` | `STORE [addr16], B` | 3 | — | `mem[addr] = B` |

`MOV` і все з регістром `H` (`0x26`–`0x2C`) — рівень Standard, поки поводяться як `NOP`.
`LOAD`/`LOADI` прапорці **не чіпають** (ISA §3).

## M1 — `set16`, молодший байт уперед

`get16` дано: `lo | (hi << 8)`. `set16` робить навпаки — розрізає число на два байти:

```cpp
Byte lo = (Byte)(value & 0xFF);         // маска: молодші 8 бітів
Byte hi = (Byte)((value >> 8) & 0xFF);  // зсув: старші 8 бітів опускаються вниз
mem_set(mem, addr, lo);                 // молодший — за меншою адресою
mem_set(mem, addr + 1, hi);
```

Якщо `addr + 1 >= MEM_SIZE`, `set16` повертає `false` і **нічого не пише**, щоб у
пам'яті не лишилась половина числа.

Дамп (`programs/lab03-set16.txt`):

```txt
ember> set16 0 0x1234
ember> dump 0 16
0000  34 12 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |4...............|
ember> get 0
52 0x34 0b00110100 '4'
ember> get 1
18 0x12 0b00010010 '.'
ember> get16 0
0x1234 = 4660
ember> set16 4095 1
address 4095 + 1 is outside 0..4095
```

`0x1234` лягло як `34 12`: спочатку молодший байт `0x34`, потім старший `0x12`.
Саме так у потоці інструкцій лежать усі 16-бітні операнди: `LOAD A, [0x0800]` — це `22 00 08`.

## M2 — як `step` читає операнди

`PC` — це адреса гостя (`uint16_t`, індекс у `data`). Операнд лежить одразу після опкода:

```txt
LOADI A, imm8       mem[PC] = 0x20   mem[PC+1] = imm8                 PC += 2
LOAD  A, [addr16]   mem[PC] = 0x22   mem[PC+1] = lo   mem[PC+2] = hi  PC += 3
```

Тому `addr = get16(mem, pc + 1)`, а `PC` рухається на `instr_size(op)` — розмір із таблиці.

Перевірки меж (замість тихого загортання):

- `PC >= MEM_SIZE` — помилка;
- опкод на `0xFFF`, а операнди мали б бути на `0x1000`, `0x1001` — помилка («instruction runs past end of memory»);
- `addr16` у `LOAD`/`STORE` може бути до `0xFFFF`, а пам'ять лише до `0xFFF` — помилка.

Після помилки машина зупинена (`[error]` у `regs`), `PC` лишається на інструкції, що впала,
наступний `step` відмовляє:

```txt
; LOADI A, 0x58 ; STORE [0x1000], A        байти 20 58 24 00 10
ember> run
error: address out of memory: 0x1000 (memory is 0x000..0xfff)

2 steps
PC=0002  A=58  B=00  (A=88 B=0)  Z=0 N=0 C=0  [error]
ember> step
halted
```

## M3 — програма: код на `0x000`, дані на `0x800`

```txt
дані:
0800  41 42 43 00          "ABC\0"

код:
адр.  байти       текст
0000  22 00 08    LOAD A, [0x0800]    ; A = mem[0x800] = 'A'
0003  02          OUT                 ; друкує A
0004  00          HALT
```

`programs/lab03-hello.txt`:

```txt
ember> dump 0x000 16
0000  22 00 08 02 00 00 00 00 00 00 00 00 00 00 00 00  |"...............|
ember> dump 0x800 16
0800  41 42 43 00 00 00 00 00 00 00 00 00 00 00 00 00  |ABC.............|
ember> run
A
3 steps
PC=0005  A=41  B=00  (A=65 B=0)  Z=0 N=0 C=0  [halted]
```

Літера `A` надрукувалась. Програма знайшла дані **за адресою**: у самому коді немає
байта `0x41`, є лише `00 08` — номер «будинку», де він лежить.

### Друга програма: усі інструкції Basic, покроково

`programs/lab03-copy.txt` — читає `A` і `C` з даних, друкує `A` двома способами, копіює `'C'`
в `0x810` і кладе туди ж `'Z'`, узяту з безпосереднього операнда.

```txt
адр.  байти       текст
0000  22 00 08    LOAD  A, [0x0800]   ; A = 'A' (65)
0003  02          OUT                 ; друкує A
0004  03          OUTN                ; друкує "65 "
0005  23 02 08    LOAD  B, [0x0802]   ; B = 'C' (67)
0008  25 10 08    STORE [0x0810], B   ; mem[0x810] = 'C'
000B  20 5A       LOADI A, 0x5A       ; A = 'Z' — число прямо в інструкції
000D  24 11 08    STORE [0x0811], A   ; mem[0x811] = 'Z'
0010  02          OUT                 ; друкує Z
0011  00          HALT
```

```txt
ember> dump 0x000 32
0000  22 00 08 02 03 23 02 08 25 10 08 20 5a 24 11 08  |"....#..%.. Z$..|
0010  02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
ember> dump 0x800 32
0800  41 42 43 00 00 00 00 00 00 00 00 00 00 00 00 00  |ABC.............|
0810  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
ember> step
mem[0x000]=0x22  ->  PC=0003  A=41  B=00  (A=65 B=0)  Z=0 N=0 C=0
ember> step
mem[0x003]=0x02  ->  APC=0004  A=41  B=00  (A=65 B=0)  Z=0 N=0 C=0
ember> step
mem[0x004]=0x03  ->  65 PC=0005  A=41  B=00  (A=65 B=0)  Z=0 N=0 C=0
ember> step
mem[0x005]=0x23  ->  PC=0008  A=41  B=43  (A=65 B=67)  Z=0 N=0 C=0
ember> step
mem[0x008]=0x25  ->  PC=000b  A=41  B=43  (A=65 B=67)  Z=0 N=0 C=0
ember> step
mem[0x00b]=0x20  ->  PC=000d  A=5a  B=43  (A=90 B=67)  Z=0 N=0 C=0
ember> step
mem[0x00d]=0x24  ->  PC=0010  A=5a  B=43  (A=90 B=67)  Z=0 N=0 C=0
ember> step
mem[0x010]=0x02  ->  ZPC=0011  A=5a  B=43  (A=90 B=67)  Z=0 N=0 C=0
ember> step
mem[0x011]=0x00  ->  PC=0012  A=5a  B=43  (A=90 B=67)  Z=0 N=0 C=0  [halted]
ember> dump 0x800 32
0800  41 42 43 00 00 00 00 00 00 00 00 00 00 00 00 00  |ABC.............|
0810  43 5a 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |CZ..............|
```

(`A`, `65 ` і `Z` перед `PC=` — це те, що надрукували `OUT`/`OUTN` посеред рядка `step`.)

Зверніть увагу, як `PC` стрибає не на 1: `0 → 3` (`LOAD`, 3 байти), `3 → 4 → 5` (`OUT`, `OUTN`
по 1), `B → D` (`LOADI`, 2 байти). Прапорці весь час `0 0 0`: пересилання їх не чіпає.

## Вказівник хоста і адреса гостя (коротко)

`CPU` тримає `Memory* mem` — вказівник **хоста**: адреса масиву в моєму процесі C++.
`*cpu.mem` — це сама пам'ять. А `pc` і `addr16` — адреси **гостя**: звичайні числа
`0x000`–`0xFFF`, індекси в `data`. Програма всередині `ember` бачить тільки числа; вказівник
хоста для неї нічого не означає і щоразу інший. Тому `LOAD` бере `uint16_t addr` і перевіряє
`addr < MEM_SIZE`, а не робить `Byte*` і не рахує на ньому.
