// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct Type Type;
typedef struct Symbol Symbol;

#include "types.c"
#include "symbol.c"
#include "package.c"
#include "print.c"

typedef map_type(const void*, Type*) MapType;

u64 map_Type_fn_hash(const void* x) {
	const Type* type = (const Type*)x;
	assert(type->kind == TYPE_FN);
	u64 hash = map_hash_mix(map_hash_u64((u64)(type->fn.ret)), map_hash_bytes(type->fn.args, type->fn.args_len));
	return hash ? hash : 1;
}
isize map_Type_fn_cmp(const void* x, const void* y) {
	const Type* tx = (const Type*)x;
	const Type* ty = (const Type*)y;
	if(tx->fn.args_len != ty->fn.args_len)
		if(memcmp(tx->fn.args, ty->fn.args, sizeof(Type*) * tx->fn.args_len) == 0)
			return 0;
	return 1;
}

MapType TypeFnMap;
void resolve_init_type_maps() {
	TypeFnMap = (MapType){{0}, new_user_def_fn(map_Type_fn_cmp, map_Type_fn_hash)};
}

Symbol* resolver_resolve_name(Package* pkg, FileLoc loc, StrIntern name, bool needresolve) {
	Symbol** sym = map_get(&pkg->symbols, (usize)name);
	return sym ? *sym : NULL;
}

Type* resolver_resolve_typedecl(Package* pkg, AstType* type, bool needresolve, bool forceresolve) {
	if(!type)
		return NULL;

	needresolve = forceresolve || needresolve;

	Type* ret = NULL;
	switch(type->kind) {
		case AST_TYPE_NAME: {
			//Symbol* sym = resolver_resolve_name(pkg, type->loc, type->name, needresolve);
			
			break;
		}
		case AST_TYPE_PTR: {
			// TODO: implement
			assert(0);
			break;
		}
		case AST_TYPE_ARRAY: {
			// TODO: implement
			assert(0);
			break;
		}
		case AST_TYPE_FN: {
			Type** args = NULL;
			for(size_t i = 0; i < type->fn.args.len; i++) {
				Type* arg_type = resolver_resolve_typedecl(pkg, type->fn.args.list[i], false, forceresolve);
				buf_push(args, arg_type);
			}
			//Type* rett = resolver_resolve_typedecl(pkg, type->fn.ret, false, forceresolve);
			//ret = type_fn(args, buf_len(args), rett);
			break;
		}
		case AST_TYPE_SLICE: {
			// TODO: implement
			assert(0);
			break;
		}
		case AST_TYPE_TUPLE: {
			// TODO: implement
			assert(0);
			break;
		}
	}
	return ret;
}

void resolver_declare_symbol(Package* pkg, Symbol* sym) {
	if(sym->state != SYMSTATE_INITIAL) {
		if(sym->state == SYMSTATE_DECLARING) {
			resolve_error(sym->decl->loc, "cyclic dependency for symbol '%s'", sym->name);
		}
		assert(sym->state >= SYMSTATE_DECLARED);
		return;
	}
	sym->state = SYMSTATE_DECLARING;
	AstDecl* decl = sym->decl;
	assert(decl); // for not resolved symbols, decl is never null
	switch(decl->kind) {
		case AST_DECL_LET:
		case AST_DECL_CONST:
			// let and const need no declare, only resolve
			break;
		case AST_DECL_FN:
			break;
		case AST_DECL_STRUCT:
			break;
		case AST_DECL_ENUM:
			break;
		case AST_DECL_TYPE:
			break;
	}
	sym->state = SYMSTATE_DECLARED;
	buf_push(pkg->symbol_order, (SymbolOrder){ sym->name, true });
}

void resolver_resolve_symbol(Package* pkg, Symbol* sym) {
	resolver_declare_symbol(pkg, sym);
	if(sym->state != SYMSTATE_DECLARED) {
		if(sym->state == SYMSTATE_RESOLVING) {
			resolve_error(sym->decl->loc, "cyclic dependency for symbol '%s'", sym->name);
		}
		assert(sym->state == SYMSTATE_RESOLVED);
		return;
	}
	sym->state = SYMSTATE_RESOLVING;
	AstDecl* decl = sym->decl;
	assert(decl); // for not resolved symbols, decl is never null
	switch(decl->kind) {
		case AST_DECL_LET:
			if(decl->let.type) {
				resolver_resolve_typedecl(pkg, decl->let.type, true, false);
			}
			break;
		case AST_DECL_CONST:
			break;
		case AST_DECL_FN:
			break;
		case AST_DECL_STRUCT:
			break;
		case AST_DECL_ENUM:
			break;
		case AST_DECL_TYPE:
			break;
	}
	sym->state = SYMSTATE_RESOLVED;
	buf_push(pkg->symbol_order, (SymbolOrder){ sym->name, false });
}

void resolver_resolve_package(Package* pkg) {
	MapSymbols* m = &pkg->symbols;
	for(MapIt it = map_begin(m); it != map_end(m); map_next(m, &it)) {
		Symbol* sym = map_iter_value(m, &it);
		resolver_declare_symbol(pkg, sym);
	}
}
