# Language Specification

## Overview

EnginotechC++ is a systems programming language with:
- Zero-cost abstractions
- Memory safety
- Pattern matching
- Type inference

## Syntax Reference

### Basic Types

| Type | Description | Size |
|------|-------------|------|
| `int` | Signed integer | Platform |
| `int8`, `int16`, `int32`, `int64` | Fixed-size integers | 1-8 bytes |
| `uint`, `uint8`, etc. | Unsigned integers | 1-8 bytes |
| `float32`, `float64` | Floating point | 4-8 bytes |
| `bool` | Boolean | 1 byte |
| `string` | Text | Dynamic |
| `char` | Character | 1 byte |
| `byte` | Raw byte | 1 byte |
| `void` | No return type | - |

### Declarations

```eng
let x: int = 42;           // Type annotation (optional)
let name = "world";        // Type inference
let items = [1, 2, 3];     // Array literal
```

### Functions

```eng
// Simple function
fn add(a: int, b: int) -> int {
    return a + b;
}

// Function with default parameters
fn greet(name: string = "world") -> string {
    return "Hello, " + name + "!";
}

// Function pointers
type Callback = fn(int) -> int;
```

### Structs

```eng
struct User {
    name: string;
    age: int;
    active: bool;
}

fn main() {
    let user = User {
        name: "Alice",
        age: 30,
        active: true,
    };
}
```

### Enums

```eng
enum Color {
    Red,
    Green,
    Blue,
}

enum Result<T> {
    Ok(T),
    Err(string),
}
```

### Pattern Matching

```eng
fn describe(color: Color) -> string {
    match color {
        Color.Red => "Red is fire",
        Color.Green => "Green is nature",
        Color.Blue => "Blue is ocean",
    }
}
```

### Control Flow

```eng
// If expression
let max = if a > b { a } else { b };

// For loop
for i in 0..10 {
    print(i);
}

// While loop
while !done {
    process();
}
```

### Generics

```eng
struct Box<T> {
    value: T;
}

fn identity<T>(x: T) -> T {
    return x;
}
```

### Traits

```eng
trait Printable {
    fn format(&self) -> string;
}

impl Printable for User {
    fn format(&self) -> string {
        return self.name + " (" + str(self.age) + ")";
    }
}
```

## Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| Logical | `&&`, `||`, `!` |
| Bitwise | `&`, `|`, `^`, `~`, `<<`, `>>` |
| Assignment | `=`, `+=`, `-=`, `*=`, `/=`, `%=` |
| String | `+` (concatenation) |

## Comments

```eng
// Single line comment

/*
   Multi-line
   comment
*/
```

## File Format

Files use `.ec` extension.
