// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct Parser {
	Lexer l;
	Token t;
	int xnest; // expression nesting level
} Parser;

FileLoc parser_loc(Parser* p) {
	return (FileLoc){ p->l.file, p->l.line, p->l.col };
}

void parser_next(Parser* p) {
	lexer_lex(&(p->l));
	//printf("[%2d:%-2d]   %s\n", p->l.token.line, p->l.token.col, ttos(p->l.token));
	p->t = p->l.token;
}

void parser_init(Parser* p, const char* file, StrRange src) {
	lexer_init(&(p->l), file, src);
	p->xnest = 0;
	parser_next(p);
}

void parser_expect(Parser* p, TokenKind tok) {
	if(p->t.tok != tok)
		parser_error("unexpected %s, expecting %s", ttos(p->t), token_kind_names[tok]);
	parser_next(p);
}

int parser_accept(Parser* p, TokenKind tok) {
	if(p->t.tok == tok) {
		parser_next(p);
		return 1;
	}
	return 0;
}

StrIntern parser_parse_ident(Parser* p) {
	if(p->t.tok == T_IDENT) {
		StrIntern n = str_intern(p->t.lit);
		parser_next(p);
		return n;
	}
	parser_error("unexpected %s, expecting identifier", ttos(p->t));
	return NULL;
}
u64 parser_parse_int(Parser* p) {
	if(p->t.tok == T_INT) {
		u64 i = strtoull(p->t.lit.s, NULL, 0);
		parser_next(p);
		return i;
	}
	parser_error("unexpected %s, expecting int", ttos(p->t));
	return 0;
}
double parser_parse_float(Parser* p) {
	if(p->t.tok == T_FLOAT) {
		double f = strtod(p->t.lit.s, NULL);
		parser_next(p);
		return f;
	}
	parser_error("unexpected %s, expecting float", ttos(p->t));
	return 0;
}
StrRange parser_parse_string(Parser* p) {
	if(p->t.tok == T_STRING) {
		StrRange s = string_range_len(p->t.lit.s + 1, p->t.lit.l - 2);
		parser_next(p);
		return s;
	}
	parser_error("unexpected %s, expecting string", ttos(p->t));
	return (StrRange){NULL, 0};
}
i32 parser_parse_char(Parser* p) {
	if(p->t.tok == T_CHAR) {
		if(p->t.lit.l != 3)
			fatal("not implemented l=%d", p->t.lit.l);
		i32 c = p->t.lit.s[1];
		parser_next(p);
		return c;
	}
	parser_error("unexpected %s, expecting char literal", ttos(p->t));
	return 0;
}


AstExpr* parser_parse_expr(Parser* p);
AstStmt* parser_parse_stmt(Parser* p);
AstType* parser_parse_type(Parser* p);

// ExprList = Expr | Expr ',' ExprList
AstExpr** parser_parse_expr_list(Parser* p, AstExpr** exprs, int trailing_comma) {
	buf_push(exprs, parser_parse_expr(p));
	while(p->t.tok == T_COMMA) {
		parser_next(p);
		if(trailing_comma && p->t.tok == T_RPAREN)
			break;
		buf_push(exprs, parser_parse_expr(p));
	}
	return exprs;
}

// TypeList = Type | Type ',' TypeList
AstType** parser_parse_type_list(Parser* p, AstType** types, int trailing_comma) {
	buf_push(types, parser_parse_type(p));
	while(p->t.tok == T_COMMA) {
		parser_next(p);
		if(trailing_comma && p->t.tok == T_RPAREN)
			break;
		buf_push(types, parser_parse_type(p));
	}
	return types;
}

// TypeTuple = '(' Type ',' TypeList ')'
//           | '(' Type ')'
//           | '(' ')'
AstType* parser_parse_type_tuple(Parser* p) {
	FileLoc loc = parser_loc(p);

	parser_expect(p, T_LPAREN);
	if(parser_accept(p, T_RPAREN))
		return ast_type_tuple(loc, ast_type_list(NULL));
	
	AstType* t = parser_parse_type(p);
	if(parser_accept(p, T_COMMA)) {
		AstType** types = NULL;
		buf_push(types, t);
		if(p->t.tok == T_RPAREN) {
			types = parser_parse_type_list(p, types, 1);
		}
		t = ast_type_tuple(loc, ast_type_list(types));
	}
	parser_expect(p, T_RPAREN);
	return t;
}

