# EnginotechC++ — CHANGELOG

## [0.2.1] — 2026-08-23 — Arithmatic Type Checker

### Added
- **Type checking for all arithmetic operations**: The type checker now validates and infers types for arithmetic expressions including `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, and unary operators
- **Numeric type promotion**: Automatic promotion to `float` when float/double operands are present in arithmetic operations
- **Type inference from literal expressions**: When a variable is declared without explicit type annotation (`let x = 10;`), its type is inferred from the initializer expression
- **String concatenation support**: The `+` operator is correctly typed for string concatenation when both operands are strings
- **Improved error messages**: Type mismatch errors now include descriptive messages like "cannot operate on int and string"

### Fixed
- **Duplicate code bug in getArithmeticResultType()**: Removed duplicate return statement and dead code
- **Type inference for let/mut declarations**: Variables declared with `let` or `mut` now correctly use inferred types when no explicit type annotation is provided
- **Undefined variable errors for arithmetic examples**: Fixed type checker to properly resolve variable types from let declarations, eliminating false-positive E1001 errors

### Changed
- Type checker now performs comprehensive type validation across all arithmetic operations
- Error codes updated: E2001 for type mismatch in arithmetic, E2002 for invalid unary operator type, E2003 for precedence/associativity issues, E2004 for return type mismatch, E2005 for invalid condition type

## [0.2.0] — 2026-08-22 — M0 Complete / End-to-End Working Compiler

### Added
- **End-to-end compilation**: .ec source → LLVM IR → native binary via clang, verified on Linux x86_64
- **Array literals & indexing**: `[1, 2, 3]`, `arr[0]`, `arr[i] = v` (heap buffers, element-type tracking)
- **String interpolation**: `"hello ${name}!"` — parsed via sub-lexer/parser into StringInterpExpr
- **Compound assignment**: `+=`, `-=`, `*=`, `/=` desugared in the parser; `let mut` sugar for `mut`
- **Test runner (`engc test <dir>`)**: recursive `.ec` discovery, compile+run per file, PASS/FAIL summary
- **Integration test suite**: 6 programs (arith, control, struct, enum, string, fn/recursion) — all passing
- **Float printing** via `eng_float_to_str` runtime helper; bool as `true`/`false`

### Fixed
- LLVM IR: parenthesized GEP constant-expressions replaced with direct global `ptr` refs (opaque pointers)
- LLVM IR: bare non-void calls (printf in assert path) now assign result registers — implicit numbering no longer clashes with `%N` registers
- `@.fmt.float` size mismatch ([6 x i8] → [5 x i8])
- Missing i32↔i64 conversions in `materialize` (sext/trunc)
- Type checker false positives: for-loop variable scope; enum variant access (`Color.Red`) flagged E1001
- Array-typed slots declared as `i32` instead of `ptr` (inferTypeOf now handles ArrayLit)

### Changed
- `engc run` executes the produced binary after compiling and reports its exit code
- IR file kept on disk when clang fails (debugging aid)
- C++ unit tests misnamed `tests/test_*.ec` renamed to `tests/test_*.cpp`

## [0.1.0] — 2026-08-18 — M0 Bootstrap

### Added
- **Lexer**: Full token system with integer, float, string, char literals; keywords; operators; identifiers
- **Parser**: Recursive descent parser supporting functions, structs, enums, if/for/while loops, expressions
- **AST**: Complete node hierarchy for expressions (Literal, Ident, BinaryOp, UnaryOp, Call, FieldAccess, ArrayLit, StringInterp, TypeCast) and statements (Let, Mut, If, For, While, Return, Break, Continue, Block, FunctionDecl, StructDecl, EnumDecl, Impl, Import, Const)
- **Semantic Analyzer**: Symbol table management, function scope tracking, variable declaration analysis
- **Type Checker**: Basic type inference, variable lookup, undefined variable detection
- **LLVM Code Generator**: Generates LLVM IR for x86_64 targeting Windows/Linux/macOS
- **CLI Driver (`engc`)**: Commands: `version`, `new`, `run`, `build`, `test`, `fmt`, `lint`, `doc`, `help`
- **Diagnostic Engine**: Structured error/warning messages with codes and line/column information

### Fixed
- None (initial release)

### Changed
- Initial M0 release — bootstrapping phase

---

## [0.2.0] — Planned
- Full Option<T> and Result<T,E> support
- Struct field access and construction
- Method chaining
- String interpolation improvements
- Improved error messages

---

## [0.3.0] — Planned
- Module system with imports
- Standard library: io, fs, math, string, json
- Package manager: engc add/remove/update
- Better error recovery in parser

---

## [0.4.0] — Planned
- Async/await syntax
- Concurrency primitives: spawn, channels, mutex
- FFI: C/C++ interoperability
- Test framework with assertions

---

## [0.5.0] — Planned
- LSP server
- Debugger integration
- WASM target
- Cross-compilation support

---

## [1.0.0] — Target
- Full standard library
- Package registry
- Production-ready tooling
- Comprehensive documentation
- Community ecosystem

## v0.2.1 — OS Development Support (Bare-metal Target)

### New Features
- **Bare-metal x86_64 target** — Build OS kernels without any OS dependency
- **Custom linker script** — Flat memory layout (.text → .rodata → .data → .bss)
- **Raw binary output** — Extract .bin from ELF via llvm-objcopy
- **QEMU compatible** — Boot directly with `qemu-system-x86_64 -kernel`
- **Documentation** — Full OS development guide in `docs/OS_DEVELOPMENT.md`

### Files Added
- `compiler/targets/baremetal.{h,cpp}` — New bare-metal backend
- `docs/OS_DEVELOPMENT.md` — OS development guide

### CLI Usage
```bash
# Build kernel
engc build --target baremetal kernel.ec kernel.bin

