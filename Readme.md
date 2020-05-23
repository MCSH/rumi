# Rumi

> Everything you possess of skill, and wealth, and handicraft, wasn't it first merely a thought and a quest? - Rumi

Rumi is a WIP compiler.

The goal is to have a language that is low level, has functional properties, can be linked with c, doesn't make local functions a nightmare, has a compile time language that is the same as the runtime language, and focuses on making programming joyful.

The current version is written in itself, you create a base compiler with C++, flex and bison, then run `self_compile.sh` so that the compiler compiles itself. The previously semi-complete version could be found in the `old` branch.
