// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

int indent;
char indent_places[256];

void print_ast_newline() {
    p_printf("\n");
    for(int i = 1; i <= indent; i++) {
        if(indent_places[i])
            p_printf("|%c", i == indent ? '-' : ' ');
        else
            p_printf(i == indent ? "`-" : "  ");
    }
    p_printf("- ");
}

void indent_nest(int notlast) {
    indent_places[++indent] = notlast;
}
void indent_last() {
    indent_places[indent] = 0;
}
void indent_unnest() {
    --indent;
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
    print_ast_newline();
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
            indent_nest(0);
            print_ast_expr(expr->member.x);
            indent_unnest();
            break;
        case AST_EXPR_CALL:
            p_printf("EXPR_CALL");
            indent_nest(1);
            print_ast_expr(expr->call.x);
            for(int i = 0; i < expr->call.args.len; i++) {
                if(i == expr->call.args.len - 1)
                    indent_last();
                print_ast_expr(expr->call.args.list[i]);
            }
            indent_unnest();
            break;
        case AST_EXPR_UNARY:
            p_printf("EXPR_UNARY '%s'", token_kind_names[expr->unary.op]);
            indent_nest(0);
            print_ast_expr(expr->unary.x);
            indent_unnest();
            break;
        case AST_EXPR_BINARY:
            p_printf("EXPR_BINARY '%s'", token_kind_names[expr->binary.op]);
            indent_nest(1);
            print_ast_expr(expr->binary.x);
            indent_last();
            print_ast_expr(expr->binary.y);
            indent_unnest();
            break;
        case AST_EXPR_CAST:
            p_printf("EXPR_CAST '");
            print_ast_type(expr->cast.type);
            p_printf("'");
            indent_nest(0);
            print_ast_expr(expr->cast.x);
            indent_unnest();
            break;
        case AST_EXPR_INDEX:
            p_printf("EXPR_INDEX");
            indent_nest(1);
            print_ast_expr(expr->index.x);
            indent_last();
            print_ast_expr(expr->index.arg);
            indent_unnest();
            break;
        case AST_EXPR_TUPLE:
            p_printf("EXPR_TUPLE");
            indent_nest(1);
            for(int i = 0; i < expr->tuple.args.len; i++) {
                if(i == expr->tuple.args.len - 1)
                    indent_last();
                print_ast_expr(expr->tuple.args.list[i]);
            }
            indent_unnest();
            break;
        case AST_EXPR_ARRAY:
            p_printf("EXPR_ARRAY %"PRIu32"", expr->array.len);
            indent_nest(0);
            print_ast_expr(expr->array.init);
            indent_unnest();
            break;
        case AST_EXPR_ARRAY_LIST:
            p_printf("EXPR_ARRAY_LIST");
            indent_nest(1);
            for(int i = 0; i < expr->array_list.args.len; i++) {
                if(i == expr->array_list.args.len - 1)
                    indent_last();
                print_ast_expr(expr->array_list.args.list[i]);
            }
            indent_unnest();
            break;
        case AST_EXPR_INIT:
            p_printf("EXPR_INIT");
            indent_nest(expr->init.fields.len > 0 ? 1 : 0);
            print_ast_expr(expr->init.x);
            for(int i = 0; i < expr->init.fields.len; i++) {
                if(i == expr->init.fields.len - 1)
                    indent_last();
                print_ast_newline();
                p_printf("FIELD \"%s\"", expr->init.fields.list[i]->name);
                indent_nest(0);
                print_ast_expr(expr->init.fields.list[i]->expr);
                indent_unnest();
            }
            indent_unnest();
            break;
    }
}

void print_ast_stmt(AstStmt* stmt);
void print_ast_stmt_list(AstStmtList list) {
    print_ast_newline();
    p_printf("BLOCK");
    indent_nest(list.len > 1 ? 1 : 0);
    for(int i = 0; i < list.len; i++) {
        if(i == list.len - 1)
            indent_last();
        print_ast_stmt(list.list[i]);
    }
    indent_unnest();
}

void print_ast_decl(AstDecl* decl);

void print_ast_stmt(AstStmt* stmt) {
    if(stmt == NULL) {
        print_ast_newline();
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
            print_ast_newline();
            p_printf("STMT_IF ");
            indent_nest(1);
            print_ast_expr(stmt->if_.cond);
            indent_last();
            print_ast_stmt(stmt->if_.els);
            indent_unnest();
            break;
        case AST_STMT_FOR: {
            print_ast_newline();
            p_printf("STMT_FOR");
            indent_nest(1);
            print_ast_expr(stmt->for_.cond);
            indent_last();
            indent_unnest();
            break;
        }
        case AST_STMT_RETURN:
            print_ast_newline();
            p_printf("STMT_RETURN");
            indent_nest(0);
            print_ast_expr(stmt->return_);
            indent_unnest();
            break;
        case AST_STMT_ASSIGN:
            print_ast_newline();
            p_printf("STMT_ASSIGN '%s'", token_kind_names[stmt->assign.op]);
            indent_nest(1);
            print_ast_expr(stmt->assign.x);
            indent_last();
            print_ast_expr(stmt->assign.y);
            indent_unnest();
            break;
        case AST_STMT_BLOCK:
            print_ast_stmt_list(stmt->block.body);
            break;
    }
}

void print_ast_decl(AstDecl* decl) {
    print_ast_newline();
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
                indent_nest(0);
                print_ast_expr(decl->let.value);
                indent_unnest();
            }
            break;
        case AST_DECL_CONST:
            p_printf("DECL_CONST \"%s\"", decl->name);
            indent_nest(0);
            print_ast_expr(decl->const_.value);
            indent_unnest();
            break;
        case AST_DECL_FN:
            p_printf("DECL_FN \"%s\"%s", decl->name, decl->fn.is_extern ? " extern" : "");
            indent_nest(decl->fn.is_extern && decl->fn.params.len == 0 ? 0 : 1);
            print_ast_newline();
            p_printf("RET '");
            print_ast_type(decl->fn.ret);
            p_printf("'");
            for(int i = 0; i < decl->fn.params.len; i++) {
                if(decl->fn.is_extern && i == decl->fn.params.len - 1)
                    indent_last();
                print_ast_newline();
                p_printf("ARG \"%s\" '", decl->fn.params.list[i]->name);
                print_ast_type(decl->fn.params.list[i]->type);
                p_printf("'");
            }
            if(!decl->fn.is_extern) {
                indent_last();
                print_ast_stmt_list(decl->fn.body);
            }
            indent_unnest();
            break;
        case AST_DECL_STRUCT:
            p_printf("DECL_STRUCT \"%s\"", decl->name);
            indent_nest(0);
            for(int i = 0; i < decl->struct_.params.len; i++) {
                print_ast_newline();
                p_printf("FIELD \"%s\" '", decl->struct_.params.list[i]->name);
                print_ast_type(decl->struct_.params.list[i]->type);
                p_printf("'");
            }
            indent_unnest();
            break;
        case AST_DECL_ENUM:
            p_printf("DECL_ENUM \"%s\"", decl->name);
            indent_nest(0);
            for(int i = 0; i < decl->enum_.params.len; i++) {
                print_ast_newline();
                p_printf("FIELD \"%s\" '", decl->enum_.params.list[i]->name);
                print_ast_type(decl->enum_.params.list[i]->type);
                p_printf("'");
            }
            indent_unnest();
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

