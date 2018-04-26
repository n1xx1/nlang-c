// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

int print_assert(const char* expr, const char* file, i32 line) {
	printf("\nAssertion failed!\nFile: %s, Line: %d\nExpression: %s\n", file, line, expr);
	return 0;
}

#define assert(expr) do { if(!(expr)) { print_assert(#expr, __FILE__, __LINE__); __asm__("int3"); } } while(0)
