// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct StrRange {
	const char* s;
	isize l;
} StrRange;

typedef struct FileLoc {
	const char* file;
	i32 line;
	i32 col;
} FileLoc;


StrRange string_range_c(const char* s) {
	return (StrRange){ s, strlen(s) };
}
StrRange string_range_len(const char* s, size_t l) {
	return (StrRange){ s, l };
}
StrRange string_range(const char* s, const char* e) {
	return (StrRange){ s, e - s };
}
StrRange string_range_copy(StrRange s) {
	char* d = xmalloc(s.l + 1);
	memcpy(d, s.s, s.l);
	d[s.l] = '\0';
	return (StrRange){ d, s.l };
}

char *strf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	i32 n = 1 + vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	char *str = xmalloc(n);
	va_start(args, fmt);
	vsnprintf(str, n, fmt, args);
	va_end(args);
	return str;
}

typedef const char* StrIntern;

MemoryPool str_intern_pool;
StrRange* str_intern_list;

StrIntern str_intern(StrRange str) {
    for(isize i = 0; i < buf_len(str_intern_list); i++) {
        if(str_intern_list[i].l == str.l && strncmp(str_intern_list[i].s, str.s, str.l) == 0) {
            return str_intern_list[i].s;
        }
    }
    char *s = mpool_alloc(&str_intern_pool, str.l + 1);
    memcpy(s, str.s, str.l);
    s[str.l] = 0;
    buf_push(str_intern_list, string_range_len(s, str.l));
    return s;
}

StrIntern str_intern_c(const char *str) {
    return str_intern(string_range_c(str));
}