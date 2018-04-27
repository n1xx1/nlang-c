// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

// include all the headers we will use across the project
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib/lib.c"
#include "syntax/syntax.c"
#include "print/print.c"

void main_compile_file(const char* name, StrRange file) {
	Lexer l;
	lexer_init(&l, name, file);

	while(true) {
		lexer_lex(&l);
		printf("[%2d:%-2d]  %s\n", l.token.line, l.token.col, ttos(l.token));
		if(l.token.tok == T_EOF)
			break;
	}
}

int main(int argc, const char* argv[]) {
	if(argc != 3) {
		printf("Usage: nc <file.nl> <out.c>\n");
		return 1;
	}

	StrRange contents = read_file(argv[1]);
	main_compile_file(argv[1], contents);
	return 0;
}

