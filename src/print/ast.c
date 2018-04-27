// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct AstExpr AstExpr;
typedef struct AstStmt AstStmt;
typedef struct AstDecl AstDecl;
typedef struct AstType AstType;

typedef enum AstExprKind {
	AST_EXPR_LIT_INT,
	AST_EXPR_LIT_FLOAT,
	AST_EXPR_LIT_STRING,
	AST_EXPR_LIT_CHAR,
	AST_EXPR_IDENT,
	AST_EXPR_MEMBER,
	AST_EXPR_CALL,
	AST_EXPR_UNARY,
	AST_EXPR_BINARY,
	AST_EXPR_CAST,
	AST_EXPR_INDEX,
	AST_EXPR_TUPLE,
	AST_EXPR_ARRAY,
	AST_EXPR_ARRAY_LIST,
	AST_EXPR_INIT,
} AstExprKind;

typedef enum AstStmtKind {
	AST_STMT_DECL,
	AST_STMT_EXPR,
	AST_STMT_IF,
	AST_STMT_FOR,
	AST_STMT_RETURN,
	AST_STMT_ASSIGN,
	AST_STMT_BLOCK
} AstStmtKind;

typedef enum AstDeclKind {
	AST_DECL_LET,
	AST_DECL_CONST,
	AST_DECL_FN,
	AST_DECL_STRUCT,
	AST_DECL_ENUM,
	AST_DECL_TYPE
} AstDeclKind;

typedef enum AstTypeKind {
	AST_TYPE_NAME,
	AST_TYPE_PTR,
	AST_TYPE_ARRAY,
	AST_TYPE_FN,
	AST_TYPE_SLICE,
	AST_TYPE_TUPLE
} AstTypeKind;

typedef struct AstParam {
	StrIntern name;
	AstType* type;
} AstParam;

typedef struct AstArg {
	StrIntern name;
	AstExpr* expr;
} AstArg;

typedef struct AstStmtList {
	AstStmt** list;
	isize len;
} AstStmtList;

typedef struct AstParamList {
	AstParam** list;
	isize len;
} AstParamList;

typedef struct AstExprList {
	AstExpr** list;
	isize len;
} AstExprList;

typedef struct AstTypeList {
	AstType** list;
	isize len;
} AstTypeList;

typedef struct AstArgList {
	AstArg** list;
	isize len;
} AstArgList;

typedef struct AstDeclList {
	AstDecl** list;
	isize len;
} AstDeclList;

struct AstExpr {
	AstExprKind kind;
	FileLoc loc;
	union {
		u64 lit_int;           // AST_EXPR_LIT_INT
		double lit_float;      // AST_EXPR_LIT_FLOAT
		StrRange lit_string;   // AST_EXPR_LIT_STRING
		i32 lit_char;          // AST_EXPR_LIT_CHAR
		StrIntern ident;       // AST_EXPR_IDENT
		struct {
			AstExpr* x;
			StrIntern name;
		} member;              // AST_EXPR_MEMBER
		struct {
			AstExpr* x;
			AstExprList args;
		} call;                // AST_EXPR_CALL
		struct {
			AstExpr* x;
			TokenKind op;
		} unary;               // AST_EXPR_UNARY
		struct {
			AstExpr* x;
			TokenKind op;
			AstExpr* y;
		} binary;              // AST_EXPR_BINARY
		struct {
			AstExpr* x;
			AstType* type;
		} cast;                // AST_EXPR_CAST
		struct {
			AstExpr* x;
			AstExpr* arg;
		} index;               // AST_EXPR_INDEX
		struct {
			AstExprList args;
		} tuple;               // AST_EXPR_TUPLE
		struct {
			AstExpr* init;
			u32 len;
		} array;               // AST_EXPR_ARRAY
		struct {
			AstExprList args;
		} array_list;          // AST_EXPR_ARRAY_LIST
		struct {
			AstExpr* x;
			AstArgList fields;
		} init;                // AST_EXPR_INIT
	};
};

