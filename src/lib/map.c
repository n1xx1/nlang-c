// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

// Hash map implementation with linear probing
// Based on https://github.com/pervognsen/bitwise/blob/master/ion/common.c
// and https://github.com/rxi/map, supports different key types.
// Usage: 
//   typedef map_type(const char*, i32) Map_str_i32;
//   Map_str_i32 m = {0};
//   map_set(&m, "key", 10);
//   i32 v = map_get(&m, "key"); // v == 10
//   map_free(&m);

typedef struct MapBaseKey {
	u64 hash;
	char key[0];
} MapBaseKey;

typedef struct MapBase {
	void* keys;
	void* values;
	isize len;
	isize cap;
} MapBase;

u64 map_hash_str(const char *str) {
	u64 hash = 5381;
	while(*str) hash = ((hash << 5) + hash) ^ *str++;
	return hash;
}
u64 map_hash_u64(u64 x) {
	x *= 0xff51afd7ed558ccd;
	x ^= x >> 32;
	return x;
}

void map_free_(MapBase* map) {
	free(map->keys);
	free(map->values);
	map->len = 0;
	map->cap = 0;
}

#define MAP_FUNCTIONS(name, K, hashfn, cmpfn) \
void* name ## _get_(MapBase* map, K key, int vsize) { \
	u64 hash = hashfn(key); \
	isize i = (isize)hash; \
	while(true) { \
		i &= map->cap - 1; \
		MapBaseKey* bkey = (MapBaseKey*)((char*)map->keys + (sizeof(MapBaseKey) + sizeof(K)) * i); \
		if(bkey->hash == hash && cmpfn(key, *(K*)bkey->key) == 0) { \
			return ((char*)map->values) + i * vsize; \
		} else if(!bkey->hash) { \
			break; \
		} \
		i++; \
	} \
	return 0; \
} \
void name ## _set_(MapBase* map, K key, void* value, int vsize); \
void name ## _grow_(MapBase* map, isize new_cap, isize vsize) { \
	new_cap = MAX(new_cap, 16); \
	MapBase new_map = { xcalloc(new_cap, sizeof(MapBaseKey) + sizeof(K)), xmalloc(new_cap * vsize), 0, new_cap}; \
	for(isize i = 0; i < map->cap; i++) { \
		MapBaseKey* bkey = (MapBaseKey*)((char*)map->keys + (sizeof(MapBaseKey) + sizeof(K)) * i); \
		if(bkey->hash) { \
			name ## _set_(&new_map, *(K*)bkey->key, (char*)map->values + i * vsize, vsize); \
		} \
	} \
	map_free_(map); \
	*map = new_map; \
} \
void name ## _set_(MapBase* map, K key, void* value, int vsize) { \
	if(2 * map->len >= map->cap) { \
		name ## _grow_(map, 2 * map->cap, vsize); \
	} \
	u64 hash = hashfn(key); \
	isize i = (isize)hash; \
	while(true) { \
		i &= map->cap - 1; \
		MapBaseKey* bkey = (MapBaseKey*)((char*)map->keys + (sizeof(MapBaseKey) + sizeof(K)) * i); \
		if(!bkey->hash) { \
			map->len++; \
			bkey->hash = hash; \
			*(K*)bkey->key = key; \
			memcpy((char*)map->values + i * vsize, value, vsize); \
			return; \
		} else if(bkey->hash == hash && cmpfn(key, *(K*)bkey->key) == 0) { \
			memcpy((char*)map->values + i * vsize, value, vsize); \
			return; \
		} \
		i++; \
	} \
} \
void name ## _remove_(MapBase* map, K key) { \
	u64 hash = hashfn(key); \
	isize i = (isize)hash; \
	while(true) { \
		i &= map->cap - 1; \
		MapBaseKey* bkey = (MapBaseKey*)((char*)map->keys + (sizeof(MapBaseKey) + sizeof(K)) * i); \
		if(bkey->hash == hash && cmpfn(key, *(K*)bkey->key) == 0) { \
			bkey->hash = 0; \
		} else if(!bkey->hash) { \
			return; \
		} \
		i++; \
	} \
}
#define MAP_COMPARE_INTEGER(a, b) ((a) == (b) ? 0 : 1)


MAP_FUNCTIONS(map_str, const char*, map_hash_str, strcmp)
MAP_FUNCTIONS(map_u64, u64, map_hash_u64, MAP_COMPARE_INTEGER)


#define GENERIC_MAP_FUNC(typ, fn) _Generic((typ), \
	const char*: map_str_ ## fn ## _, \
	u64: map_u64_ ## fn ## _)

#define map_type(K, V) struct { MapBase base; K* kref; V* ref; V tmp; }
#define map_get(m, key)  ( (m)->ref = GENERIC_MAP_FUNC(*(m)->kref, get)(&(m)->base, key, sizeof((m)->tmp)) )
#define map_set(m, key, value)  ( (m)->tmp = (value), GENERIC_MAP_FUNC(*(m)->kref, set)(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)) )
#define map_remove(m, key) ( GENERIC_MAP_FUNC(*(m)->kref, remove)(&(m)->base, key) )
#define map_free(m)  map_free_(&(m)->base)
