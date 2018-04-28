// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

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
#include "resolver/resolver.c"
#include "print/print.c"

typedef map_type(const char*, i32) MyMap;

void main_compile_file(const char* name, StrRange contents) {
	Parser p;
	parser_init(&p, name, contents);

	//AstFile* file = parser_parse_file(&p);
	//puts(string_ast_file(file));

	MyMap m = {0};
	map_set(&m, "test1", 1);
	map_set(&m, "test2", 2);

	for(MapIterator it = map_begin(&m); it != map_end(&m); map_next(&m, &it)) {
		const char* key = map_iter_key(&m, &it);
		i32* value = map_iter_value(&m, &it);
		printf("%s = %d\n", key, *value);
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

