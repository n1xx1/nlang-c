// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

int print_ast_i;
char print_ast_ipos[256];

void print_ast_nl() {
    p_printf("\n");
    for(int i = 1; i <= print_ast_i; i++) {
        if(print_ast_ipos[i])
            p_printf("|%c", i == print_ast_i ? '-' : ' ');
        else
            p_printf(i == print_ast_i ? "`-" : "  ");
    }
    p_printf("- ");
}

void print_ast_nest(int notlast) {
    print_ast_ipos[++print_ast_i] = notlast;
}
void print_ast_last() {
    print_ast_ipos[print_ast_i] = 0;
}
void print_ast_unnest() {
    --print_ast_i;
}


void print_ast_type(AstType* type) {
    if(type == NULL) {
        p_printf("nil");
        return;
    }
    switch(type->kind) {
        case AST_TYPE_NAME:
            p_printf("%s", type->name);
            break;
        case AST_TYPE_PTR:
            p_printf("*");
            print_ast_type(type->ptr);
            break;
        case AST_TYPE_ARRAY:
            p_printf("[");
            print_ast_type(type->array.type);
            p_printf(", %d]", type->array.size);
            break;
        case AST_TYPE_SLICE:
            p_printf("[");
            print_ast_type(type->slice.type);
            p_printf("]");
            break;
        case AST_TYPE_FN:
            p_printf("fn(");
            for(int i = 0; i < type->fn.args.len; i++) {
                AstType* t = type->fn.args.list[i];
                if(i > 0) p_printf(", ");
                print_ast_type(t);
            }
            if(type->fn.ret) {
                p_printf(" -> ");
                print_ast_type(type->fn.ret);
            }
            break;
        case AST_TYPE_TUPLE: 
            p_printf("(");
            for(int i = 0; i < type->tuple.args.len; i++) {
                AstType* t = type->tuple.args.list[i];
                if(i > 0) p_printf(", ");
                print_ast_type(t);
            }
            p_printf(")");
            break;
    }
}

