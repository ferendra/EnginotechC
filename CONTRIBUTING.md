# Contributing to EnginotechC++

## Development Setup

### Prerequisites

- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 14+, or MSVC 2019+)
- LLVM/clang for code generation
- Git

### Building from Source

```bash
# Clone repository
git clone https://github.com/engine-soft/enginotech-cpp.git
cd enginotech-cpp

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run tests
cd build
ctest
```

## Project Structure

```
enginotech-cpp/
├── build-system/       # Build system helper
├── compiler/           # Core compiler
│   ├── lexer/          # Tokenizer
│   ├── parser/         # AST parser
│   ├── semantic/       # Semantic analysis
│   ├── types/          # Type checker
│   ├── codegen/        # IR and native codegen
│   ├── ast/            # AST nodes
│   └── diagnostics/    # Error handling
├── std/                # Standard library
├── runtime/            # Runtime library
├── formatter/          # Code formatter
├── linter/             # Linter
├── package-manager/    # Package manager
├── examples/           # Example programs
├── tests/              # Test suite
└── docs/               # Documentation
```

## Code Style

### C++ Code

- Use C++20 standard
- Follow Google C++ Style Guide
- Maximum line length: 100 characters
- Use 4-space indentation

### EnginotechC++ Code

- Use 4-space indentation
- snake_case for functions and variables
- PascalCase for types and structs
- Use `let` for immutable, `mut` for mutable

## Adding New Features

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make changes and test
4. Run the linter: `engc lint src/main.ec`
5. Commit changes: `git commit -m 'Add my feature'`
6. Push to the branch: `git push origin feature/my-feature`
7. Create a Pull Request

## Testing

Run all tests:
```bash
cd build
make test
```

Run specific test:
```bash
cd build
./tests/test_basic
```

## Reporting Issues

1. Check existing issues first
2. Provide minimal reproducible example
3. Include compiler version and OS info
4. Use descriptive titles

## License

This project is licensed under the MIT License.
