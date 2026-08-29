# EnginotechC++

A modern, modular, industrial-grade programming language with C-like syntax.

**Version:** 0.5.0 (Generics & Monomorphization)
**Status:** Pre-release — Lexer, Parser, AST, Semantic Analysis, LLVM Codegen working
**Language:** C++20 with LLVM 14+
**Build System:** CMake 3.16+
**Build Scripts:** Shell (.sh), JavaScript (.js), Python (.py)
**Target:** x86_64 (Windows, Linux, macOS), Web (JavaScript), Bare-metal

## Quick Start

```bash
# Clone and build
git clone https://github.com/enginotech/enginotech-cpp.git
cd enginotech-cpp
mkdir build && cd build
cmake .. -DLLVM_DIR=<llvm-install>/lib/cmake/llvm
cmake --build . --config Release

# Run a program
./engc run examples/hello/main.ec
```

## Hello World

```ec
fn main() {
    print("Hello, EnginotechC++!");
}
```

```bash
engc run main.ec
# Output: Hello, EnginotechC++!
```

## Features (M0)

- [x] Lexer with token system
- [x] Recursive descent parser
- [x] AST with expression and statement trees
- [x] Semantic analysis
- [x] Type checking (basic)
- [x] LLVM IR generation (x86_64)
- [x] Native binary compilation
- [x] CLI: `run`, `build`, `new`, `test`, `fmt`, `lint`

## Architecture

```
compiler/
├── lexer/     — Token stream generation
├── parser/    — Recursive descent parser → AST
├── ast/       — AST node definitions
├── diagnostics/ — Error/warning messages
├── semantic/  — Semantic analysis
├── types/     — Type system & checker
├── codegen/   — LLVM IR generation
└── driver/    — CLI entry point (engc)
```

## Build Requirements

- C++20 compatible compiler (MSVC, GCC, Clang)
- LLVM 14+ with development headers
- CMake 3.16+

## License

MIT

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md)

## 🌐 Web Development (JavaScript Target)

EC can now compile to JavaScript for web applications!

```bash
# Compile to JavaScript
engc build --target web hello.ec hello.js

# Run with Node.js
node hello.js

# Use in browser
# Include in HTML: <script src="hello.js"></script>
```

## 🔒 Strict Typing (v0.3.0+)

**New:** Functions must have explicit return types!

```ec
// ✅ Good - explicit return type
fn add(a: int, b: int) -> int {
    return a + b;
}

// ❌ Error - missing return type
fn add(a: int, b: int) {  // E2006: Function must have return type
    return a + b;
}
```

### Type Errors

| Code | Description |
|------|-------------|
| E2006 | Function missing explicit return type |
| E2004 | Return type mismatch |
| E2003 | Assignment type mismatch |
| E2005 | If/While condition must be bool |

## 📊 Strong Type System

### Supported Types

| EC Type | LLVM Type | Description |
|---------|-----------|-------------|
| `int`, `int8-64`, `uint`, `uint8-64` | i8/i16/i32/i64 | Integers (proper sizes!) |
| `float`, `float32`, `float64`, `double` | float/double | Floating point |
| `bool` | i1 | Boolean |
| `string`, `str` | ptr | String |
| `char` | i32 | Character |
| `byte` | i8 | Byte |
| `void` | void | No return |

### Smart Type Inference

```ec
let x = 42;           // inferred as int
let pi = 3.14;        // inferred as float
let name = "EC";      // inferred as string
let active = true;    // inferred as bool
```

## 🎯 Usage Examples

```bash
# Native compilation (no GIL, native execution)
engc build hello.ec hello
./hello

# Web compilation
engc build --target web hello.ec hello.js
node hello.js

# Embedded (bare-metal kernel)
engc build --target baremetal kernel.ec kernel.bin
qemu-system-x86_64 -kernel kernel.bin
```

## 🗣️ Human-Friendly Syntax (v0.4.0)