struct AstStmt {
	AstStmtKind kind;
	FileLoc loc;
	union {
		AstDecl* decl;         // AST_STMT_DECL
		AstExpr* expr;         // AST_STMT_EXPR
		struct {
			AstExpr* cond;
			AstStmtList body;
			AstStmt* els;
		} if_;                 // AST_STMT_IF
		struct {
			AstExpr* cond;
			AstStmtList body;
		} for_;                // AST_STMT_FOR
		AstExpr* return_;      // AST_STMT_RETURN
		struct {
			AstExpr* x;
			TokenKind op;
			AstExpr* y;
		} assign;              // AST_STMT_ASSIGN
		struct {
			AstStmtList body;
		} block;               // AST_STMT_BLOCK
	};
};

struct AstType {
	AstTypeKind kind;
	FileLoc loc;
	union {
		StrIntern name;        // AST_TYPE_NAME
		AstType* ptr;          // AST_TYPE_PTR
		struct {
			i32 size;
			AstType* type;
		} array;               // AST_TYPE_ARRAY
		struct {
			AstType* type;
		} slice;               // AST_TYPE_SLICE
		struct {
			AstType* ret;
			AstTypeList args;
		} fn;                  // AST_TYPE_FN
		struct {
			AstTypeList args;
		} tuple;               // AST_TYPE_TUPLE
	};
};

struct AstDecl {
	AstDeclKind kind;
	FileLoc loc;
	StrIntern name;
	union {
		struct {
			AstExpr* value;
			AstType* type;
			bool is_extern;
		} let;                // AST_DECL_LET
		struct {
			AstExpr* value;
		} const_;             // AST_DECL_CONST
		struct {
			int is_extern;
			AstParamList params;
			AstType* ret;
			AstStmtList body;
		} fn;                 // AST_DECL_FN
		struct {
			AstParamList params;
		} enum_;              // AST_DECL_ENUM
		struct {
			AstParamList params;
		} struct_;            // AST_DECL_STRUCT
		struct {
			AstType* type;
		} type;               // AST_DECL_TYPE
	};
};

typedef struct AstFile {
	const char* path;
	AstDeclList decls;
} AstFile;

MemoryPool ast_pool;

void* ast_alloc(size_t size) {
    assert(size != 0);
    void *ptr = mpool_alloc(&ast_pool, size);
    memset(ptr, 0, size);
    return ptr;
}
void* ast_dup(void* src, size_t size) {
    void* dst = ast_alloc(size);
    memcpy(dst, src, size);
    return dst;
}

AstArg* ast_arg_new(StrIntern name, AstExpr* expr) {
    AstArg* arg = ast_alloc(sizeof(AstArg));
    arg->name = name;
    arg->expr = expr;
    return arg;
}

AstParam* ast_param_new(StrIntern name, AstType* type) {
    AstParam* arg = ast_alloc(sizeof(AstParam));
    arg->name = name;
    arg->type = type;
    return arg;
}

AstStmtList ast_stmt_list(AstStmt** buf) {
    size_t len = buf_len(buf);
    AstStmt** stmts = buf ? ast_dup(buf, len * sizeof(*buf)) : NULL;
    buf_free(buf);
    return (AstStmtList){stmts, len};
}
AstParamList ast_param_list(AstParam** buf) {
    size_t len = buf_len(buf);
    AstParam** params = buf ? ast_dup(buf, len * sizeof(*buf)) : NULL;
    buf_free(buf);
    return (AstParamList){params, len};
}
AstArgList ast_arg_list(AstArg** buf) {
    size_t len = buf_len(buf);
    AstArg** args = buf ? ast_dup(buf, len * sizeof(*buf)) : NULL;
    buf_free(buf);
    return (AstArgList){args, len};
}
AstExprList ast_expr_list(AstExpr** buf) {
    size_t len = buf_len(buf);
    AstExpr** exprs = buf ? ast_dup(buf, len * sizeof(*buf)) : NULL;
    buf_free(buf);
    return (AstExprList){exprs, len};
}
AstTypeList ast_type_list(AstType** buf) {
    size_t len = buf_len(buf);
    AstType** types = buf ? ast_dup(buf, len * sizeof(*buf)) : NULL;
    buf_free(buf);
    return (AstTypeList){types, len};
}
AstDeclList ast_decl_list(AstDecl** buf) {
    size_t len = buf_len(buf);
    AstDecl** decls = buf ? ast_dup(buf, len * sizeof(*buf)) : NULL;
    buf_free(buf);
    return (AstDeclList){decls, len};
}

