# EnginotechC++ — Getting Started

## Installation

### From Source
```bash
git clone https://github.com/enginotech/enginotech-cpp.git
cd enginotech-cpp
mkdir build && cd build
cmake .. -DLLVM_DIR=<llvm-install>/lib/cmake/llvm
cmake --build . --config Release
```

### Binary Release (coming soon)
```bash
curl -sSL https://github.com/enginotech/enginotech-cpp/releases/download/v0.1.0/engc-win.exe -o engc.exe
```

## Your First Program

Create `hello.ec`:
```ec
fn main() {
    let name = "World";
    print("Hello, {name}!");
}
```

Run it:
```bash
engc run hello.ec
# Output: Hello, World!
```

## Project Structure

```
my-project/
├── src/
│   └── main.ec       # Entry point
├── tests/
│   └── test_main.ec  # Tests
├── docs/
│   └── README.md
├── .engc             # Project config
├── engc.lock         # Dependency lockfile
└── README.md
```

## Commands

```bash
engc new myapp          # Create new project
engc run main.ec        # Compile and run
engc build main.ec      # Compile to binary
engc test               # Run tests
engc fmt                # Format code
engc lint               # Check for issues
engc doc main.ec        # Generate docs
engc version            # Show version
```

## Next Steps

1. Read the [Language Reference](./language/reference.md)
2. Check out [Examples](../../examples/)
3. Join the [community](https://github.com/enginotech/enginotech-cpp)
