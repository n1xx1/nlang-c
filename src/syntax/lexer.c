// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct Lexer {
	StrRange src;     // file contents
	const char* file; // file name
	i32 r0, r;        // last and current read character
	i32 line0, line;  // last and current line index
	i32 col0, col;    // last and current column index
	bool nlsemi;      // should insert a semi at the end
	Token token;
} Lexer;

void lexer_init(Lexer* l, const char* file, StrRange src) {
	l->src = src;
	l->file = file ? file : "<source>";
	l->r0 = l->r = 0;
	l->line0 = l->line = 1;
	l->col0 = l->col = 1;
	l->nlsemi = 0;
	l->token.tok = T_UNKNOWN;
	l->token.line = l->token.col = 0;
}

i32 lexer_getr(Lexer* l) {
	l->r0 = l->r;
	l->line0 = l->line;
	l->col0 = l->col;

	if(l->r >= l->src.l) return -1;

	i32 b = l->src.s[l->r];
	if(b < 0x80) {
		//printf("getr %c\n", b);
		// ascii character
		l->r++;
		l->col++;
		if(b == 0)
			fatal("invalid NULL character");
		if(b == '\n') {
			l->line++;
			l->col = 1;
		}
		return b;
	}
	fatal("UTF8 not implemented");
	return -1;
}
void lexer_ungetr(Lexer* l) {
	l->r = l->r0;
	l->line = l->line0;
	l->col = l->col0;
}
void lexer_ungetr2(Lexer* l) {
	lexer_ungetr(l);
	l->r0--;
	l->col0--;
}
void lexer_lex_number(Lexer* l, i32 c) {
	const char* b = l->src.s + l->r0;
	if(c != '.') {
		l->token.tok = T_INT;
		if(c == '0') {
			c = lexer_getr(l);
			if(c == 'x' || c == 'X') {
				// hex
				c = lexer_getr(l);
				int hasDigit = 0;
				while(('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) {
					c = lexer_getr(l);
					hasDigit = 1;
				}
				if(!hasDigit)
					fatal("malformed hex constant");
				
				goto done;
			}
			int has8or9 = 0;
			while('0' <= c && c <= '9') {
				if(c > '7') has8or9 = 1;
				c = lexer_getr(l);
			}
			if(c != '.' && c != 'e' && c != 'E') {
				// octal
				if(has8or9)
					fatal("malformed oct constant");
				goto done;
			}
		} else {
			while('0' <= c && c <= '9')
				c = lexer_getr(l);
		}
	}
	if(c == '.') {
		l->token.tok = T_FLOAT;
		c = lexer_getr(l);
		while('0' <= c && c <= '9')
			c = lexer_getr(l);
	}
done:
	lexer_ungetr(l);
	l->token.lit = string_range(b, l->src.s + l->r0);
	l->nlsemi = 1;
}

int lexer_lex_escape(Lexer* l, char quote) {
	int n;
	u32 base, max;

	i32 c = lexer_getr(l);
	switch(c) {
		case 'a': case 'b': case 'f': case 'n': case 'r': case 't': case 'v': case '\\':
			return 1;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
			n = 3;
			base = 8;
			max = 255;
			break;
		case 'x':
			c = lexer_getr(l);
			n = 2;
			base = 16;
			max = 255;
			break;
		case 'u':
			c = lexer_getr(l);
			n = 4;
			base = 16;
			max = 0x0010FFFF;
			break;
		case 'U':
			c = lexer_getr(l);
			n = 8;
			base = 16;
			max = 0x0010FFFF;
			break;
		default:
			if(c < 0) return 1;
			fatal("unknown escape sequence");
			return 0;
	}
	u32 x = 0;
	for(int i = n; i > 0; i--) {
		u32 d = base;
		if('0' <= c && c <= '9')
			d = c - '0';
		else if('a' <= c && c <= 'f')
			d = c - 'a' + 10;
		else if('A' <= c && c <= 'F')
			d = c - 'A' + 10;
		
		if(d >= base) {
			if(c < 0) return 1;
			fatal("non-%s character in escape sequence: %c", base == 8 ? "octal" : "hex", c);
			lexer_ungetr(l);
			return 0;
		}
		x = x*base + d;
		c = lexer_getr(l);
	}
	lexer_ungetr(l);
	if(x > max && base == 8) {
		fatal("octal escape value > 255: %d", x);
		return 0;
	}
	if(x > max || (0xD800 <= x && x < 0xE00)) {
		fatal("escape sequence is invalid Unicode point");
		return 0;
	}
	return 1;
}

void lexer_lex_char(Lexer* l) {
	const char* b = l->src.s + l->r0;
	int n = 0;
	while(1) {
		i32 c = lexer_getr(l);
		if(c == '\'') {
			break;
		}
		if(c == '\\') {
			lexer_lex_escape(l, '\'');
			continue;
		}
		if(c == '\n') {
			lexer_ungetr(l);
			fatal("newline in character literal");
			break;
		}
		if(c < 0) {
			fatal("invalid character literal");
			break;
		}
		n++;
	}
	if(n == 0)
		fatal("empty character literal");
	else if(n != 1)
		fatal("invalid character literal");
	
	l->nlsemi = 1;
	l->token.tok = T_CHAR;
	l->token.lit = string_range(b, l->src.s + l->r);
}

void lexer_lex_string(Lexer* l) {
	const char* b = l->src.s + l->r0;
	while(1) {
		i32 c = lexer_getr(l);
		if(c == '"') {
			break;
		}
		if(c == '\\') {
			lexer_lex_escape(l, '"');
			continue;
		}
		if(c == '\n') {
			lexer_ungetr(l);
			fatal("newline in string");
			break;
		}
		if(c < 0) {
			fatal("string not termintaed");
		}
	}
	l->nlsemi = 1;
	l->token.tok = T_STRING;
	l->token.lit = string_range(b, l->src.s + l->r);
}

void lexer_lex_ident(Lexer* l) {
	const char* b = l->src.s + l->r0;
	i32 c = lexer_getr(l);
	while(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || ('0' <= c && c <= '9'))
		c = lexer_getr(l);
	
	if(c >= 0x80)
		fatal("UTF8 not implemented");
	
	lexer_ungetr(l);
	StrRange lit = string_range(b, l->src.s + l->r0);
	l->token.tok = T_IDENT;
	l->token.lit = lit;

	if(0);
#define TOKEN_KW(n, v) \
	else if(lit.l == sizeof(v) - 1 && memcmp(lit.s, v, sizeof(v) - 1) == 0) {\
		l->token.tok = n;\
		l->nlsemi = n == T_BREAK || n == T_CONTINUE || n == T_RETURN;\
	}
#include "tokens.inc.h"
	else {
		l->nlsemi = 1;
	}
}


void lexer_lex_full_comment(Lexer* l) {
	i32 c = lexer_getr(l);
	while(c >= 0) {
		if(c == '*') {
			c = lexer_getr(l);
			if(c == '/') 
				return;
		}
		c = lexer_getr(l);
	}
	fatal("comment not terminated");
}

void lexer_lex_line_comment(Lexer* l) {
	i32 c = lexer_getr(l);
	while(c >= 0) {
		if(c == '\n') {
			lexer_ungetr(l);
			break;
		}
		c = lexer_getr(l);
	}
}


#define LEX_CASE1(c1, t1, c2, t2) case c1:\
	c = lexer_getr(l);\
	if(c == c2) { l->token.tok = t2; }\
	else { lexer_ungetr(l); l->token.tok = t1; }\
	break;

#define LEX_CASE2(c1, t1, c21, t21, c22, t22) case c1:\
	c = lexer_getr(l);\
	if(c == c21) { l->token.tok = t21; }\
	else if(c == c22) { l->token.tok = t22; }\
	else { lexer_ungetr(l); l->token.tok = t1; }\
	break;

#define LEX_CASE3a(c1, t1, c21, t21, c22, t22, c23, t23) case c1:\
	c = lexer_getr(l);\
	if(c == c21) { l->token.tok = t21; }\
	else if(c == c22) { l->token.tok = t22; }\
	else if(c == c23) { l->token.tok = t23; }\
	else { lexer_ungetr(l); l->token.tok = t1; }\
	break;

#define LEX_CASE3(c1, t1, c21, t21, c22, t22, c3, t3) case c1:\
	c = lexer_getr(l);\
	if(c == c21) { l->token.tok = t21; }\
	else if(c == c22) {\
		c = lexer_getr(l);\
		if(c == c3) { l->token.tok = t3; }\
		else { lexer_ungetr(l); l->token.tok = t22; }\
	} else { lexer_ungetr(l); l->token.tok = t1; }\
	break;

void lexer_lex(Lexer* l) {
	i32 c;
	int nlsemi = l->nlsemi;
	l->nlsemi = 0;
redo:
	c = lexer_getr(l);
	while(c == ' ' || c == '\t' || (c == '\n' && !nlsemi) || c == '\r')
		c = lexer_getr(l);
	
	l->token.line = l->line0;
	l->token.col = l->col0;

	if(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') {
		lexer_lex_ident(l);
		return;
	}

	switch(c) {
		case -1:
			if(nlsemi) {
				l->token.lit = string_range_c("eof");
				l->token.tok = T_SEMI;
				break;
			}
			l->token.tok = T_EOF;
			break;
		case '\n':
			l->token.lit = string_range_c("newline");
			l->token.tok = T_SEMI;
			break;
		case '\'':
			lexer_lex_char(l);
			break;
		case '"':
			lexer_lex_string(l);
			break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
			lexer_lex_number(l, c);
			break;
		case '(':
			l->token.tok = T_LPAREN;
			break;
		case '[':
			l->token.tok = T_LBRACK;
			break;
		case '{':
			l->token.tok = T_LBRACE;
			break;
		case ')':
			l->token.tok = T_RPAREN;
			l->nlsemi = 1;
			break;
		case ']':
			l->token.tok = T_RBRACK;
			l->nlsemi = 1;
			break;
		case '}':
			l->token.tok = T_RBRACE;
			l->nlsemi = 1;
			break;
		case ';':
			l->token.lit = string_range_c("semicolon");
			l->token.tok = T_SEMI;
			break;
		case ':':
			l->token.tok = T_COLON;
			break;
		case '.':
			l->token.tok = T_DOT;
			break;
		case ',':
			l->token.tok = T_COMMA;
			break;
		case '/':
			c = lexer_getr(l);
			if(c == '/') {
				lexer_lex_line_comment(l);
				goto redo;
			}
			if(c == '*') {
				lexer_lex_full_comment(l);
				if(l->line > l->token.line && nlsemi) {
					l->token.tok = T_SEMI;
					l->token.lit = string_range_c("newline");
				}
				goto redo;
			}
			if(c == '=') {
				l->token.tok = T_DIV_ASSIGN;
			} else {
				lexer_ungetr(l);
				l->token.tok = T_DIV;
			}
			break;
		
		LEX_CASE2('+', T_ADD, '=', T_ADD_ASSIGN, '+', T_INC; l->nlsemi = 1)
		LEX_CASE3a('-', T_SUB, '=', T_SUB_ASSIGN, '>', T_ARROW, '-', T_DEC; l->nlsemi = 1)
		LEX_CASE1('*', T_MUL, '=', T_MUL_ASSIGN)        
		LEX_CASE1('%', T_REM, '=', T_REM_ASSIGN)
		LEX_CASE2('&', T_AND, '=', T_AND_ASSIGN, '&', T_LAND)
		LEX_CASE2('|', T_OR, '=', T_OR_ASSIGN, '|', T_LOR)
		LEX_CASE1('^', T_XOR, '=', T_XOR_ASSIGN)
		LEX_CASE3('<', T_LT, '=', T_LTE, '<', T_LSHIFT, '=', T_LSHIFT_ASSIGN)
		LEX_CASE3('>', T_GT, '=', T_GTE, '>', T_RSHIFT, '=', T_RSHIFT_ASSIGN)
		LEX_CASE1('=', T_ASSIGN, '=', T_EQL)
		LEX_CASE1('!', T_NOT, '=', T_NEQ)

		default:
			l->token.tok = T_UNKNOWN;
			fatal("invalid character %c", c);
	}
}