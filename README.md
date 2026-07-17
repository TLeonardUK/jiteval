![jiteval - single header expression evaluator](./resources/banner.png?raw=true)

![GitHub license](https://img.shields.io/github/license/TLeonardUK/jiteval)
![GitHub release](https://img.shields.io/github/release/TLeonardUK/jiteval)
![GitHub downloads](https://img.shields.io/github/downloads/TLeonardUK/jiteval/total)

# What is this project?
JitEval is a single-header library written in C, that provides support for evaluating arbitrary C-style expressions quickly and easily. It uses no external dependencies and is licensed under MIT. It optionally supports just-in-time (JIT) compiling the expressions to machine code in situations where the speed is critical.

It was originally designed for real-time, performance critical, applications such as video games.

# Features
- C11 with no dependencies.
- Single header files.
- Simple to use, while being very fast.
- JIT compiling for supported ISA's for maximum performance.
- Implements standard C operators and precedence.
- Custom variable and function support.
- No runtime malloc usage.
- Thread safe (using separate contexts per thread).
- MIT Licensed.

# What type of expressions are supported?
The library supports all standard C operators, along with function calls and variables. It uses strongly-typed, case-sensitive semantics (again like C).

Support data types are: int, float, string, bool.

For full syntax details see the comments at the top of jiteval.h

Some arbitrary examples of supported expressions:

```c
(MY_FLAG_A | MY_FLAG_B) & ~MY_FLAG_C
```
```c
sin(3.0f) * cos((b + c) / PI)
```
```c
my_function(a, b) < my_function(d)
```
```c
"Hello World: " + ((string)3.4)
```

# How can I use it?
You simple need to drop the header file in your project include it a source file somewhere with JITEVAL_IMPL defined before it, like so:

```c
#define JITEVAL_IMPL
#include <jiteval.h>
```

You can then use it anywhere in your codebase just by including the header file.

The simplest (but least efficient) way of evaluating an expression is using the je_eval_<type> functions, like so:

```c
int result = je_eval_int("(4 + 4) / 2", NULL, 0);
```

A more full-featured example is contained in sample.c

Extensive documentation of the API's available to you are located in the comments at the top of the jiteval.h

# Where can I download it?
The latest released version of the header file is available on the github releases page - https://github.com/TLeonardUK/jiteval/releases

# Whats the platform support?

| Platform | Compiler | ISA | Non-JIT Support |JIT Support |
| --- | --- | --- | --- | --- |
| Windows | MSVC | x86 | :heavy_check_mark: | :heavy_check_mark: |
| Windows | Clang | x86 | :heavy_check_mark: | :heavy_check_mark: |
| Windows | MSVC | x64 | :heavy_check_mark: | :heavy_check_mark: |
| Windows | Clang | x64 | :heavy_check_mark: | :heavy_check_mark: |
| Linux | Clang | x86 | :heavy_check_mark: | :heavy_check_mark: |
| Linux | GCC | x86 | :heavy_check_mark: | :heavy_check_mark: |
| Linux | Clang | x64 | :heavy_check_mark: | :heavy_check_mark: |
| Linux | GCC | x64 | :heavy_check_mark: | :heavy_check_mark: |
| All Others | - | - | :heavy_check_mark: | :x: |

# Whats in the repository?
```
/
├── resources/             General resources used for building and packaging - icons/readmes/etc.
├── projects/              Projects for different IDE's used for developing the project.
├── jiteval.h              The main header file containing the project.
├── sample.c               Simple example of using the library with explanatory comments.
├── benchmark.c            Simple benchmark script for JIT/No-JIT, main script used in IDE projects.
```

# How can I help?
Useful pull requests are happily accepted. 

The exception is minor PR's such as typo fixes as they are often done as drive-by PR's by people fluffing their github profiles. Open an issue for those.

No AI generated pull requests are accepted.
