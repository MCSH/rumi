# Rumi

> Everything you possess of skill, and wealth, and handicraft, wasn't it first merely a thought and a quest? - Rumi

Rumi is a WIP compiler.

The goal is to have a language that is low level, has functional properties, can be linked with c, doesn't make local functions a nightmare, has a compile time language that is the same as the runtime language, and focuses on making programming joyful.

It is implemented with flex, bison and llvm.

# How to compile the compiler?

Install `llvm (>=8.0)`, `bison` and `flex` then run make.

# How to compile using the compiler?

Run `./rum test.rum` and then do `gcc test.rum.o`, your compiled file is available at `a.out`.

# How to debug?

As of now, `rumi` is just a proof of concept. There is almost no error reporting and every feature is experimental. If you ever ran into any problem, while coding in `rumi` or while developing it, try these:

## See if LLVM has anything to say.

Run `./rum test.rum > test.ll` followed by `lli test.ll`.

## See what gdb has to say

Run `gdb rum` followed by `run test.rum`. You can try the following commands:

* `break classname::methodname`
* `break filename lineno`
* `bt` (back trace)
* `print EXPR` (prints value of EXPR)

# Current Language Features:

* Variables
* Functions
* Expressions
* Varargs
* String literals
* if else
* while
* structs
* pointers

# Current supported types:
* Arch default int
* Strings: string
* Structs
* signed and unsigned integers of 8, 16, 32, 64
  u8, u16, u32, u64
  s8, s16, s32, s64
* floats of 32 and 64 bits (float and double in c) f32, f64
* pointers!
* arrays

# Top Priority Language TODO list

* heap allocation / deallocation (malloc/free)
* better vararg support
* better array support

# Current Compiler TODO list:

* Add compiler flag support (`-o` for example)
* Allow multiple file compile at the same time
* Integrate linker into the compiler