AstDecl* ast_decl_new(FileLoc loc, AstDeclKind kind, StrIntern name) {
    AstDecl* decl = ast_alloc(sizeof(AstDecl));
    decl->loc = loc;
    decl->kind = kind;
    decl->name = name;
    return decl;
}

AstDecl* ast_decl_let(FileLoc loc, StrIntern name, AstExpr* value, AstType* type, int is_extern) {
    AstDecl* decl = ast_decl_new(loc, AST_DECL_LET, name);
    decl->let.value = value;
    decl->let.type = type;
    decl->let.is_extern = is_extern;
    return decl;
}
AstDecl* ast_decl_const(FileLoc loc, StrIntern name, AstExpr* value) {
    AstDecl* decl = ast_decl_new(loc, AST_DECL_CONST, name);
    decl->const_.value = value;
    return decl;
}
AstDecl* ast_decl_fn(FileLoc loc, StrIntern name, int is_extern, AstParamList params, AstType* ret, AstStmtList body) {
    AstDecl* decl = ast_decl_new(loc, AST_DECL_FN, name);
    decl->fn.is_extern = is_extern;
    decl->fn.params = params;
    decl->fn.ret = ret;
    decl->fn.body = body;
    return decl;
}
AstDecl* ast_decl_enum(FileLoc loc, StrIntern name, AstParamList params) {
    AstDecl* decl = ast_decl_new(loc, AST_DECL_ENUM, name);
    decl->enum_.params = params;
    return decl;
}
AstDecl* ast_decl_struct(FileLoc loc, StrIntern name, AstParamList params) {
    AstDecl* decl = ast_decl_new(loc, AST_DECL_STRUCT, name);
    decl->struct_.params = params;
    return decl;
}
AstDecl* ast_decl_type(FileLoc loc, StrIntern name, AstType* type) {
    AstDecl* decl = ast_decl_new(loc, AST_DECL_TYPE, name);
    decl->type.type = type;
    return decl;
}

AstExpr* ast_expr_new(FileLoc loc, AstExprKind kind) {
    AstExpr* expr = ast_alloc(sizeof(AstExpr));
    expr->loc = loc;
    expr->kind = kind;
    return expr;
}
AstExpr* ast_expr_lit_int(FileLoc loc, u64 lit) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_LIT_INT);
    expr->lit_int = lit;
    return expr;
}
AstExpr* ast_expr_lit_float(FileLoc loc, double lit) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_LIT_FLOAT);
    expr->lit_float = lit;
    return expr;
}
AstExpr* ast_expr_lit_string(FileLoc loc, StrRange lit) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_LIT_STRING);
    expr->lit_string = lit;
    return expr;
}
AstExpr* ast_expr_lit_char(FileLoc loc, i32 lit) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_LIT_CHAR);
    expr->lit_char = lit;
    return expr;    
}
AstExpr* ast_expr_ident(FileLoc loc, StrIntern ident) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_IDENT);
    expr->ident = ident;
    return expr;
}
AstExpr* ast_expr_member(FileLoc loc, AstExpr* x, StrIntern name) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_MEMBER);
    expr->member.x = x;
    expr->member.name = name;
    return expr;
}
AstExpr* ast_expr_call(FileLoc loc, AstExpr* x, AstExprList args) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_CALL);
    expr->call.x = x;
    expr->call.args = args;
    return expr;
}
AstExpr* ast_expr_unary(FileLoc loc, AstExpr* x, TokenKind op) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_UNARY);
    expr->unary.x = x;
    expr->unary.op = op;
    return expr;
}
AstExpr* ast_expr_binary(FileLoc loc, AstExpr* x, TokenKind op, AstExpr* y) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_BINARY);
    expr->binary.x = x;
    expr->binary.op = op;
    expr->binary.y = y;
    return expr;
}
AstExpr* ast_expr_cast(FileLoc loc, AstExpr* x, AstType* type) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_CAST);
    expr->cast.x = x;
    expr->cast.type = type;
    return expr;
}
AstExpr* ast_expr_index(FileLoc loc, AstExpr* x, AstExpr* arg) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_INDEX);
    expr->index.x = x;
    expr->index.arg = arg;
    return expr;
}
AstExpr* ast_expr_tuple(FileLoc loc, AstExprList args) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_TUPLE);
    expr->tuple.args = args;
    return expr;
}
AstExpr* ast_expr_array(FileLoc loc, AstExpr* init, u32 len) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_ARRAY);
    expr->array.init = init;
    expr->array.len = len;
    return expr;
}
AstExpr* ast_expr_array_list(FileLoc loc, AstExprList args) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_ARRAY_LIST);
    expr->array_list.args = args;
    return expr;
}
AstExpr* ast_expr_init(FileLoc loc, AstExpr* x, AstArgList fields) {
    AstExpr* expr = ast_expr_new(loc, AST_EXPR_INIT);
    expr->init.x = x;
    expr->init.fields = fields;
    return expr;
}

