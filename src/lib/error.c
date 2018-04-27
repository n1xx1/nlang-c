// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

void print_error_pos(FileLoc loc, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("%s:%d:%d: ", loc.file, loc.line, loc.col);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

#define resolve_error(loc, fmt, ...) (print_error_pos(loc, "resolve error: " fmt, ##__VA_ARGS__), exit(1))

#define resolve_warning(loc, fmt, ...) (print_error_pos(loc, "resolve warning: " fmt, ##__VA_ARGS__))

#define parser_error(fmt, ...) (print_error_pos(parser_loc(p), "parse error: " fmt, ##__VA_ARGS__), exit(1))

void fatal(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("FATAL: ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
	exit(1);
}