void print_ast_expr(AstExpr* expr) {
    print_ast_nl();
    if(expr == NULL) {
        p_printf("<NULL>");
        return;
    }
    switch(expr->kind) {
        case AST_EXPR_LIT_INT:
            p_printf("EXPR_LIT_INT %"PRIu64"", expr->lit_int);
            break;
        case AST_EXPR_LIT_FLOAT:
            p_printf("EXPR_LIT_FLOAT %f", expr->lit_float);
            break;
        case AST_EXPR_LIT_STRING:
            p_printf("EXPR_LIT_STRING \"%.*s\"", (int)expr->lit_string.l, expr->lit_string.s);
            break;
        case AST_EXPR_LIT_CHAR:
            p_printf("EXPR_LIT_CHAR '%c'", expr->lit_char);
            break;
        case AST_EXPR_IDENT:
            p_printf("EXPR_IDENT \"%s\"", expr->ident);
            break;
        case AST_EXPR_MEMBER:
            p_printf("EXPR_MEMBER \"%s\"", expr->member.name);
            print_ast_nest(0);
            print_ast_expr(expr->member.x);
            print_ast_unnest();
            break;
        case AST_EXPR_CALL:
            p_printf("EXPR_CALL");
            print_ast_nest(1);
            print_ast_expr(expr->call.x);
            for(int i = 0; i < expr->call.args.len; i++) {
                if(i == expr->call.args.len - 1)
                    print_ast_last();
                print_ast_expr(expr->call.args.list[i]);
            }
            print_ast_unnest();
            break;
        case AST_EXPR_UNARY:
            p_printf("EXPR_UNARY '%s'", token_kind_names[expr->unary.op]);
            print_ast_nest(0);
            print_ast_expr(expr->unary.x);
            print_ast_unnest();
            break;
        case AST_EXPR_BINARY:
            p_printf("EXPR_BINARY '%s'", token_kind_names[expr->binary.op]);
            print_ast_nest(1);
            print_ast_expr(expr->binary.x);
            print_ast_last();
            print_ast_expr(expr->binary.y);
            print_ast_unnest();
            break;
        case AST_EXPR_CAST:
            p_printf("EXPR_CAST '");
            print_ast_type(expr->cast.type);
            p_printf("'");
            print_ast_nest(0);
            print_ast_expr(expr->cast.x);
            print_ast_unnest();
            break;
        case AST_EXPR_INDEX:
            p_printf("EXPR_INDEX");
            print_ast_nest(1);
            print_ast_expr(expr->index.x);
            print_ast_last();
            print_ast_expr(expr->index.arg);
            print_ast_unnest();
            break;
        case AST_EXPR_TUPLE:
            p_printf("EXPR_TUPLE");
            print_ast_nest(1);
            for(int i = 0; i < expr->tuple.args.len; i++) {
                if(i == expr->tuple.args.len - 1)
                    print_ast_last();
                print_ast_expr(expr->tuple.args.list[i]);
            }
            print_ast_unnest();
            break;
        case AST_EXPR_ARRAY:
            p_printf("EXPR_ARRAY %"PRIu32"", expr->array.len);
            print_ast_nest(0);
            print_ast_expr(expr->array.init);
            print_ast_unnest();
            break;
        case AST_EXPR_ARRAY_LIST:
            p_printf("EXPR_ARRAY_LIST");
            print_ast_nest(1);
            for(int i = 0; i < expr->array_list.args.len; i++) {
                if(i == expr->array_list.args.len - 1)
                    print_ast_last();
                print_ast_expr(expr->array_list.args.list[i]);
            }
            print_ast_unnest();
            break;
        case AST_EXPR_INIT:
            p_printf("EXPR_INIT");
            print_ast_nest(expr->init.fields.len > 0 ? 1 : 0);
            print_ast_expr(expr->init.x);
            for(int i = 0; i < expr->init.fields.len; i++) {
                if(i == expr->init.fields.len - 1)
                    print_ast_last();
                print_ast_nl();
                p_printf("FIELD \"%s\"", expr->init.fields.list[i]->name);
                print_ast_nest(0);
                print_ast_expr(expr->init.fields.list[i]->expr);
                print_ast_unnest();
            }
            print_ast_unnest();
            break;
    }
}

void print_ast_stmt(AstStmt* stmt);
void print_ast_stmt_list(AstStmtList list) {
    print_ast_nl();
    p_printf("BLOCK");
    print_ast_nest(list.len > 1 ? 1 : 0);
    for(int i = 0; i < list.len; i++) {
        if(i == list.len - 1)
            print_ast_last();
        print_ast_stmt(list.list[i]);
    }
    print_ast_unnest();
}

void print_ast_decl(AstDecl* decl);

void print_ast_stmt(AstStmt* stmt) {
    if(stmt == NULL) {
        print_ast_nl();
        p_printf("<NULL>");
        return;
    }
    switch(stmt->kind) {
        case AST_STMT_DECL:
            print_ast_decl(stmt->decl);
            break;
        case AST_STMT_EXPR:
            print_ast_expr(stmt->expr);
            break;
        case AST_STMT_IF:
            print_ast_nl();
            p_printf("STMT_IF ");
            print_ast_nest(1);
            print_ast_expr(stmt->if_.cond);
            print_ast_last();
            print_ast_stmt(stmt->if_.els);
            print_ast_unnest();
            break;
        case AST_STMT_FOR: {
            print_ast_nl();
            p_printf("STMT_FOR");
            print_ast_nest(1);
            print_ast_expr(stmt->for_.cond);
            print_ast_last();
            print_ast_unnest();
            break;
        }
        case AST_STMT_RETURN:
            print_ast_nl();
            p_printf("STMT_RETURN");
            print_ast_nest(0);
            print_ast_expr(stmt->return_);
            print_ast_unnest();
            break;
        case AST_STMT_ASSIGN:
            print_ast_nl();
            p_printf("STMT_ASSIGN '%s'", token_kind_names[stmt->assign.op]);
            print_ast_nest(1);
            print_ast_expr(stmt->assign.x);
            print_ast_last();
            print_ast_expr(stmt->assign.y);
            print_ast_unnest();
            break;
        case AST_STMT_BLOCK:
            print_ast_stmt_list(stmt->block.body);
            break;
    }
}