AstStmt* ast_stmt_new(FileLoc loc, AstStmtKind kind) {
    AstStmt* stmt = ast_alloc(sizeof(AstStmt));
    stmt->loc = loc;
    stmt->kind = kind;
    return stmt;
}
AstStmt* ast_stmt_decl(FileLoc loc, AstDecl* decl) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_DECL);
    stmt->decl = decl;
    return stmt;
}
AstStmt* ast_stmt_expr(FileLoc loc, AstExpr* expr) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_EXPR);
    stmt->expr = expr;
    return stmt;
}
AstStmt* ast_stmt_if(FileLoc loc, AstExpr* cond, AstStmtList body, AstStmt* els) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_IF);
    stmt->if_.cond = cond;
    stmt->if_.body = body;
    stmt->if_.els = els;
    return stmt;
}
AstStmt* ast_stmt_for(FileLoc loc, AstExpr* cond, AstStmtList body) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_FOR);
    stmt->for_.cond = cond;
    stmt->for_.body = body;
    return stmt;
}
AstStmt* ast_stmt_return(FileLoc loc, AstExpr* return_) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_RETURN);
    stmt->return_ = return_;
    return stmt;
}
AstStmt* ast_stmt_assign(FileLoc loc, AstExpr* x, TokenKind op, AstExpr* y) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_ASSIGN);    
    stmt->assign.x = x;
    stmt->assign.op = op;
    stmt->assign.y = y;
    return stmt;
}
AstStmt* ast_stmt_block(FileLoc loc, AstStmtList body) {
    AstStmt* stmt = ast_stmt_new(loc, AST_STMT_BLOCK);
    stmt->block.body = body;
    return stmt;
}

AstType* ast_type_new(FileLoc loc, AstTypeKind kind) {
    AstType* type = ast_alloc(sizeof(AstType));
    type->loc = loc;
    type->kind = kind;
    return type;
}
AstType* ast_type_name(FileLoc loc, StrIntern name) {
    AstType* type = ast_type_new(loc, AST_TYPE_NAME);
    type->name = name;
    return type;
}
AstType* ast_type_ptr(FileLoc loc, AstType* ptr) {
    AstType* type = ast_type_new(loc, AST_TYPE_PTR);
    type->ptr = ptr;
    return type;
}
AstType* ast_type_array(FileLoc loc, u32 size, AstType* type_) {
    AstType* type = ast_type_new(loc, AST_TYPE_ARRAY);
    type->array.size = size;
    type->array.type = type_;
    return type;
}
AstType* ast_type_fn(FileLoc loc, AstType* ret, AstTypeList args) {
    AstType* type = ast_type_new(loc, AST_TYPE_FN);
    type->fn.ret = ret;
    type->fn.args = args;
    return type;
}
AstType* ast_type_slice(FileLoc loc, AstType* type_) {
    AstType* type = ast_type_new(loc, AST_TYPE_SLICE);
    type->slice.type = type_;
    return type;
}
AstType* ast_type_tuple(FileLoc loc, AstTypeList args) {
    AstType* type = ast_type_new(loc, AST_TYPE_TUPLE);
    type->tuple.args = args;
    return type;
}
AstFile* ast_file(const char* path, AstDeclList decls) {
    AstFile* file = ast_alloc(sizeof(AstFile));
    file->path = path;
    file->decls = decls;
    return file;
}




