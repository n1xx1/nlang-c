
typedef enum TokenKind {
	T_UNKNOWN,
	#define TOKEN(n, v) n,
	#include "tokens.inc.h"
} TokenKind;

const char* token_kind_names[] = {
	#define TOKEN(n, v) [n] = v,
	#include "tokens.inc.h"
	[T_UNKNOWN] = "unknown"
};

int token_operator_prec[] = {
	#define TOKEN_OP(n, v, p) [n] = p,
	#include "tokens.inc.h"
	[T_AS] = 3
};

typedef struct Token {
	TokenKind tok;
	i32 line;
	i32 col;
	StrRange lit;
} Token;

const char* ttos(Token t) {
	switch(t.tok) {
		case T_IDENT:
		case T_SEMI:
			return strf("%.*s", (int)t.lit.l, t.lit.s);
		case T_INT:
		case T_FLOAT:
		case T_STRING:
			return strf("literal %.*s", (int)t.lit.l, t.lit.s);
		default:
			return token_kind_names[t.tok];
	}
}

