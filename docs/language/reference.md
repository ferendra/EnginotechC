# EnginotechC++ Language Reference

> **Status:** v0.2.x — Berdasarkan perilaku aktual compiler, bukan aspirasi.
> Diperbarui: 2026-08-22.

## 1. Ringkasan

EnginotechC++ (EC) adalah bahasa pemrograman statically-typed, compiled ke LLVM IR,
dengan sintaks mirip Rust/C. Target utama: sistem programming untuk hardware (ARM Cortex-M, ESP32)
dan native execution (x86_64 Linux).

### Fitur Utama (yang sudah bekerja)
- ✅ Type checking statis (int, float, string, bool, struct, enum)
- ✅ Functions dengan return type eksplisit
- ✅ Variables: `let` (immutable), `mut` (mutable)
- ✅ Control flow: if/else, for-range, while, match
- ✅ Arrays/vectors dengan len/append/for-in
- ✅ Structs dan methods
- ✅ Enum variants
- ✅ String concatenation dengan `+`
- ✅ Function calls (termasuk recursive)
- ✅ OS builtins: sleep, env_get, cwd, path_join
- ✅ Serial port I/O (embedded targets)

### Batasan Saat Ini
- ❌ Tidak ada generics (`vector<Todo>` tidak bekerja)
- ❌ Tidak ada import multi-file (pakai lib/std.ec + concat)
- ❌ Tidak ada operator ternary (`cond ? a : b`)
- ❌ Tidak ada struct field access (`.field` pada instance)
- ❌ String indexing `str[i]` tidak didukung
- ❌ Array initialization dengan type annotation (`array`) harus `vector`

---

## 2. Dasar-Dasar

### 2.1 Program Structure
Setiap program EC harus memiliki fungsi `main()`:

```ec
fn main() {
    let message = "Hello, EC!";
    print(message);
}
```

Script mode juga didukung — statements di top-level akan dibungkus dalam implicit `main()`:

```ec
// Top-level script mode
let x = 42;
print(x);
```

### 2.2 Comments
```ec
// Single line comment

/* Multi-line
   comment */
```

### 2.3 Identifiers
- Dimulai dengan huruf atau underscore
- Diikuti huruf, digit, underscore
- Case-sensitive (`myVar` ≠ `myvar`)
- Keywords tidak boleh dipakai sebagai identifier

---

## 3. Types

### 3.1 Primitive Types

| Type    | LLVM Type  | Deskripsi                  |
|---------|------------|----------------------------|
| int     | i32        | Signed 32-bit integer      |
| int8    | i8         | Small signed integer       |
| int16   | i16        | Medium signed integer      |
| int32   | i32        | Standard signed int        |
| int64   | i64        | Large signed integer       |
| uint    | i32        | Unsigned integer           |
| uint8   | i8         | Small unsigned int         |
| uint16  | i16        | Medium unsigned int        |
| uint32  | i32        | Standard unsigned          |
| uint64  | i64        | Large unsigned int         |
| float   | double     | Double precision float     |
| float32 | float      | Single precision (alias)   |
| float64 | double     | Double precision (alias)   |
| bool    | i1         | true atau false            |
| string  | ptr        | NUL-terminated C string    |
| char    | i32        | Single character (codepoint)|
| void    | void       | Tidak ada return value     |

### 3.2 Inferred Types
Compiler bisa infer type dari literal:

```ec
let x = 42;         // int (dari literal integer)
let f = 3.14;       // float (dari literal float)
let s = "hello";    // string (dari string literal)
let b = true;       // bool (dari boolean literal)
```

### 3.3 Type Annotations
Type annotation bersifat wajib untuk parameter dan return type:

```ec
fn add(a: int, b: int) -> int {
    return a + b;
}
```

### 3.4 Arrays/Vectors
Array direpresentasikan sebagai pointer ke buffer heap dengan header `[len][elems]`.

```ec
let nums: vector = [1, 2, 3, 4, 5];
let n = len(nums);           // length
nums.append(6);              // append element
for x in nums {               // iterate
    print(x);
}
let first = nums[0];          // index access
```