// Type = ident | '*' Type | '(' Type ')'
//      | '[' Type ']'
//      | '[' Type ';' int ']'
//      | '(' Type ',' TypeList ')'
AstType* parser_parse_type(Parser* p) {
	FileLoc loc = parser_loc(p);
	switch(p->t.tok) {
		case T_IDENT: {
			StrIntern n = parser_parse_ident(p);
			return ast_type_name(loc, n);
		}
		case T_MUL:
			parser_next(p);
			return ast_type_ptr(loc, parser_parse_type(p));
		case T_LPAREN:
			return parser_parse_type_tuple(p);
		case T_LBRACK: {
			parser_next(p);
			AstType* t = parser_parse_type(p);
			if(parser_accept(p, T_COMMA)) {
				u64 size = parser_parse_int(p);
				parser_expect(p, T_RBRACK);
				return ast_type_array(loc, size, t);
			}
			parser_expect(p, T_RBRACK);
			return ast_type_slice(loc, t);
		}
		default:
			parser_error("unexpected %s, expecting type", ttos(p->t));
			return NULL;
	}
}

// Operand = ident | int | float | string 
//         | '(' Expr ')'
//         | '(' Expr ',' ExprList ')' | '(' ')' 
//         | '[' Expr ';' int ']'
//         | '[' ExprList ']'
AstExpr* parser_parse_operand(Parser* p) {
	FileLoc loc = parser_loc(p);
	switch(p->t.tok) {
		case T_IDENT: {
			StrIntern n = parser_parse_ident(p);
			return ast_expr_ident(loc, n);
		}
		case T_INT:
			return ast_expr_lit_int(loc, parser_parse_int(p));
		case T_FLOAT:
			return ast_expr_lit_float(loc, parser_parse_float(p));
		case T_STRING:
			return ast_expr_lit_string(loc, parser_parse_string(p));
		case T_CHAR:
			return ast_expr_lit_char(loc, parser_parse_char(p));
		case T_LPAREN: {
			parser_next(p);
			if(parser_accept(p, T_RPAREN)) {
				return ast_expr_tuple(loc, ast_expr_list(NULL));
			}
			p->xnest++;
			AstExpr* x = parser_parse_expr(p);
			if(parser_accept(p, T_COMMA)) {
				AstExpr** exprs = NULL;
				buf_push(exprs, x);
				if(p->t.tok != T_RPAREN) {
					exprs = parser_parse_expr_list(p, exprs, 0);
				}
				x = ast_expr_tuple(loc, ast_expr_list(exprs));
			}
			p->xnest--;
			parser_expect(p, T_RPAREN);
			return x;
		}
		case T_LBRACK: {
			parser_next(p);
			p->xnest++;
			AstExpr* x = parser_parse_expr(p);
			if(parser_accept(p, T_SEMI)) {
				if(p->t.tok != T_INT) {
					parser_error("unexpected %s, expecting array size", ttos(p->t));
					return NULL;
				}
				x = ast_expr_array(loc, x, parser_parse_int(p));
			} else {
				AstExpr** exprs = NULL;
				buf_push(exprs, x);
				if(parser_accept(p, T_COMMA) && p->t.tok != T_RBRACK) {
					exprs = parser_parse_expr_list(p, exprs, 0);
				}
				x = ast_expr_array_list(loc, ast_expr_list(exprs));
			}
			p->xnest--;
			parser_expect(p, T_RBRACK);
			return x;
		}
		default:
			parser_error("unexpected %s, expecting expression", ttos(p->t));
			return NULL;
	}
}

