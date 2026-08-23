# EnginotechC++ — Language Specification v0.1 (M0 Bootstrap)

## Overview
EnginotechC++ (EC) is a systems programming language with C-like syntax,
designed for: CLI tools, backend services, game tooling, AI infrastructure,
and native applications.

## File Extension
`.ec`

## Compiler
```
engc run main.ec        # Compile and run
engc build main.ec      # Compile to binary
engc test               # Run tests
engc fmt                # Format source
engc lint               # Lint source
engc version            # Show version
engc new myapp          # Create new project
```

## Syntax

### Hello World
```ec
fn main() {
    print("Hello, EnginotechC++!");
}
```

### Variables
```ec
let name = "Pedi";
let age = 14;
let x: int = 100;

mut score = 0;
score = score + 10;
```

### Constants
```ec
const MAX_USERS = 1000;
```

### Types
```text
int, int8, int16, int32, int64
uint, uint8, uint16, uint32, uint64
float32, float64
bool, char, string, byte, void
Option<T>, Result<T, E>
```

### Functions
```ec
fn add(a: int, b: int) -> int {
    return a + b;
}

fn square(x: int) -> int => x * x;
```

### Control Flow
```ec
if condition {
    // ...
} else {
    // ...
}

for item in items {
    print(item);
}

while condition {
    // ...
}

match value {
    1 => { print("one"); }
    2 => { print("two"); }
    _ => { print("other"); }
}
```

### Struct
```ec
struct User {
    name: string;
    age: int;
    email: string;
}
```

### Enums
```ec
enum Status {
    Active,
    Inactive,
    Error
}
```

### Import
```ec
import http;
import json;
```

### Error Handling
```ec
fn read_file(path: string) -> Result<string, Error> {
    // ...
}

let content = read_file("data.txt")?;
```

## Memory Model
M0 bootstrap: reference counting with manual lifetimes.

## Compiler Pipeline
```
Source (.ec) → Lexer → Token Stream
                ↓
              Parser → AST
                ↓
           Semantic Analysis
                ↓
             Type Checking
                ↓
            LLVM IR
                ↓
            Native Binary
```

## Platform Support
- Windows x86_64
- Linux x86_64
- Linux ARM64
- macOS ARM64
- macOS x86_64

## Roadmap
- M0: Lexer, Parser, AST, LLVM codegen (THIS VERSION)
- M1: Full type inference, Option/Result
- M2: Struct fields, method calls
- M3: Module system, std library
- M4: Package manager
- M5: Async/await
- M6: Concurrency primitives
- M7: FFI (C/C++ interop)
- M8: LSP
- M9: Debugger
- M10: WASM target
