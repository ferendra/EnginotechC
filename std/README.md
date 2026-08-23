# Std Library - EnginotechC++ Standard Library

Standard library modules for EnginotechC++.

## Modules

| Module | Description |
|--------|-------------|
| `io` | Input/output operations |
| `collections` | Vector, Map, Set data structures |
| `math` | Mathematical functions |
| `string` | String manipulation utilities |
| `json` | JSON parsing and serialization |
| `fs` | Filesystem operations |
| `testing` | Unit testing framework |

## Usage

```eng
use std::io;
use std::collections::Vector;
use std::math;

fn main() {
    let v = Vector<int>{};
    v.push(42);
    print(v.get(0));
}
```
