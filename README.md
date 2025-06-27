# Bminor → C Translator

A lightweight **command-line compiler front-end** that converts programs written in the _Bminor_ teaching language into readable, vanilla C.

> **Why?**  
> Bminor is a tiny C-like language used in compiler courses (notably at the University of Wisconsin).  
> Translating Bminor to C lets you:
> 1.  run Bminor programs anywhere a C compiler exists, and  
> 2.  peek under the hood of basic compilation techniques—scanning, parsing, semantic checks, and code generation—without diving into full-blown LLVM.

---

## Key Features

| Capability | Details |
|------------|---------|
| **Type-aware symbol table** | Tracks `integer`, `boolean`, `character`, and `string` identifiers for smarter translation (e.g., correct `printf` format specifiers). |
| **Pretty C output** | Preserves indentation & comments, rewrites keywords (`True`/`False` → `true`/`false`), and converts `^` into `pow()` calls. |
| **Function support** | Parses Bminor `function` blocks (name, parameters, return type) and emits idiomatic C signatures. |
| **Rich `print` handling** | Builds a single `printf()` call with auto-generated format strings, handling literals & mixed arguments. |
| **Control flow** | Translates `if`, `while`, `return`, array declarations, and assignment statements. |
| **Zero external deps** | Pure C99—compiles with `gcc`, `clang`, or MSVC. |

---

## Build

```bash
# Clone
git clone https://github.com/<your-user>/bminor-to-c.git
cd bminor-to-c

# Compile (GCC example)
gcc -std=c99 -Wall -Wextra -o Parser Parser.c
```

---

Usage
```bash
./Parser <source.bminor> <output.c>
# then compile the generated C
gcc output.c -o program
./program
```
Example
```bminor
// fibonacci.bminor
n: integer = 10;

fib: function integer (x: integer) = {
    if (x < 2) {
        return x;
    };
    return fib(x - 1) + fib(x - 2);
};

print "fib(", n, ") = ", fib(n);
```
```bash
./Parser fibonacci.bminor fibonacci.c
gcc fibonacci.c -o fib
./fib
# -> fib(10) = 55
```
Generated snippet (truncated):
```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

int n = 10;

int fib(int x) {
    if (x < 2) {
        return x;
    }
    return fib(x - 1) + fib(x - 2);
}

int main() {
    printf("fib(%d) = %d\n", n, fib(n));
    return 0;
}
```

---

Project Structure
```
.
├── Parser.c         # Translator source (this repo)
├── examples/        # Sample Bminor programs
└── README.md
```

---

Current Limitations / TODO:

•	No full grammar verification—assumes well-formed Bminor input.

•	Arrays are treated as int only.

•	Functions returning non-primitive types are not yet supported.

•	No static or dynamic semantic checks (e.g., undefined variables) beyond a simple symbol table.

•	Power operator supports only integer exponentiation via (int)pow(a,b).

---
Author

Amir-Abbas Alvand
Feel free to connect on GitHub or reach out with feedback!
