# NLang C Compiler
C compiler for NLang, an experimental programming language

## Compilation
The compiler uses a single compilation unit; everything is included in the main.c file.

**Debug**
    
    gcc -Wall -Werror -Wno-format-zero-length -std=c11 -g -O0 -o bin/nc.exe src/main.c

**Release**

    gcc -Wall -Werror -Wno-format-zero-length -std=c11 -Os -o bin/nc.exe src/main.c
