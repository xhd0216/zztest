#ifndef HASH_MAP_DEFINITIONS
#define HASH_MAP_DEFINITIONS
typedef int (*value_clone_cb_f)(void **, const void * source);
typedef void (*value_free_cb_f)(void *);
typedef int (*key_cmp_cb_f)(const void *, const void *);
typedef int (*key_clone_cb_f)(void **, const void *);
typedef void (*key_free_cb_f)(void *);
typedef char * (*key_to_string_cb_f)(const void *);
typedef char * (*value_to_string_cb_f)(const void *);
typedef struct hash_map_entry_s{
	void * key;
	void * value;
	value_clone_cb_f  value_clone;
	value_free_cb_f  value_free;
	struct hash_map_entry_s * next;
	struct hash_map_entry_s * prev;
}hash_map_entry_t;

#define HASH_MAP_MAX_BUCKETS 7

typedef int (*hash_map_function)(const void * key);
typedef struct hash_map_s{
	/* I put all key-related functions on the hash_map_t
	 * Because I want the keys to have the same format
	 * at least, the hash map should know how to compare two keys
	 * it is hard to implement a cmp function of two keys in 
	 * different format
	 */
	hash_map_function  hash_f;
	key_cmp_cb_f  key_cmp;
	key_clone_cb_f  key_clone;
	key_free_cb_f  key_free;
	hash_map_entry_t * table[HASH_MAP_MAX_BUCKETS];
}hash_map_t;

int hash_map_delete_entry(hash_map_t *,
					const void *);
/* be careful of the call back function:
 * could cbf leak memory? */
/* clone value using call back function */
int hash_map_lookup(hash_map_t *,
					const void * key,
					void ** value);
int hash_map_init(hash_map_t **,
				hash_map_function,
				key_cmp_cb_f,
				key_free_cb_f,
				key_clone_cb_f);
/* copy value using call back function */
int hash_map_insert(hash_map_t *,
					const void * key,
					const void * value,
					value_clone_cb_f,
					value_free_cb_f);
void hash_map_dump(hash_map_t *,
				key_to_string_cb_f,
				value_to_string_cb_f);
void hash_map_fini(hash_map_t *);
#endif
