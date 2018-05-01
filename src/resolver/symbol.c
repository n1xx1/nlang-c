// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef enum SymbolKind {
	SYMBOL_LET,
	SYMBOL_CONST,
	SYMBOL_FN,
	SYMBOL_TYPE,
} SymbolKind;

typedef enum SymbolStatus {
	SYMSTATE_INITIAL,
	SYMSTATE_DECLARING,
	SYMSTATE_DECLARED,
	SYMSTATE_RESOLVING,
	SYMSTATE_RESOLVED,
} SymbolStatus;

struct Symbol {
	SymbolKind kind;
	SymbolStatus state;
	StrIntern name;
	Type* type;
	AstDecl* decl;
};

typedef struct SymbolOrder {
	StrIntern name;
	bool is_decl;
} SymbolOrder;

Symbol* symbol_new(SymbolKind kind, StrIntern name, AstDecl* decl) {
	Symbol* sym = xcalloc(1, sizeof(Symbol));
	sym->kind = kind;
	sym->name = name;
	sym->decl = decl;
	return sym;
}
