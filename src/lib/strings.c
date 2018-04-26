
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