// PrimaryExpr = Operand 
//             | PrimaryExpr '::' ident
//             | PrimaryExpr '::' '<' TypeList '>'
//             | PrimaryExpr '.' ident 
//             | PrimaryExpr '[' Expr ']' 
//             | PrimaryExpr '(' ExprList ')' 
//             | PrimaryExpr '{' ExprFields '}'
AstExpr* parser_parse_expr_primary(Parser* p) {
	FileLoc loc = parser_loc(p);
	AstExpr* x = parser_parse_operand(p);
	while(1) {
		switch(p->t.tok) {
			case T_DOT:
				parser_next(p);
				StrIntern n = parser_parse_ident(p);
				x = ast_expr_member(loc, x, n);
				break;
			case T_DCOLON:
				parser_next(p);
				parser_parse_ident(p);
				fatal("ACCESS SCOPE not implemented");
				break;
			case T_LBRACK:
				parser_next(p);
				p->xnest++;
				AstExpr* arg = parser_parse_expr(p);
				p->xnest--;
				parser_expect(p, T_RBRACK);
				x = ast_expr_index(loc, x, arg);
				break;
			case T_LPAREN: {
				parser_next(p);
				p->xnest++;
				AstExpr** exprs = NULL;
				if(!parser_accept(p, T_RPAREN)) {
					exprs = parser_parse_expr_list(p, NULL, 0);
					parser_expect(p, T_RPAREN);
				}
				p->xnest--;
				x = ast_expr_call(loc, x, ast_expr_list(exprs));
				break;
			}
			case T_LBRACE: {
				int complit = 0;
				switch(x->kind) {
					case AST_EXPR_IDENT:
						if(p->xnest >= 0)
							complit = 1;
					case AST_EXPR_ARRAY:
					case AST_EXPR_ARRAY_LIST:
						complit = 1;
					default:
						break;
				}
				if(!complit)
					goto end;

				parser_next(p);
				p->xnest++;
				AstArg** exprs = NULL;
				while(p->t.tok != T_RBRACE) {
					StrIntern name = parser_parse_ident(p);
					parser_expect(p, T_COLON);
					AstExpr* expr = parser_parse_expr(p);
					buf_push(exprs, ast_arg_new(name, expr));
					if(p->t.tok == T_RBRACE)
						break;
					parser_expect(p, T_COMMA);
				}
				p->xnest--;
				parser_expect(p, T_RBRACE);
				x = ast_expr_init(loc, x, ast_arg_list(exprs));
				break;
			}
			default:
				goto end;
		}
	}
end:
	return x;
}

// UnaryExpr = PrimaryExpr | unary_op UnaryExpr
AstExpr* parser_parse_expr_unary(Parser* p) {
	FileLoc loc = parser_loc(p);
	switch(p->t.tok) {
		case T_MUL: case T_ADD: case T_SUB: case T_NOT: case T_AND: {
			TokenKind op = p->t.tok;
			parser_next(p);
			return ast_expr_unary(loc, parser_parse_expr_unary(p), op);
		}
		default:
			break;
	}
	return parser_parse_expr_primary(p);
}

// BinaryExpr = UnaryExpr | BinaryExpr binary_op BinaryExpr
AstExpr* parser_parse_expr_binary(Parser* p, int prec) {
	FileLoc loc = parser_loc(p);
	AstExpr* lhs = parser_parse_expr_unary(p);
	while(token_operator_prec[p->t.tok] > prec) {
		TokenKind op = p->t.tok;
		int tprec = token_operator_prec[p->t.tok];
		parser_next(p);
		if(op == T_AS)
			lhs = ast_expr_cast(loc, lhs, parser_parse_type(p));
		else
			lhs = ast_expr_binary(loc, lhs, op, parser_parse_expr_binary(p, tprec));
		
	}
	return lhs;
}

// Expr = BinaryExpr
AstExpr* parser_parse_expr(Parser* p) {
	return parser_parse_expr_binary(p, 0);
}

// StmtList = Stmt | Stmt ';' StmtList
AstStmt** parser_parse_stmt_list(Parser* p) {
	AstStmt** stmts = NULL;
	while(p->t.tok != T_EOF && p->t.tok != T_RBRACE) {
		AstStmt* stmt = parser_parse_stmt(p);
		if(stmt)
			buf_push(stmts, stmt);
		
		if(!parser_accept(p, T_SEMI) && p->t.tok != T_RBRACE) {
			parser_error("unexpected %s at end of statement", ttos(p->t));
			return NULL;
		}
	}
	return stmts;
}

