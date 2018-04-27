#ifndef TOKEN
#define TOKEN(n, v) 
#endif
#ifndef TOKEN_OP
#define TOKEN_OP(n, v, p) TOKEN(n, v)
#endif
#ifndef TOKEN_KW
#define TOKEN_KW(n, v) TOKEN(n, v)
#endif

TOKEN(T_UNKNOWN, "unknown")
TOKEN(T_EOF, "eof")

TOKEN(T_IDENT, "ident")
TOKEN(T_INT, "int")
TOKEN(T_FLOAT, "float")
TOKEN(T_STRING, "string")
TOKEN(T_CHAR, "char")

TOKEN(T_SEMI, ";")
TOKEN(T_COMMA, ",")
TOKEN(T_COLON, ":")
TOKEN(T_DCOLON, "::")
TOKEN(T_DOT, ".")
TOKEN(T_DOTDOTDOT, "...")
TOKEN(T_ARROW, "->")
TOKEN(T_LPAREN, "(")
TOKEN(T_LBRACK, "[")
TOKEN(T_LBRACE, "{")
TOKEN(T_RPAREN, ")")
TOKEN(T_RBRACK, "]")
TOKEN(T_RBRACE, "}")

// binary ops
TOKEN_OP(T_LOR, "||", 1)

TOKEN_OP(T_LAND, "&&", 2)

TOKEN_OP(T_EQL, "==", 3)
TOKEN_OP(T_NEQ, "!=", 3)
TOKEN_OP(T_LT, "<", 3)
TOKEN_OP(T_GT, ">", 3)
TOKEN_OP(T_LTE, "<=", 3)
TOKEN_OP(T_GTE, ">=", 3)

TOKEN_OP(T_ADD, "+", 4)
TOKEN_OP(T_SUB, "-", 4)
TOKEN_OP(T_OR, "|", 4)
TOKEN_OP(T_XOR, "^", 4)

TOKEN_OP(T_MUL, "*", 5)
TOKEN_OP(T_DIV, "/", 5)
TOKEN_OP(T_REM, "%", 5)
TOKEN_OP(T_AND, "&", 5)
TOKEN_OP(T_LSHIFT, "<<", 5)
TOKEN_OP(T_RSHIFT, ">>", 5)

// unary ops
// T_ADD
// T_SUB
// T_MUL
// T_AND
TOKEN(T_NOT, "!")
TOKEN(T_INC, "++")
TOKEN(T_DEC, "--")

// assign ops
TOKEN(T_ASSIGN, "=")
TOKEN(T_ADD_ASSIGN, "+=")
TOKEN(T_SUB_ASSIGN, "-=")
TOKEN(T_MUL_ASSIGN, "*=")
TOKEN(T_DIV_ASSIGN, "/=")
TOKEN(T_REM_ASSIGN, "%=")
TOKEN(T_AND_ASSIGN, "&=")
TOKEN(T_OR_ASSIGN, "|=")
TOKEN(T_XOR_ASSIGN, "^=")
TOKEN(T_LSHIFT_ASSIGN, "<<=")
TOKEN(T_RSHIFT_ASSIGN, ">>=")

TOKEN_KW(T_AS, "as")
TOKEN_KW(T_FN, "fn")
TOKEN_KW(T_IF, "if")
TOKEN_KW(T_LET, "let")
TOKEN_KW(T_FOR, "for")
TOKEN_KW(T_ELSE, "else")
TOKEN_KW(T_ENUM, "enum")
TOKEN_KW(T_TYPE, "type")
TOKEN_KW(T_CONST, "const")
TOKEN_KW(T_BREAK, "break")
TOKEN_KW(T_STRUCT, "struct")
TOKEN_KW(T_EXTERN, "extern")
TOKEN_KW(T_RETURN, "return")
TOKEN_KW(T_CONTINUE, "continue")

#undef TOKEN 
#undef TOKEN_OP
#undef TOKEN_KW