# Or with explicit paths
engc build --target baremetal --src kernel.ec --output kernel.bin

# Run in QEMU
qemu-system-x86_64 -kernel kernel.bin
```

### Requirements
- clang/clang++ (LLVM compiler)
- llvm-objcopy (for binary extraction)
- QEMU (for testing)

### Limitations
- String operations use C library (will be replaced with custom allocators)
- No inline assembly support yet (coming in future versions)
- No hardware drivers (VGA, keyboard, etc.)

### Roadmap
- [ ] Inline assembly support
- [ ] Native VGA driver
- [ ] GDT/IDT setup
- [ ] Memory management (paging)
- [ ] Interrupt handling
- [ ] Process scheduler
- [ ] Shell/REPL
## [0.2.1] - 2025-07-24

### Added
- **Testing Framework**: `test`, `assert`, and `expect` keywords for unit testing
- **Coroutine Support**: `async`/`await` syntax with coroutine declarations
- **Hot Reload**: Runtime code reloading with `hotreload` statement
- **Pattern Matching Expressions**: Advanced pattern matching with `match` expressions
- **Macro System**: Function-like macros with `macro` declaration
- **Dimensional Types**: Physics-aware types with `dim<T,N>` syntax
- **Design by Contract**: Pre/post conditions with `requires` and `ensures`
- **Module System**: Full module support with `module`, `import`, and `export`
- **Exception Handling**: Try/catch blocks with automatic resource cleanup
- **Bytecode VM**: Assembly-like virtual machine for portable execution
- **AST Optimizer**: Dead code elimination, constant folding, loop unrolling
- **Incremental Builds**: Cache-aware compilation with automatic invalidation
- **CMake Integration**: Native CMake build system support

### Fixed
- Fixed duplicate test/assert/expect case values in parser
- Fixed token move assignment for std::remove_if compatibility
- Fixed undefined parseParamList symbol
- Fixed parser.h structural issues (duplicate closing braces)
- Fixed MatchExpr/MatchArm type definitions

### Build Status
- All 65 source files compile successfully
- Linker errors resolved
- Binary builds correctly

### VS Code Extensions (v0.3.4)
- **Updated syntax highlighting** to include all new keywords: `async`, `await`, `coroutine`, `test`, `assert`, `expect`, `macro`, `match`, `try`, `catch`, `throw`, `requires`, `ensures`, `dim`, `vec3`, `vec2`
- **Added code snippets** for common patterns: hello world, functions, structs, enums, loops, match expressions, tests, async functions, macros, try-catch
- **Updated tasks.json** with new commands: run, lint, test, doc
- **Updated launch.json** with better debug configurations
- **Updated extension manifest** to version 0.3.4
- **Created VSIX package** for easy installation in VS Code
