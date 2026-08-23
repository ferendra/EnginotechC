# Getting Started with EnginotechC++

## Installation

### From Source

```bash
git clone https://github.com/engine-soft/enginotech-cpp.git
cd enginotech-cpp
mkdir build && cd build
cmake ..
make
sudo make install
```

### Quick Start

1. Create a new project:
   ```bash
   engc new my_project
   cd my_project
   ```

2. Edit `src/main.ec`:
   ```eng
   fn main() {
       let message = "Hello, EnginotechC++!";
       print(message);
   }
   ```

3. Build and run:
   ```bash
   engc build src/main.ec
   ./main.exe
   ```

## Language Basics

### Variables

```eng
let x = 42;           // immutable integer
let name = "world";   // string
let flag = true;      // boolean

mut y = 10;           // mutable variable
y = y + 5;            // reassignment
```

### Functions

```eng
fn add(a: int, b: int) -> int {
    return a + b;
}

fn greet(name: string) {
    print("Hello, " + name + "!");
}
```

### Control Flow

```eng
// If-else
if condition {
    // do something
} else {
    // do something else
}

// For loop
for item in collection {
    print(item);
}

// While loop
while count > 0 {
    count = count - 1;
}
```

### Structs

```eng
struct Point {
    x: int;
    y: int;
}

fn main() {
    let p = Point { x: 10, y: 20 };
    print(p.x);
}
```

## Project Structure

```
my_project/
├── .engc          # Project config
├── src/           # Source files
│   └── main.ec
├── tests/         # Test files
└── docs/          # Documentation
```

## Commands

| Command | Description |
|---------|-------------|
| `engc new <name>` | Create new project |
| `engc build <file.ec>` | Compile to binary |
| `engc run <file.ec>` | Compile and run |
| `engc lint <file.ec>` | Check source |
| `engc test [dir]` | Run tests |
| `engc help` | Show help |

## Next Steps

- [Language Specification](../language/spec.md)
- [Standard Library](../../std/README.md)
- [Examples](../../examples/README.md)
