// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct Type Type;
typedef struct Symbol Symbol;

#include "types.c"
#include "symbol.c"

typedef map_type(const char*, Symbol*) MapSymbols;

typedef struct Package {
	StrIntern name;
	const char* path;
	MapSymbols symbols;
	MapSymbols exported_symbols;
} Package;

void package_init(Package* p, const char* path) {
	p->name = str_intern_c("main");
	p->path = path;
	p->symbols = (MapSymbols){0};
	p->exported_symbols = (MapSymbols){0};
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
	return sym;
}

void package_add_file(Package* p, AstFile* file) {
	for(isize i = 0; i < file->decls.len; i++) {
		AstDecl* decl = file->decls.list[i];
		map_set(&p->symbols, decl->name, package_add_decl(p, decl));
	}
}
