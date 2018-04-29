// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct Type Type;
typedef struct Symbol Symbol;

#include "types.c"
#include "symbol.c"
#include "print.c"

// we are using interned strings, so we need a pointers map
typedef map_type(u64, Symbol*) MapSymbols;

typedef struct Package {
	StrIntern name;
	const char* path;
	MapSymbols symbols;
	MapSymbols exported_symbols;
} Package;

Symbol* package_add_type(Package* p, Type* type, StrIntern name) {
	Symbol* sym = symbol_new(SYMBOL_TYPE, name, NULL);
	sym->status = SYMSTATE_RESOLVED;
	sym->type = type;
	map_set(&p->symbols, (usize)name, sym);
	return sym;
}

Symbol* package_add_decl(Package* p, AstDecl* decl) {
	SymbolKind kind;
	switch(decl->kind) {
		case AST_DECL_LET:
			kind = SYMBOL_LET;
			break;
		case AST_DECL_CONST:
			kind = SYMBOL_CONST;
			break;
		case AST_DECL_FN:
			kind = SYMBOL_FN;
			break;
		case AST_DECL_STRUCT:
		case AST_DECL_ENUM:
		case AST_DECL_TYPE:
			kind = SYMBOL_TYPE;
			break;
	}

	StrIntern name = decl->name;
	MapIterator it = map_begin(&p->symbols);
	while(it != map_end(&p->symbols)) {
		if(name == map_iter_key(&p->symbols, &it)) {
			Symbol* sym = map_iter_value(&p->symbols, &it);
			resolve_warning(decl->loc, "symbol '%s' already declared in this package.");
			resolve_error(sym->decl->loc, "previous definition was here.");
			return NULL;
		}
		map_next(&p->symbols, &it);
	}
	Symbol* sym = symbol_new(kind, name, decl);
	map_set(&p->symbols, (u64)name, sym);
	return sym;
}

void package_add_file(Package* p, AstFile* file) {
	for(isize i = 0; i < file->decls.len; i++) {
		AstDecl* decl = file->decls.list[i];
		package_add_decl(p, decl);
	}
}

void package_init(Package* p, const char* path) {
	p->name = str_intern_c("main");
	p->path = path;
	p->symbols = (MapSymbols){0};
	p->exported_symbols = (MapSymbols){0};

	package_add_type(p, primitive_void, str_intern_c("void"));
	package_add_type(p, primitive_bool, str_intern_c("bool"));
	package_add_type(p, primitive_u8, str_intern_c("u8"));
	package_add_type(p, primitive_u16, str_intern_c("u16"));
	package_add_type(p, primitive_u32, str_intern_c("u32"));
	package_add_type(p, primitive_u64, str_intern_c("u64"));
	package_add_type(p, primitive_usize, str_intern_c("usize"));
	package_add_type(p, primitive_i8, str_intern_c("i8"));
	package_add_type(p, primitive_i16, str_intern_c("i16"));
	package_add_type(p, primitive_i32, str_intern_c("i32"));
	package_add_type(p, primitive_i64, str_intern_c("i64"));
	package_add_type(p, primitive_isize, str_intern_c("isize"));
}
