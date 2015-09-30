/*
typedef struct key_s{
	void * key;
	int size;
}key_t;

typedef struct value_s{
	void * value;
	int size;
}value_t;*/


/* callback function:
 * copy value to (*target)
 * return the size copied
 */
typedef int (*value_clone_cb_f)(void ** target, const void * source);

typedef struct hash_map_entry_s{
	void * key;
	void * value;
	int key_size;
	int value_size;	
	struct hash_map_entry_s * next;
	struct hash_map_entry_s * prev;
}hash_map_entry_t;

#define HASH_MAP_MAX_BUCKETS 100

typedef int (*hash_map_function)(const void * key, int);
typedef struct hash_map_s{
	hash_map_function hash_f;
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
					int key_size,
					void ** value,
					value_clone_cb_f);
int hash_map_init(hash_map_t **,
				hash_map_function);
/* copy value using call back function */
int hash_map_insert(hash_map_t *,
					const void * key,
					int key_size,
					const void * value,
					value_clone_cb_f);
