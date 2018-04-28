// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef enum TypeKind {
	TYPE_VOID,
	TYPE_UNSIGNED,
	TYPE_SIGNED,
	TYPE_BOOLEAN,
	TYPE_FN,
} TypeKind;

struct Type {
	TypeKind kind;
	isize size;
	isize align;
	Symbol* symbol;
	union {
		struct {
			Type** args;
			isize args_len;
			Type* ret;
		} fn;
	};
};

Type* primitive_void = &(Type){ TYPE_VOID, 0, 0 };
Type* primitive_bool = &(Type){ TYPE_BOOLEAN, 0, 0 };

Type* primitive_u8 = &(Type){  TYPE_UNSIGNED, 1, 1 };
Type* primitive_u16 = &(Type){ TYPE_UNSIGNED, 2, 2 };
Type* primitive_u32 = &(Type){ TYPE_UNSIGNED, 4, 4 };
Type* primitive_u64 = &(Type){ TYPE_UNSIGNED, 8, 8 };
Type* primitive_usize = &(Type){ TYPE_UNSIGNED, sizeof(usize), sizeof(usize) };
Type* primitive_i8 = &(Type){  TYPE_SIGNED, 1, 1 };
Type* primitive_i16 = &(Type){ TYPE_SIGNED, 2, 2 };
Type* primitive_i32 = &(Type){ TYPE_SIGNED, 4, 4 };
Type* primitive_i64 = &(Type){ TYPE_SIGNED, 8, 8 };
Type* primitive_isize = &(Type){ TYPE_SIGNED, sizeof(isize), sizeof(isize) };

//Type* primitive_f32 = &(Type){ TYPE_FLOATING, 4, 4 };
//Type* primitive_f64 = &(Type){ TYPE_FLOATING, 8, 4 };

