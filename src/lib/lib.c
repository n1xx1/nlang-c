// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

#include "common.c"
#include "assert.c"
#include "memory.c"
#include "buffers.c"
#include "pool.c"
#include "strings.c"

void fatal(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("FATAL: ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
	exit(1);
}

StrRange read_file(const char* name) {
	char* contents_str = NULL;
	isize contents_len = 0;
	FILE* file;
	fopen_s(&file, name, "r");
	isize nread;
	char buf[256];
	while((nread = fread(buf, 1, sizeof(buf), file)) > 0) {
		contents_str = realloc(contents_str, contents_len + nread);
		memcpy(contents_str + contents_len, buf, nread);
		contents_len += nread;
	}
	fclose(file);
	return (StrRange){ contents_str, contents_len };
}

void write_file(const char* name, StrRange out) {
	FILE* file;
	fopen_s(&file, name, "w");
	isize off = 0;
	isize nwrote = 0;
	while(off < out.l) {
		nwrote = fwrite(out.s + off, 1, MIN(out.l - off, 1024), file);
		off += nwrote;
		if(nwrote == 0)
			return;
	}
	fclose(file);
}