AstParam** parser_parse_arg_list(Parser* p) {
	AstParam** args = NULL;
	while(1) {
		StrIntern n = parser_parse_ident(p);
		parser_expect(p, T_COLON);
		AstType* t = parser_parse_type(p);
		buf_push(args, ast_param_new(n, t));
		if(p->t.tok == T_RPAREN)
			break;
		parser_expect(p, T_COMMA);
	}
	return args;
}

// DeclFn = 'fn' ident '(' ArgList ')' '->' Type '{' StmtList '}'
//        | 'fn' 'extern' ident '(' ArgList ')' '->' Type
AstDecl* parser_parse_decl_fn(Parser* p, int is_extern) {
	FileLoc loc = parser_loc(p);
	StrIntern n = parser_parse_ident(p);
	AstParam** args = NULL;
	AstType* type = NULL;
	AstStmt** stmts = NULL;
	
	parser_expect(p, T_LPAREN);
	if(p->t.tok != T_RPAREN)
		args = parser_parse_arg_list(p);
	
	parser_expect(p, T_RPAREN);
	if(parser_accept(p, T_ARROW)) {
		type = parser_parse_type(p);
	}
	if(!is_extern) {
		parser_expect(p, T_LBRACE);
		stmts = parser_parse_stmt_list(p);
		parser_expect(p, T_RBRACE);
	}
	return ast_decl_fn(loc, n, is_extern, ast_param_list(args), type, ast_stmt_list(stmts));
}

// DeclLet = 'let' ident '=' Expr
AstDecl* parser_parse_decl_let(Parser* p, int is_extern) {
	FileLoc loc = parser_loc(p);
	StrIntern n = parser_parse_ident(p);
	if(is_extern) {
		parser_expect(p, T_COLON);
		return ast_decl_let(loc, n, NULL, parser_parse_type(p), 1);
	} else {
		parser_expect(p, T_ASSIGN);
		return ast_decl_let(loc, n, parser_parse_expr(p), NULL, 0);
	}
}

// DeclConst = 'const' ident '=' Expr
AstDecl* parser_parse_decl_const(Parser* p) {
	FileLoc loc = parser_loc(p);
	StrIntern n = parser_parse_ident(p);
	parser_expect(p, T_ASSIGN);
	return ast_decl_const(loc, n, parser_parse_expr(p));
}

// StructFields = '{'  '}'
AstParamList parser_parse_decl_struct_fields(Parser* p, FileLoc loc) {
	parser_expect(p, T_LBRACE);
	AstParam** args = NULL;
	while(p->t.tok != T_RBRACE) {
		StrIntern name = parser_parse_ident(p);
		parser_expect(p, T_COLON);
		AstType* type = parser_parse_type(p);
		buf_push(args, ast_param_new(name, type));
		parser_expect(p, T_COMMA);
	}
	parser_expect(p, T_RBRACE);
	return ast_param_list(args);
}

// DeclEnum = 'enum' ident '{' EnumFields '}'
AstDecl* parser_parse_decl_enum(Parser* p) {
	FileLoc loc = parser_loc(p);
	StrIntern n = parser_parse_ident(p);
	parser_expect(p, T_LBRACE);
	AstParam** args = NULL;
	while(p->t.tok != T_RBRACE) {
		//FileLoc loc1 = parser_loc(p);
		StrIntern name = parser_parse_ident(p);
		AstType* type = NULL;
		if(p->t.tok == T_LPAREN) {
			type = parser_parse_type_tuple(p);
		} else if(p->t.tok == T_LBRACE) {
			parser_error("not supported yet!");
		}
		buf_push(args, ast_param_new(name, type));
		parser_expect(p, T_COMMA);
	}
	parser_expect(p, T_RBRACE);
	return ast_decl_enum(loc, n, ast_param_list(args));
}

// DeclStruct = 'struct' ident StructFields
AstDecl* parser_parse_decl_struct(Parser* p) {
	FileLoc loc = parser_loc(p);
	StrIntern n = parser_parse_ident(p);
	return ast_decl_struct(loc, n, parser_parse_decl_struct_fields(p, loc));
}


// DeclType = 'type' ident = Type
AstDecl* parser_parse_decl_type(Parser* p) {
	FileLoc loc = parser_loc(p);
	StrIntern n = parser_parse_ident(p);
	parser_expect(p, T_ASSIGN);
	return ast_decl_type(loc, n, parser_parse_type(p));
}