Write code that reads like plain language — every construct maps onto standard EC:

```ec
function main() {
    set counter = 3            // declare a mutable variable
    set total to 0             // '=' or the word 'to' both work

    repeat counter times {     // loop exactly N times
        say "tick"             // print without parentheses
    }

    for i in 1..4 {
        total += i
    }

    if total is 6 and not false {
        say "sum = " + str(total)
    }
}
```

| Human word | Meaning |
|------------|---------|
| `say <expr>` | `print(<expr>)` (also bare `print` / `output`) |
| `set x = v` / `set x to v` | declare mutable variable |
| `ask name` | `mut name = input()` |
| `repeat n times { }` | `for __i in 0..n { }` |
| `give [expr]` | `return` |
| `function f() {}` | alias for `fn` |
| `is`, `and`, `or`, `not` | `==`, `&&`, `\|\|`, `!` |

See [examples/human/main.ec](examples/human/main.ec).

## 🚀 M1–M4 Features (v0.3.0)

### Dynamic Typing Mode (`--dynamic`)

Return types become optional — Python-style:

```ec
// With --dynamic flag:
fn add(a, b) { return a + b; }   // no -> int required!

// Static mode still works (default):
fn add(a: int, b: int) -> int { return a + b; }
```

```bash
engc run --dynamic script.ec out
```

### Interactive REPL

```bash
engc repl
>>> let x = 42
; ok
>>> .exit
```

### Struct Methods (inline)

Methods can be declared directly inside struct bodies — `self` is the explicit first parameter:

```ec
struct Point {
    x: int;
    y: int;

    fn dist(self) -> int {
        return self.x * self.x + self.y * self.y;
    }
}

fn main() {
    let p = Point { x: 3, y: 4 };
    print(str(p.dist()));   // dot-notation method call
}
```

Classic `impl Point { ... }` blocks continue to work.

### Modules & Imports

`import` statements now resolve to real files. Search order:

1. `<dir of importing file>/<path>.ec`
2. `./<path>.ec`
3. `./std/<path>.ec`
4. `.ec_packages/<path>.ec` (installed packages)

```ec
import mathutil;        // loads mathutil.ec from the same directory
import mylib.utils;     // loads mylib/utils.ec (dots → slashes)
```

Installed packages participate automatically:

```bash
engc add mylib          # installs into .ec_packages/
```
```ec
import mylib;           // compiler finds it in .ec_packages/mylib/
```

## 🔬 Generics (v0.5.0)

Full generic programming support with monomorphization:

### Generic Functions
```ec
fn identity<T>(x: T) -> T { return x; }
fn pair<A, B>(a: A, b: B) -> Pair<A, B> { return Pair { a, b }; }

let x = identity(42);        // T = int (inferred)
let y = identity("hello");   // T = string (inferred)
```

### Generic Structs
```ec
struct Vec<T> {
    data: array<T>;
    len: int;
}

impl<T> Vec<T> {
    fn new() -> Vec<T> { return Vec { data: [], len: 0 }; }
    fn push(self, item: T) -> void { }
    fn pop(self) -> Option<T> { }
}

let v: Vec<int> = Vec<int>::new();
v.push(1);
v.push(2);
```

### Generic Enums
```ec
enum Option<T> {
    Some(T),
    None
}

enum Result<T, E> {
    Ok(T),
    Err(E)
}

let opt: Option<string> = Option.Some("hello");
let res: Result<int, string> = Result.Ok(42);
```

### Stdlib Generic Types
```ec
import std.vec;
import std.option;
import std.result;

let mut v = Vec<int>::new();
v.push(1);
let first = v.get(0).unwrap_or(-1);

let opt = Option.Some("value");
let mapped = opt.map(|s| s + "!");

let result = Result.Ok(42);
let doubled = result.map(|x| x * 2);
```

**Monomorphization**: Compiler generates specialized code per concrete type (e.g., `identity<int>`, `identity<string>`) for zero-overhead abstraction.
