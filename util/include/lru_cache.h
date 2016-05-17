#include "zalloc.h"
#include "hash_map.h"

typedef struct lru_entry_s{
	struct lru_entry_s * next;
	struct lru_entry_s * prev;
	void * value;
	data_free_cb_f  value_free;
	data_clone_cb_f value_clone;
	hash_map_entry_t * pointer_back;
}lru_entry_t;
typedef struct lru_cache_s{
	alloc_t * alloc;
	/* add a lock */
	hash_map_t * hashmap;
	lru_entry_t * head;
	lru_entry_t * tail;
	int max_entries; // max num of entries
}lru_cache_t;

lru_cache_t *
lru_cache_constuct(alloc_t *,
				hash_map_function,
				key_cmp_fb_f,
				data_free_cb_f,
				data_clone_cb_f);
/* TODO: will destruct lru only, lru->alloc will need to destruct seperately */
void
lru_cache_destruct(lru_cache_t *);
/*
int lru_cache_init(lru_cache_t **, 
				hash_map_function,
				key_cmp_cb_f,
				data_free_cb_f keyf,
				data_clone_cb_fk eyc);
*/
int lru_cache_get(allot_t *,
					lru_cache_t *,
				    const void * key,
					void ** res);
int lru_cache_insert(lru_cache_t *,
					 const void * key,
					 const void * value,
					 data_clone_cb_f valuec,
					 data_free_cb_f valuef);
int lru_delete_auto(lru_cache_t *);
void lru_dump(lru_cache_t *, value_to_string_cb_f, key_to_string_cb_f);