// Decl = DeclFn | DeclLet | DeclConst | DeclType
AstDecl* parser_parse_decl(Parser* p) {
	switch(p->t.tok) {
		case T_EXTERN:
			parser_next(p);
			if(parser_accept(p, T_FN))
				return parser_parse_decl_fn(p, 1);
			else
				return parser_parse_decl_let(p, 1);
		case T_FN:
			parser_next(p);
			return parser_parse_decl_fn(p, 0);
		case T_LET:
			parser_next(p);
			return parser_parse_decl_let(p, 0);
		case T_CONST:
			parser_next(p);
			return parser_parse_decl_const(p);
		case T_ENUM:
			parser_next(p);
			return parser_parse_decl_enum(p);
		case T_STRUCT:
			parser_next(p);
			return parser_parse_decl_struct(p);
		case T_TYPE:
			parser_next(p);
			return parser_parse_decl_type(p);
		default:
			parser_error("non-declaration statment outside function body, (%s)", ttos(p->t));
			return NULL;
	}
}

void parser_parse_decls(Parser* p) {
	while(p->t.tok != T_EOF) {
		parser_parse_decl(p);
		if(p->t.tok != T_EOF && !parser_accept(p, T_SEMI)) {
			parser_error("unexpected %s after top level declaration", ttos(p->t));
		}
	}
}

// Stmt = DeclLet 
//      | DeclConst
//      | 'return' Expr?
//      | Expr
AstStmt* parser_parse_stmt(Parser* p) {
	FileLoc loc = parser_loc(p);
	switch(p->t.tok) {
		case T_LET:
			parser_next(p);
			return ast_stmt_decl(loc, parser_parse_decl_let(p, 0));
		case T_CONST:
			parser_next(p);
			return ast_stmt_decl(loc, parser_parse_decl_const(p));
		case T_IF: {
			parser_next(p);
			AstExpr* cond = parser_parse_expr(p);
			parser_expect(p, T_LBRACE);
			AstStmt** body = parser_parse_stmt_list(p);
			parser_expect(p, T_RBRACE);

			AstStmt* els = NULL;
			if(parser_accept(p, T_ELSE)) {
				FileLoc locelse = parser_loc(p);
				parser_expect(p, T_LBRACE);
				els = ast_stmt_block(locelse, ast_stmt_list(parser_parse_stmt_list(p)));
				parser_expect(p, T_RBRACE);                
			}
			return ast_stmt_if(loc, cond, ast_stmt_list(body), els);
		}
		case T_FOR: {
			parser_next(p);
			AstExpr* cond = parser_parse_expr(p);
			parser_expect(p, T_LBRACE);
			AstStmt** stmts = parser_parse_stmt_list(p);
			parser_expect(p, T_RBRACE);
			return ast_stmt_for(loc, cond, ast_stmt_list(stmts));
		}
		case T_RETURN: {   
			parser_next(p);
			AstExpr* ret = NULL;
			if(p->t.tok != T_SEMI)
				ret = parser_parse_expr(p);
			return ast_stmt_return(loc, ret);
		}
		case T_LBRACE: {
			parser_next(p);
			AstStmt** body = parser_parse_stmt_list(p);
			parser_expect(p, T_RBRACE);
			return ast_stmt_block(loc, ast_stmt_list(body));
		}
		case T_SEMI:
			return NULL;
		default: {
			AstExpr* x = parser_parse_expr(p);
			if(p->t.tok >= T_ASSIGN && p->t.tok <= T_RSHIFT_ASSIGN) {
				TokenKind op = p->t.tok;
				parser_next(p);
				return ast_stmt_assign(loc, x, op, parser_parse_expr(p));
			}
			return ast_stmt_expr(loc, x);
		}
	}
}

AstFile* parser_parse_file(Parser* p) {
	AstDecl** decls = NULL;
	while(p->t.tok != T_EOF) {
		AstDecl* decl = parser_parse_decl(p);
		buf_push(decls, decl);
		if(p->t.tok == T_EOF)
			break;
		parser_expect(p, T_SEMI);
	}
	return ast_file("", ast_decl_list(decls));
}
