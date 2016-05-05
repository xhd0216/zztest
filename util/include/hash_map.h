#ifndef HASH_MAP_DEFINITIONS
#define HASH_MAP_DEFINITIONS
#include "zalloc.h"
//typedef int (*value_clone_cb_f)(void **, const void * source);
//typedef void (*value_free_cb_f)(void *);
//typedef int (*key_clone_cb_f)(void **, const void *);
//typedef void (*key_free_cb_f)(void *);
typedef char * (*key_to_string_cb_f)(const void *);
typedef char * (*value_to_string_cb_f)(const void *);


//typedef struct alloc_s alloc_t;
/* struct key_s should have a clone and a free method */
//typedef struct key_s key_t;

/*struct value_s should have a clone and a free method */
//typedef struct value_s value_t;

/* function to clone a value, first parameter is the allocator */
typedef void * (*data_clone_cb_f)(alloc_t * alloc, const key_t * source);
typedef void   (*data_free_cb_f)(alloc_t *, void *);
typedef int    (*key_cmp_cb_f)(const void *, const void *);
typedef char * (*data_to_string_cb_f)(const void *);

typedef struct hash_map_entry_s{
	void * key;
	void * value;
	/* the value in each entry could have different format
	 * but keys must have the same format
	 */
	data_clone_cb_f value_clone;
	data_free_cb_f  value_free;
	struct hash_map_entry_s * next;
	struct hash_map_entry_s * prev;
}hash_map_entry_t;

#define HASH_MAP_MAX_BUCKETS 7

typedef int (*hash_map_function)(const void * key);
typedef struct hash_map_s{
	/* I put all key-related functions on the hash_map_t.
	 * Because I want the keys to have the same format.
	 * the hash map should know how to compare two keys.
	 * it is hard to implement a cmp function of two keys in 
	 * different format
	 */
	alloc_t             * alloc;
	// need a lock here;
	hash_map_function   hash_f;
	key_cmp_cb_f        key_cmp;
	data_clone_cb_f     key_clone;
	data_free_cb_f      key_free;
	hash_map_entry_t    ** table;
	int                 size; /* number of buckets in table */
	int                 entries; /* number of entries in hash map */
}hash_map_t;

int hash_map_delete_entry(hash_map_t *,
					      const void * key);
/* XXX:be careful of the call back function:
 * could cbf read memory to get other entries?
 * that's fine. the process using this library 
 * should have full control of this hash map.
 */

/* clone value using call back function */
int hash_map_lookup(hash_map_t *,
					const void * key,
					void ** value);
/*int hash_map_init(hash_map_t **,
				  hash_map_function,
				  key_cmp_cb_f,
				  data_free_cb_f,
				  data_clone_cb_f);*/
hash_map_t * hash_map_construct(alloc_t *,
								int,
								hash_map_function,
								key_cmp_cb_f,
								data_free_cb_f,
								data_clone_cb_f);
/* copy value using call back function */
int hash_map_insert(hash_map_t *,
					const void * key,
					const void * value,
					data_clone_cb_f,
					data_free_cb_f);
void hash_map_dump(hash_map_t *,
				key_to_string_cb_f,
				data_to_string_cb_f);
void hash_map_destruct(hash_map_t *);
#endif
