// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef enum TokenKind {
	#define TOKEN(n, v) n,
	#include "tokens.inc.h"
	TOKEN_MAX
} TokenKind;

const char* token_kind_names[] = {
	#define TOKEN(n, v) [n] = v,
	#include "tokens.inc.h"
	[TOKEN_MAX] = ""
};

int token_operator_prec[] = {
	#define TOKEN_OP(n, v, p) [n] = p,
	#include "tokens.inc.h"
	[T_AS] = 3,
	[TOKEN_MAX] = 0
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

