// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

// Stretchy Buffers implementation, based on 
// https://github.com/nothings/stb/blob/master/stretchy_buffer.h
// Usage:
//   i32* vec = NULL;
//   buf_push(vec, 10);
//   buf_puuh(vec, 20);
//   buf_push(vec, 30);
//   i32 a1 = vec[1]; // v == 20
//   isize size = buf_len(vec); // size == 3

#define buf__raw(b) ((isize*)(b) - 2)
#define buf__cap(b) buf__raw(b)[0]
#define buf__len(b) buf__raw(b)[1]

#define buf_cap(b) ((b) ? buf__cap(b) : 0)
#define buf_len(b) ((b) ? buf__len(b) : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__raw(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__len(b)++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_vprintf(b, ...) ((b) = buf__vprintf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__len(b) = 0 : 0)
#define buf_copy(b1, b2) (buf_fit(b2, buf_len(b1)), buf__copy(b1, b2, sizeof(*(b1)), sizeof(*(b2))))

void* buf__grow(const void *buf, isize new_len, isize elem_size) {
	isize new_cap = MAX(16, MAX(1 + 2 * buf_cap(buf), new_len));
	isize new_size = sizeof(isize) * 2 + new_cap * elem_size;
	isize* new_hdr = xrealloc(buf ? buf__raw(buf) : 0, new_size);
	if(!buf)
		new_hdr[1] = 0;
	new_hdr[0] = new_cap;
	return new_hdr + 2;
}
void buf__copy(void* buf1, void* buf2, size_t elem_size1, size_t elem_size2) {
	assert(elem_size1 == elem_size2);
	isize* hdr = buf__raw(buf2);
	size_t new_len = buf_len(buf1);
	hdr[1] = new_len;
	if(new_len > 0)
		memcpy(buf2, buf1, new_len * elem_size1);
}
char* buf__vprintf(char* buf, const char *fmt, va_list args) {
	va_list args1;
	va_copy(args1, args);
	
	size_t cap = buf_cap(buf) - buf_len(buf);
	size_t n = 1 + vsnprintf(NULL, 0, fmt, args1);
	va_end(args1);
	
	if(n > cap) {
		buf_fit(buf, n + buf_len(buf));
		cap = buf_cap(buf) - buf_len(buf);
	}
	buf__len(buf) += vsnprintf(buf_end(buf), cap, fmt, args);
	va_end(args);
	
	return buf;
}
char *buf__printf(char *buf, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	return buf__vprintf(buf, fmt, args);
}
