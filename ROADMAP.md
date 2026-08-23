# EnginotechC++ — Roadmap

## Current Phase: v0.2.x — FOUNDATION COMPLETE ✓

### Completed ✓ (v0.2.1)
- [x] Lexer with token system
- [x] Recursive descent parser
- [x] AST node hierarchy
- [x] Diagnostic engine (with source line + caret display)
- [x] Semantic analyzer
- [x] Type checker (with enum registration, type validation)
- [x] LLVM IR generator
- [x] CLI driver (`engc`)
- [x] CMake build system
- [x] End-to-end compilation: .ec → LLVM IR → native binary
- [x] Integration tests — 11/11 passing
- [x] Error-corpus tests — 5/5 failing cleanly
- [x] Library std.ec with self-test
- [x] Stage 1 self-hosting (lexer)
- [x] Benchmark compile
- [x] OS builtins: sleep, env_get, cwd, path_join, serial I/O
- [x] Multi-target support: native, Arduino, ESP32, AVR
- [x] Documentation: language reference, diagnostics, string contract, memory schema

### Working Examples
- [x] examples/hello — Basic I/O
- [x] examples/bench — Recursion + loops
- [x] examples/arithmetic — Type operations
- [x] examples/calculator — Functions
- [x] examples/os-demo — OS builtins
- [x] examples/gui/demo — GUI (with import)

### Known Limitations (Documented)
- ❌ Generics not supported (`vector<Todo>` fails)
- ❌ No multi-file import system yet
- ❌ No struct field access on instances
- ❌ No string slicing
- ❌ Runtime exit cleanup crash (pre-existing, doesn't affect correctness)

## Upcoming Phases

### Phase 1: Generics & Collections (2-3 weeks)
- [ ] Generic types: `vector<T>`, `map<K,V>`
- [ ] Type inference improvements
- [ ] Standard library expansion (collections, algorithms)

### Phase 2: Advanced OOP (2 weeks)
- [ ] Struct field access: `point.x`
- [ ] Method chaining: `obj.method().method()`
- [ ] Trait/interface system
- [ ] Constructor/des
- Full struct support with fields
- Method chaining: `obj.method().method()`
- Default methods
- Trait/interface system
- Borrow checking (Rust-inspired)

### M3: Standard Library (4 weeks)
- io module (print, println, read)
- fs module (read, write, path)
- string module (split, join, trim)
- collections module (Array, List, Map)
- math module
- testing module

### M4: Modules & Packages (3 weeks)
- Module system with imports
- Package manager (`engc add/remove/update`)
- Lockfile generation
- Registry integration

### M5: Concurrency (4 weeks)
- Async/await syntax
- spawn() for threads
- Channels for communication
- Mutex and atomics
- Futures and Promises

### M6: FFI & Interop (3 weeks)
- C function bindings
- C++ class wrapping
- Callback support
- Memory layout compatibility

### M7: Tooling (4 weeks)
- LSP server
- Debugger integration (DAP)
- Format improvements
- Profiling support

### M8: Performance (3 weeks)
- Benchmark suite
- Optimization passes
- WASM target
- Cross-compilation

### M9: Stabilization (4 weeks)
- API freeze
- Documentation complete
- Examples complete
- Security audit

### M10: v1.0.0 (2 weeks)
- Release candidate
- Community feedback
- Final bug fixes
- Launch

## Long-term Vision

### v2.0
- Macro system
- Generative programming
- Built-in web server framework
- GPU compute shaders

### v3.0
- Distributed compilation
- Cloud-native deployment tools
- AI-assisted coding (optional)

### Phase 5: Evaluator & Assessment (COMPLETE)
### Phase 6: Multi-Target Codegen (1 week)
### Phase 7: Package Manager + Language Server (2 weeks)
- [x] Package manager (already exists)
- [x] LSP server (JSON-RPC 2.0)
- [x] Completion, diagnostics, hover
- [x] `engc lsp` command
- [x] JavaScript transpiler (fix for-loop bug)
- [x] Python transpiler
- [x] WebAssembly text format emitter
- [x] EC Virtual Machine bytecode compiler + interpreter
- [x] 7/7 multi-target tests passing
- [x] Program evaluator framework (correctness, performance, quality, best practices)
- [x] Batch evaluation mode for all programs
- [x] Score reporting with detailed breakdown
- [x] Integration with test suite

## Achievement Summary

### v0.2.x Foundation (COMPLETE)
- ✅ Lexer, parser, AST, type checker, IR generator
- ✅ Multi-target: native, Arduino, ESP32, AVR
- ✅ 11/11 integration tests passing
- ✅ 5/5 error corpus tests clean
- ✅ Standard library + OS builtins
- ✅ Language reference (accurate, from actual behavior)
- ✅ Evaluator framework with scoring
- ✅ Benchmark baseline: fib(32)+loop100M in 0.013s

### Known Limitations (Documented)
- ❌ Generics not supported
- ❌ No multi-file import
- ❌ No struct field access on instances
- ❌ Runtime exit cleanup crash (pre-existing)
