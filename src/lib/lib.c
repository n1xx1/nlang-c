// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

#include "common.c"
#include "assert.c"
#include "memory.c"
#include "buffers.c"
#include "map.c"
#include "pool.c"
#include "strings.c"
#include "error.c"

StrRange read_file(const char* name) {
	char* contents_str = NULL;
	isize contents_len = 0;
	FILE* file;
	if(fopen_s(&file, name, "r") != 0) {
		fatal("cannot open input file \"%s\"", name);
		return string_range_c("");
	}
	isize nread;
	char buf[256];
	while((nread = fread(buf, 1, sizeof(buf), file)) > 0) {
		contents_str = realloc(contents_str, contents_len + nread);
		memcpy(contents_str + contents_len, buf, nread);
		contents_len += nread;
	}
	fclose(file);
	return string_range_len(contents_str, contents_len);
}

void write_file(const char* name, StrRange out) {
	FILE* file;
	if(fopen_s(&file, name, "w") != 0) {
		fatal("cannot open output file \"%s\"", name);
		return;
	}
	isize off = 0;
	isize nwrote = 0;
	while(off < out.l) {
		nwrote = fwrite(out.s + off, 1, MIN(out.l - off, 1024), file);
		off += nwrote;
		if(nwrote == 0)
			break;
	}
	fclose(file);
}