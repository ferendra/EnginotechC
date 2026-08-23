# ETC Script Target Documentation

## Overview
The ETC (Enginotech Script) target provides a simplified scripting language for quick tasks like printing output, reading input, and basic operations.

## File Extension
`.etc` files are processed by the ETC skeleton target.

## Syntax

### Print Statement
```etc
print("Hello World")
output "Hello World"
say "Hello World"
```

### Multiple Lines
```etc
print("Line 1")
print("Line 2")
print("Line 3")
```

### Comments
```etc
# This is a comment
print("Hello")  # Inline comment
```

### String Literals
Strings are enclosed in double quotes:
```etc
print("Text here")
print("Numbers: 42")
```

## Token Types
The lexer recognizes the following tokens for ETC scripts:
- `PRINT` - The `print` keyword
- `OUTPUT` - The `output` keyword (alias for print)
- `SAY` - The `say` keyword (alias for print)
- `INPUT` - The `input` keyword (for reading stdin)
- `LPAREN` - Opening parenthesis `(`
- `RPAREN` - Closing parenthesis `)`
- `STRING_LITERAL` - String content in quotes
- `COMMENT` - Comments starting with `#`
- `NEWLINE` - Line breaks
- `EOF` - End of file

## Running ETC Files

### Using Standalone Test
```bash
./test_etc_standalone examples/ets/skeleton_demo.etc
```

### Using Compiler Driver (future)
```bash
engc run examples/file.etc
```

## Example Output
For the demo file `skeleton_demo.etc`:
```
=== Token Summary ===
Total tokens: 33
Tokens by type:
  COMMENT: 4
  PRINT: 6
  LPAREN: 6
  STRING: 6
  RPAREN: 6
  NEWLINE: 7
  EOF: 1

--- Execution ---
Hello from ETC!
This is a simple script
Testing output command
Greeting from say command
Line 1
Line 2
Line 3
The answer is: 42
```

## Implementation Files
- `compiler/targets/etskeleton.h` - Header with class definition
- `compiler/targets/etskeleton.cpp` - Implementation
- `examples/ets/skeleton_demo.etc` - Demo script

## Future Enhancements
- Support for variables
- Support for arithmetic expressions
- Support for input reading
- Support for control flow (if/else, loops)
- Integration with full compiler pipeline
