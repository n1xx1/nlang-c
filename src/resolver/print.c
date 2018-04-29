// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

void print_type_fn(Type* type);

void print_type(Type* type) {
    switch(type->kind) {
        case TYPE_VOID:
        case TYPE_SIGNED:
        case TYPE_UNSIGNED:
        case TYPE_BOOLEAN:
            if(type->symbol) {
                p_printf("%s", type->symbol->name);
                return;
            }
            p_printf("<unknown symbol>");
            break;
        case TYPE_FN:
            p_printf("fn");
            print_type_fn(type);
            break;
    }
}

void print_type_fn(Type* type) {
    assert(type->kind == TYPE_FN);
    p_printf("(");
    for(int i = 0; i < type->fn.args_len; i++) {
        Type* t = type->fn.args[i];
        if(i > 0) p_printf(", ");
        print_type(t);
    }
    p_printf(") -> ");
    print_type(type->fn.ret);
}

PRINT_STRING_FUNC1(type, Type*)
PRINT_STRING_FUNC1(type_fn, Type*)