**Catatan:** Gunakan `vector` (bukan `array`) untuk type annotation.

---

## 4. Variables

### 4.1 Let (Immutable)
```ec
let name = "world";
let count: int = 42;
let pi: float = 3.14159;
```

### 4.2 Mut (Mutable)
```ec
mut score = 0;
score = score + 10;

mut items: vector = [];
items.append(1);
items.append(2);
```

### 4.3 Shadowing
Variable baru bisa menimpa variable lama di scope yang lebih dalam:

```ec
fn main() {
    let x = 10;
    {
        let x = 20;  // shadows outer x
        print(x);    // prints 20
    }
    print(x);        // prints 10
}
```

---

## 5. Functions

### 5.1 Definition
Return type wajib kecuali untuk `main()` (implicit `-> int`).

```ec
fn add(a: int, b: int) -> int {
    return a + b;
}

fn greet(name: string) -> string {
    return "Hello, " + name + "!";
}

fn main() {
    let result = add(3, 4);
    print(result);  // prints 7
}
```

### 5.2 Parameters
Parameter functions adalah immutable oleh default:

```ec
fn compute(x: int, y: int) -> int {
    return x * y;
}
```

### 5.3 Recursive Functions
Support recursive calls:

```ec
fn factorial(n: int) -> int {
    if n <= 1 {
        return 1;
    }
    return n * factorial(n - 1);
}
```

### 5.4 Built-in Functions
| Function    | Description                    |
|-------------|--------------------------------|
| `print(x)`  | Print to stdout               |
| `str(x)`    | Convert any type to string    |
| `int(s)`    | Parse string to int           |
| `float(s)`  | Parse string to float         |
| `len(x)`    | Get length of array/string    |
| `abs(x)`    | Absolute value                |
| `min(a,b)`  | Minimum of two values         |
| `max(a,b)`  | Maximum of two values         |
| `sqrt(x)`   | Square root                   |
| `pow(b,e)`  | Power function                |
| `floor(x)`  | Floor                         |
| `ceil(x)`   | Ceiling                       |
| `input()`   | Read line from stdin          |
| `assert(x)` | Runtime assertion             |

### 5.5 OS & Hardware Functions
| Function                    | Description                    |
|-----------------------------|--------------------------------|
| `sleep(ms)`                 | Delay in milliseconds          |
| `env_get(name)`             | Get environment variable       |
| `cwd()`                     | Current working directory      |
| `path_join(base, rel)`      | Join paths                     |
| `run_command(cmd)`          | Execute shell command          |
| `exec_output(cmd)`          | Get command output as string   |
| `open_app(path)`            | Open application               |
| `read_file(path)`           | Read file content              |
| `write_file(path, data)`    | Write to file                  |
| `file_exists(path)`         | Check if file exists           |
| `serial_open(port, baud)`   | Open serial port               |
| `serial_close()`            | Close serial port              |
| `serial_write(data)`        | Write to serial port           |
| `serial_read_line()`        | Read line from serial          |

---

## 6. Control Flow

### 6.1 If/Else
```ec
if condition {
    // true branch
} else if other_condition {
    // other branch
} else {
    // default branch
}
```

Contoh:
```ec
let x = 10;
if x > 5 {
    print("greater");
} else {
    print("lesser");
}
```

### 6.2 For Loop (Range)
```ec
for i in 0..10 {
    print(i);  // 0 to 9
}

for i in 0..=10 {
    print(i);  // 0 to 10 (inclusive)
}
```

### 6.3 For Loop (Iterable)
```ec
let nums: vector = [1, 2, 3, 4, 5];
for x in nums {
    print(x);
}
```

### 6.4 While Loop
```ec
mut i = 0;
while i < 5 {
    print(i);
    i = i + 1;
}
```

### 6.5 Break & Continue
```ec
for i in 0..100 {
    if i == 3 {
        continue;
    }
    if i == 7 {
        break;
    }
    print(i);
}
```

### 6.6 Match
Pattern matching pada integer dan string:

```ec
match value {
    1 => { print("one"); },
    2 => { print("two"); },
    _ => { print("other"); },
}
```

Match juga bisa dipakai dengan enum:

