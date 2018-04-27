// Copyright 2018 Simone Miraglia. See the LICENSE
// file at the top-level directory of this distribution

// Hash map implementation with linear probing
// Usage: 
//   typedef map_type(i32) Map_i32;
//   Map_i32 m = {0};
//   map_set(&m, "key", 10);
//   i32 v = map_get(&m, "key"); // v == 10

typedef struct MapBaseKey {
	u64 hash;
	const char* key;
} MapBaseKey;

typedef struct MapBase {
	MapBaseKey* keys;
	void* values;
	isize len;
	isize cap;
} MapBase;

#define map_type(type) struct { MapBase base; type* ref; type tmp; }

static unsigned map_hash_(const char *str) {
	u64 hash = 5381;
	while(*str) hash = ((hash << 5) + hash) ^ *str++;
	return hash;
}
void map_free_(MapBase* map) {
    free(map->keys);
    free(map->values);
	map->len = 0;
	map->cap = 0;
}
void* map_get_(MapBase* map, const char* key, int elem_size) {
	u64 hash = map_hash_(key);
	isize i = (isize)hash;
	while(true) {
		i &= map->cap - 1;
		if(map->keys[i].hash == hash && strcmp(key, map->keys[i].key) == 0) {
			return ((char*)map->values) + i * elem_size;
		} else if(!map->keys[i].hash) {
			break;
		}
		i++;
	}
	return 0;
}

void map_set_(MapBase* map, const char* key, void* value, int elem_size);

void map_grow_(MapBase* map, isize new_cap, isize elem_size) {
	new_cap = MAX(new_cap, 16);
    MapBase new_map = {
        .keys = xcalloc(new_cap, sizeof(MapBaseKey)),
        .values = xmalloc(new_cap * elem_size),
        .cap = new_cap,
    };
    for(isize i = 0; i < map->cap; i++) {
        if(map->keys[i].hash) {
			map_set_(&new_map, map->keys[i].key, (char*)map->values + i * elem_size, elem_size);
        }
    }
	map_free_(map);
    *map = new_map;
}
void map_set_(MapBase* map, const char* key, void* value, int elem_size) {
	if(2*map->len >= map->cap) {
		map_grow_(map, 2*map->cap, elem_size);
	}
	u64 hash = map_hash_(key);
	isize i = (isize)hash;
	while(true) {
		i &= map->cap - 1;
		if(!map->keys[i].hash) {
			map->len++;
			map->keys[i].hash = hash;
			map->keys[i].key = key;
			memcpy((char*)map->values + i * elem_size, value, elem_size);
			return;
		} else if(map->keys[i].hash == hash && strcmp(key, map->keys[i].key) == 0) {
			memcpy((char*)map->values + i * elem_size, value, elem_size);
			return;
		}
		i++;
	}
}

#define map_get(m, key)  ( (m)->ref = map_get_(&(m)->base, key, sizeof((m)->tmp)) )
#define map_set(m, key, value)  ( (m)->tmp = (value), map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)) )
#define map_free(m)  map_free_(&(m)->base)