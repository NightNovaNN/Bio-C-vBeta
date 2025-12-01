# BioC

BioC is a modern, developer-friendly language that transpiles directly into clean, portable ANSI C.  
It provides expressive syntax, built-in utilities, and high-level features while preserving the performance, clarity, and compatibility of traditional C.

BioC acts as a “TypeScript for C”:  
you write readable BioC code, and the compiler emits predictable plain C.

---

## Features

### Modern, Clean Syntax
BioC improves C’s readability with a simple, expressive grammar.

```bioc
int add(int a, int b) -> int {
    return a + b;
}

var x: int = 5;
var y: int = 7;

echo(add(x, y));
````

---

## String Utilities

BioC ships with safe, high-level string helpers:

* `strip(str)`
* `lower(str)`
* `upper(str)`
* `startswith(str, prefix)`
* `endswith(str, suffix)`
* `replace(str, from, to)`
* `split(str, delimiter, &count)` → returns `string*` (`char**` in C)

Example:

```bioc
var s: string = "Hello BioC   ";
echo(strip(s));

var yes: bool = startswith("anish", "an");
var no: bool = endswith("hello", "xyz");

var replaced: string = replace("hello world", "world", "BioC");
echo(replaced);
```

---

## Split Function (char**)

`split` returns a pointer to a dynamically allocated list of strings.

```bioc
var count: int = 0;
var parts: string* = split("a,b,c", ',', &count);

echo(count);

inline_c {
    printf("parts[0] = %s\n", parts[0]);
}
```

---

## Dynamic Arrays

BioC includes a small dynamic array type implemented in the BioC stdlib.

* `array_new()`
* `array_push(arr, value)`
* `array_len(arr)`
* `array_get(arr, index)`

Example:

```bioc
var arr: array = array_new();

inline_c {
    array_push(arr, "hello");
    array_push(arr, "world");
}

echo(array_len(arr));
```

---

## Ternary Operator

BioC supports C-style ternary expressions:

```bioc
var mx: int = (10 > 5) ? 10 : 5;
echo(mx);
```

---

## File Output Syntax

A dedicated DSL for writing to files:

```bioc
out[result.txt]: "BioC OK\n";
```

Transpiles to:

```c
{
    FILE *_fp = fopen("result.txt", "w");
    fprintf(_fp, "BioC OK\n");
    fclose(_fp);
}
```

---

## Inline C Support

Inline C blocks are copied directly into the generated C output.

```bioc
inline_c {
    printf("Inline C works inside BioC!\n");
}
```

---

## Zero Runtime Overhead

BioC does not add a VM, garbage collector, or heavy runtime.
All generated C code is clean, ANSI-compatible, and portable.

---

# Example Program (Alpha4-Lite)

### BioC Source

```bioc
var msg: string = "Hello BioC   ";
echo(strip(msg));

var parts_count: int = 0;
var parts: string* = split("a,b,c", ',', &parts_count);

echo(parts_count);

inline_c {
    printf("parts[0] = %s\n", parts[0]);
}

var arr: array = array_new();

inline_c {
    array_push(arr, "hello");
    array_push(arr, "world");
}

echo(array_len(arr));

int sum(int a, int b) -> int {
    return a + b;
}

echo(sum(5, 7));

var mx: int = (10 > 5) ? 10 : 5;
echo(mx);

out[result.txt]: "BioC OK\n";

inline_c {
    printf("Inline C OK!\n");
}

exit 0;
```

---

### Generated C Output (full)

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bioc_std.h"

/* --- BioC transpiled code --- */

int sum(int a, int b) {
    return a + b;
}

int main() {
    char* msg = "Hello BioC   ";
    printf("%s\n", strip(msg));

    int parts_count = 0;
    char** parts = split("a,b,c", ',', &parts_count);
    printf("%d\n", parts_count);

    /* inline C start */
    printf("parts[0] = %s\n", parts[0]);
    printf("parts[1] = %s\n", parts[1]);
    printf("parts[2] = %s\n", parts[2]);
    /* inline C end */

    array* arr = array_new();

    /* inline C start */
    array_push(arr, "hello");
    array_push(arr, "world");
    /* inline C end */

    printf("%d\n", array_len(arr));

    /* inline C start */
    printf("arr[0] = %s\n", (char*)array_get(arr, 0));
    printf("arr[1] = %s\n", (char*)array_get(arr, 1));
    /* inline C end */

    int z = sum(5, 7);
    printf("%d\n", z);

    int mx = (10 > 5) ? 10 : 5;
    printf("%d\n", mx);

    { 
        FILE *_fp = fopen("result.txt", "w"); 
        fprintf(_fp, "BioC OK\n"); 
        fclose(_fp); 
    }

    /* inline C start */
    printf("Inline C OK!\n");
    /* inline C end */

    return 0;
}
```

---
