// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

typedef struct MemoryPool {
	void* ptr;
    void* end;
    void** blocks;
} MemoryPool;

#define MEMORY_POOL_ALIGNMENT 8
#define MEMORY_POOL_BLOCK_SIZE 1024

void mpool_grow(MemoryPool* p, isize min_size) {
	isize size = ALIGN_UP(MAX(MEMORY_POOL_BLOCK_SIZE, min_size), MEMORY_POOL_ALIGNMENT);
	p->ptr = xmalloc(size);
	p->end = (char*)p->ptr + size;
	buf_push(p->blocks, p->ptr);
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
	for(int i = 0; i < buf_len(p->blocks); i++) {
		free(p->blocks[i]);
	}
	buf_free(p->blocks);
}