```ec
enum Color { Red, Green, Blue }

fn describe(c: Color) -> string {
    if c == Color.Red {
        return "red";
    } else if c == Color.Green {
        return "green";
    } else {
        return "blue";
    }
}
```

---

## 7. Data Types

### 7.1 Structs
```ec
struct Point {
    x: float,
    y: float,
}

fn new_point(x: float, y: float) -> Point {
    return Point{x: x, y: y};
}
```

### 7.2 Enums
```ec
enum Color {
    Red,
    Green,
    Blue,
}

fn main() {
    let c = Color.Red;
    assert(c == Color.Red);
    print(c);
}
```

Enum variants diakses dengan dot notation: `Color.Red`.

### 7.3 Structs with Methods
Method definitions menggunakan `impl`:

```ec
struct Point {
    x: float,
    y: float,
}

impl Point {
    fn distance_to_origin(p: Point) -> float {
        return sqrt(p.x * p.x + p.y * p.y);
    }
}
```

---

## 8. Operators

### 8.1 Arithmetic
| Operator | Description      |
|----------|------------------|
| `+`      | Addition         |
| `-`      | Subtraction      |
| `*`      | Multiplication   |
| `/`      | Division         |
| `%`      | Modulo           |
| `-x`     | Unary negation   |
| `+x`     | Unary plus       |

### 8.2 Comparison
| Operator | Description      |
|----------|------------------|
| `==`     | Equal            |
| `!=`     | Not equal        |
| `<`      | Less than        |
| `>`      | Greater than     |
| `<=`     | Less or equal    |
| `>=`     | Greater or equal |

### 8.3 Logical
| Operator | Description      |
|----------|------------------|
| `&&`     | AND              |
| `||`     | OR               |
| `!`      | NOT              |

### 8.4 Assignment
| Operator | Example       | Equivalent     |
|----------|---------------|----------------|
| `=`      | `x = 5`       | Assign         |
| `+=`     | `x += 2`      | `x = x + 2`    |
| `-=`     | `x -= 2`      | `x = x - 2`    |
| `*=`     | `x *= 2`      | `x = x * 2`    |
| `/=`     | `x /= 2`      | `x = x / 2`    |

### 8.5 String Operations
- **Concatenation**: `"hello" + " " + "world"`
- **Length**: `len(str)`
- **Substring**: Not yet supported

### 8.6 Operator Precedence
1. `()` �� Grouping
2. `[]` — Indexing
3. `.` — Field access
4. `!` `-` — Unary
5. `*` `/` `%` — Multiplicative
6. `+` `-` — Additive
7. `<` `<=` `>` `>=` — Comparison
8. `==` `!=` — Equality
9. `&&` — AND
10. `||` — OR
11. `=` `+=` `-=` `*=` `/=` — Assignment

---

## 9. String Handling

### 9.1 Representation
String adalah **NUL-terminated C-string** (`ptr i8`). Tidak ada length prefix.

### 9.2 String Interpolation
```ec
let name = "EC";
let version = "0.2";
let msg = "Hello ${name} v${version}!";
```

### 9.3 Memory Management
String concatenation mengalikan buffer baru via `malloc`. Buffer tidak di-free otomatis.

**Rekomendasi:** Hindari loop dengan string concatenation berulang.

### 9.4 Standard Library Functions
```ec
// From lib/std.ec
strReverse(s)        // Reverse string
startsWith(s, p)     // Prefix check
endsWith(s, p)       // Suffix check
containsChar(s, c)   // Character contains
countChar(s, c)      // Character count
repeatChar(c, n)     // Repeat character
padLeft(s, n, c)     // Pad to left
```

---

## 10. Error Handling

### 10.1 Compile-Time Errors
Compiler melaporkan error dengan format:
```
[error] (E####) Message
    | source_line
    |       ^
```

### 10.2 Runtime Assertions
```ec
fn main() {
    assert(1 + 2 == 3);
    assert(true);
}
```

