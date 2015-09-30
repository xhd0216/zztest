#include "hash_map.h"
typedef struct lru_entry_s{
	struct lru_entry_s * next;
	struct lru_entry_s * prev;
	void * value;
	int value_size;
	hash_map_entry_t * pointer_back;
}lru_entry_t;
typedef struct lru_cache_s{
	hash_map_t * hashmap;
	lru_entry_t * head;
	lru_entry_t * tail;
}lru_cache_t;

typedef int (*lru_value_copy_cb_f)(void ** target, const void * value);

int lru_cache_init(lru_cache_t **, hash_map_function);

int lru_cache_get(lru_cache_t *,
				  const void * key,
				  int size,
				  void ** target,
				  lru_value_copy_cb_f);
int lru_cache_insert(lru_cache_t *,
					 const void * key,
					 int key_size,
					 const void * value,
//					 int value_size,
					 lru_value_copy_cb_f);
