#include "hash_map.h"
typedef struct lru_entry_s{
	struct lru_entry_s * next;
	struct lru_entry_s * prev;
	void * value;
	value_free_cb_f * value_free;	
	hash_map_entry_t * pointer_back;
}lru_entry_t;
typedef struct lru_cache_s{
	hash_map_t * hashmap;
	lru_entry_t * head;
	lru_entry_t * tail;
}lru_cache_t;


int lru_cache_init(lru_cache_t **, 
				hash_map_function *,
				key_cmp_cb_f *,
				key_free_cb_f *,
				key_clone_cb_f *);

int lru_cache_get(lru_cache_t *,
				  const void * key,
				  void ** target,
				  value_clone_cb_f *);
int lru_cache_insert(lru_cache_t *,
					 const void * key,
					 const void * value,
					 value_clone_cb_f *,
					 value_free_cb_f *);
int lru_delete_auto(lru_cache_t *);