### 10.3 Error Codes
| Code    | Severity | Description                    |
|---------|----------|--------------------------------|
| L000    | Lexer    | Lexer error                    |
| E0001   | Error    | Unexpected token               |
| E1001   | Error    | Undefined variable             |
| E1002   | Error    | Undefined function             |
| E2001   | Error    | Type mismatch in arithmetic    |
| E2003   | Error    | Type mismatch in assignment    |
| E2006   | Error    | Missing return type            |
| CG001   | Error    | LLVM IR generation failed      |
| CG002   | Error    | LLVM compilation failed        |
| W1001   | Warning  | Unused parameter               |

Lihat `docs/diagnostics.md` untuk dokumentasi lengkap.

---

## 11. Modules & Libraries

### 11.1 Standard Library
```ec
import "lib/std.ec";
```

### 11.2 Workaround untuk Import
Karena multi-file import belum stabil, gunakan concat:
```bash
cat lib/std.ec myprogram.ec > /tmp/combo.ec
engc run /tmp/combo.ec
```

### 11.3 Testing
```ec
// Test file pattern
fn test_add() {
    assert(add(2, 3) == 5);
    print("test_add OK");
}
```

---

## 12. Memory Management

### 12.1 Stack Variables
Variabel lokal dialokasikan di stack via `alloca`. Lifetime sepanjang function scope.

### 12.2 Heap Allocation
- Array: `malloc` dengan header `[len][elems]`
- String concatenation: `malloc` untuk buffer baru
- Tidak ada automatic GC

### 12.3 Known Issues
- **Memory leak**: String hasil concatenation tidak di-free
- **Runtime exit crash**: `free(): invalid pointer` saat cleanup (pre-existing)
- **No UAF protection**: Compiler tidak melacak ownership

Lihat `docs/memory-schema.md` untuk detail.

---

## 13. Examples

### 13.1 Hello World
```ec
fn main() {
    let message = "Hello, EnginotechC++!";
    print(message);
}
```

### 13.2 Calculator
```ec
fn add(a: int, b: int) -> int {
    return a + b;
}

fn main() {
    print("10 + 5 = " + str(add(10, 5)));
}
```

### 13.3 Recursion
```ec
fn fib(n: int) -> int {
    if n < 2 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

fn main() {
    print("fib(10) = " + str(fib(10)));
}
```

### 13.4 Struct & Methods
```ec
struct Point {
    x: float,
    y: float,
}

impl Point {
    fn distance(p: Point) -> float {
        return sqrt(p.x * p.x + p.y * p.y);
    }
}

fn main() {
    let p = Point{x: 3.0, y: 4.0};
    let d = Point.distance(p);
    print("Distance: " + str(d));
}
```

### 13.5 Enum
```ec
enum Color { Red, Green, Blue }

fn describe(c: Color) -> string {
    if c == Color.Red {
        return "red";
    } else if c == Color.Green {
        return "green";
    } else {
        return "blue";
    }
}

fn main() {
    let c = Color.Red;
    print(describe(c));
}
```

---

## 14. Build & Run

### 14.1 Quick Start
```bash
# Build compiler
./scripts/build.sh ~/.local/bin/engc

# Run program
engc run program.ec

# Build to LLVM IR
engc build program.ec program.ll

# Test suite
./scripts/test.sh
```

### 14.2 Target Selection
```bash
# Native x86_64
engc run --target native program.ec

# Arduino
engc run --target arduino program.ec

# ESP32
engc run --target esp32 program.ec
```

### 14.3 Environment Variables
- `EC_KEEP_IR` — Simpan intermediate LLVM IR
- `CXXFLAGS` — Override compiler flags (contoh: ASan)

---

## 15. Limitations & Roadmap

### 15.1 Current Limitations
- Tidak ada generics
- Tidak ada trait/implementation system
- Tidak ada borrow checker
- Tidak ada pattern matching dengan destructuring
- String slicing tidak didukung

### 15.2 Planned Features
- Generics (`vector<T>`)
- Trait system
- Borrow checking
- Better error recovery
- Standard library expansion

---

## Referensi

- `docs/diagnostics.md` — Error codes
- `docs/string-contract.md` — String behavior
- `docs/memory-schema.md` — Memory management
- `lib/std.ec` — Standard library source
- `tests/integration/` — Integration tests
