typedef int (*value_clone_cb_f)(void ** target, const void * source);
typedef void (*value_free_cb_f)(void *);
typedef int (*key_cmp_cb_f)(const void *, const void *);
typedef int (*key_clone_cb_f)(void *, const void *);
typedef void (*key_free_cb_f)(void *);

typedef struct hash_map_entry_s{
	void * key;
	void * value;
	int key_size;
	value_clone_cb_f * value_clone;
	value_free_cb_f * value_free;
	struct hash_map_entry_s * next;
	struct hash_map_entry_s * prev;
}hash_map_entry_t;

#define HASH_MAP_MAX_BUCKETS 100

typedef int (*hash_map_function)(const void * key);
typedef struct hash_map_s{
	hash_map_function * hash_f;
	key_cmp_cb_f * key_cmp;
	key_clone_cb_f * key_clone;
	key_free_cb_f * key_free;
	hash_map_entry_t * table[HASH_MAP_MAX_BUCKETS];
}hash_map_t;

int hash_map_delete(hash_map_t *,
					const void *,
					int);
/* be careful of the call back function:
 * could cbf leak memory? */
/* clone value using call back function */
int hash_map_lookup(hash_map_t *,
					const void * key,
					//int key_size,
					void ** value);
					//value_clone_cb_f);
int hash_map_init(hash_map_t **,
				hash_map_function);
/* copy value using call back function */
int hash_map_insert(hash_map_t *,
					const void * key,
					//int key_size,
					const void * value);
					//value_clone_cb_f);
