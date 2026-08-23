# EnginotechC++ — Design Decision Records

## ADR-0001: Compiler Implementation Language

**Date:** 2026-08-18
**Status:** Accepted
**Context:** Need to choose implementation language for the compiler
**Decision:** Use C++20 with LLVM for code generation
**Reason:** 
- LLVM provides mature, well-tested code generation infrastructure
- C++ offers zero-cost abstractions and control over memory layout
- The project targets systems programming; C++ is the natural choice
- LLVM integration is seamless via C++ API
**Consequences:**
- Requires LLVM dependency for compilation
- Enables native binary output
- Allows future JIT compilation and WASM generation

## ADR-0002: Memory Model

**Date:** 2026-08-18
**Status:** Accepted
**Context:** Determine memory management strategy
**Decision:** Reference counting with manual lifetime hints (borrow checker deferred to M2)
**Reason:**
- Simpler than Rust's ownership model
- More predictable than garbage collection
- Suitable for systems programming
**Consequences:**
- No automatic borrowing at M0
- Manual `ref`/`unref` for complex cases
- Borrow checker planned for M2 after core stability

## ADR-0003: Type System

**Date:** 2026-08-18
**Status:** Accepted
**Context:** Choose type system approach
**Decision:** Static typing with type inference (Hindley-Milner style)
**Reason:**
- Balances safety and ergonomics
- Inference reduces boilerplate
- Clear error messages when inference fails
**Consequences:**
- All variables have explicit or inferred types
- Generics use monomorphization
- No runtime type checking overhead

## ADR-0004: Error Handling

**Date:** 2026-08-18
**Status:** Accepted
**Context:** How to handle errors in EC
**Decision:** Result<T, E> and Option<T> with `?` operator
**Reason:**
- Similar to Rust/Go approaches
- Explicit error propagation
- No exceptions or silent failures
**Consequences:**
- Every fallible function returns Result
- `?` unwraps or propagates error
- Panic possible via `unwrap()` for testing

## ADR-0005: Module System

**Date:** 2026-08-18
**Status:** Accepted
**Context:** Organize code into modules
**Decision:** File-based modules with explicit imports
**Reason:**
- Simple mental model
- No hidden dependencies
- Predictable import resolution
**Consequences:**
- Each file is a module
- `import http;` loads from std or packages
- Public API via `pub` keyword

## ADR-0006: Package Manager

**Date:** 2026-08-18
**Status:** Accepted
**Context:** Dependency management
**Decision:** Centralized registry with lockfile
**Reason:**
- Similar to Cargo/npm models
- Reproducible builds
- Easy dependency resolution
**Consequences:**
- `engc add <pkg>` fetches from registry
- `engc.lock` ensures reproducibility
- Semantic versioning enforced

## ADR-0007: Standard Library Design

**Date:** 2026-08-18
**Status:** Accepted
**Context:** What to include in std lib
**Decision:** Minimal core + incremental expansion
**Reason:**
- Start small, prove value
- Avoid feature creep
- Let ecosystem grow organically
**Consequences:**
- M0: io, string, collections, math
- M1+: fs, net, json, http, async
- M2+: crypto, testing, regex

## ADR-0008: Testing Strategy

**Date:** 2026-08-18
**Status:** Accepted
**Context:** How to test the compiler
**Decision:** Integration tests via EC programs
**Reason:**
- Tests EC code in EC
- Validates end-to-end behavior
- Catches regressions early
**Consequences:**
- Test suite written in EC
- `engc test` runs all test programs
- Regression tests for each bug fix

## ADR-0009: LSP Support

**Date:** 2026-08-18
**Status:** Deferred
**Context:** IDE integration
**Decision:** Implement in M4
**Reason:**
- Core language must stabilize first
- LSP requires stable AST and type system
**Consequences:**
- No IDE support in M0-M3
- Language Server Protocol implemented in M4
- Works with VSCode, Neovim, Emacs

## ADR-0010: Debugging Support

**Date:** 2026-08-18
**Status:** Deferred
**Context:** Debugging capabilities
**Decision:** Integrate with DAP (Debug Adapter Protocol) in M5
**Reason:**
- Building custom debugger is complex
- DAP enables VSCode/Neovim integration
**Consequences:**
- Breakpoints via `engc debug`
- Step-through execution
- Variable inspection
