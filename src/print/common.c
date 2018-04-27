// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef int (*p_printf_type)(const char* fmt, ...);
p_printf_type p_printf = printf;

char* p_printf_buffer;
int p_printf_to_buffer(const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
    buf_vprintf(p_printf_buffer, fmt, args);
    va_end(args);
    return 0;
}

#define PRINT_STRING_FUNC_IMPL(name, args1, args2) \
const char* string_##name args1 {\
    p_printf_type old_printf = p_printf;\
    p_printf = p_printf_to_buffer;\
    p_printf_buffer = NULL;\
    print_##name args2;\
    char* ret = memdup(p_printf_buffer, buf_len(p_printf_buffer) + 1);\
    ret[buf_len(p_printf_buffer)] = '\0';\
    buf_free(p_printf_buffer);\
    p_printf = old_printf;\
    return ret;\
}

#define PRINT_STRING_FUNC1(name, t1) PRINT_STRING_FUNC_IMPL(name, (t1 x1), (x1))
#define PRINT_STRING_FUNC2(name, t1, t2) PRINT_STRING_FUNC_IMPL(name, (t1 x1, t2 x2), (x1, x2))
#define PRINT_STRING_FUNC3(name, t1, t2, t3) PRINT_STRING_FUNC_IMPL(name, (t1 x1, t2 x2, t3 x3), (x1, x2, x3))