void print_ast_decl(AstDecl* decl) {
    print_ast_nl();
    if(decl == NULL) {
        p_printf("<NULL>");
        return;
    }
    switch(decl->kind) {
        case AST_DECL_LET:
            p_printf("DECL_LET \"%s\" ", decl->name);
            if(decl->let.is_extern) {
                p_printf("extern '");
                print_ast_type(decl->let.type);
                p_printf("'");
            }
            if(!decl->let.is_extern) {
                print_ast_nest(0);
                print_ast_expr(decl->let.value);
                print_ast_unnest();
            }
            break;
        case AST_DECL_CONST:
            p_printf("DECL_CONST \"%s\"", decl->name);
            print_ast_nest(0);
            print_ast_expr(decl->const_.value);
            print_ast_unnest();
            break;
        case AST_DECL_FN:
            p_printf("DECL_FN \"%s\"%s", decl->name, decl->fn.is_extern ? " extern" : "");
            print_ast_nest(decl->fn.is_extern && decl->fn.params.len == 0 ? 0 : 1);
            print_ast_nl();
            p_printf("RET '");
            print_ast_type(decl->fn.ret);
            p_printf("'");
            for(int i = 0; i < decl->fn.params.len; i++) {
                if(decl->fn.is_extern && i == decl->fn.params.len - 1)
                    print_ast_last();
                print_ast_nl();
                p_printf("ARG \"%s\" '", decl->fn.params.list[i]->name);
                print_ast_type(decl->fn.params.list[i]->type);
                p_printf("'");
            }
            if(!decl->fn.is_extern) {
                print_ast_last();
                print_ast_stmt_list(decl->fn.body);
            }
            print_ast_unnest();
            break;
        case AST_DECL_STRUCT:
            p_printf("DECL_STRUCT \"%s\"", decl->name);
            print_ast_nest(0);
            for(int i = 0; i < decl->struct_.params.len; i++) {
                print_ast_nl();
                p_printf("FIELD \"%s\" '", decl->struct_.params.list[i]->name);
                print_ast_type(decl->struct_.params.list[i]->type);
                p_printf("'");
            }
            print_ast_unnest();
            break;
        case AST_DECL_ENUM:
            p_printf("DECL_ENUM \"%s\"", decl->name);
            print_ast_nest(0);
            for(int i = 0; i < decl->enum_.params.len; i++) {
                print_ast_nl();
                p_printf("FIELD \"%s\" '", decl->enum_.params.list[i]->name);
                print_ast_type(decl->enum_.params.list[i]->type);
                p_printf("'");
            }
            print_ast_unnest();
            break;
        case AST_DECL_TYPE:
            p_printf("DECL_TYPE \"%s\" '", decl->name);
            print_ast_type(decl->type.type);
            p_printf("'");
            break;
    }
}

void print_ast_file(AstFile* file) {
    for(int i = 0; i < file->decls.len; i++) {
        AstDecl* decl = file->decls.list[i];
        print_ast_decl(decl);
        p_printf("\n");
    }
}

PRINT_STRING_FUNC1(ast_type, AstType*)
PRINT_STRING_FUNC1(ast_decl, AstDecl*)
PRINT_STRING_FUNC1(ast_expr, AstExpr*)
PRINT_STRING_FUNC1(ast_stmt_list, AstStmtList)
PRINT_STRING_FUNC1(ast_stmt, AstStmt*)
PRINT_STRING_FUNC1(ast_file, AstFile*)
