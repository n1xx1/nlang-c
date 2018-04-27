// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

// Memory pool implementation.
// allocations are 8 byte aligned; MemoryPool::ptr is a 
// pointer to the data but it actually follow a pointer 
// to the previous block that is used for later clearing 
// the pool

typedef struct MemoryPool {
	void* begin;
	void* ptr;
	void* end;
} MemoryPool;

#define MEMORY_POOL_ALIGNMENT 8
#define MEMORY_POOL_BLOCK_SIZE 1024

void mpool_grow(MemoryPool* p, isize min_size) {
	isize size = ALIGN_UP(MAX(MEMORY_POOL_BLOCK_SIZE, min_size), MEMORY_POOL_ALIGNMENT);
	void* new_ptr = xmalloc(sizeof(void*) + size);
	p->end = (char*)new_ptr + sizeof(void*) + size;
	*(void**)new_ptr = p->begin;
	p->begin = (void**)new_ptr + 1;
	p->ptr = p->begin;
}

void* mpool_alloc(MemoryPool* p, isize size) {
	if(size > (isize)((char*)p->end - (char*)p->ptr)) {
		mpool_grow(p, size);
	}
	void* ptr = p->ptr;
	p->ptr = ALIGN_UP_PTR((char*)p->ptr + size, MEMORY_POOL_ALIGNMENT);
	return ptr;
}

void mpool_free(MemoryPool* p) {
	void* next = p->begin;
	while(next) {
		void* tmp = (char*)next - sizeof(void*);
		next = *(void**)tmp;
		free(tmp);
	}
}
