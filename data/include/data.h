#include "data_key.h"
#include "data_value.h"
#include "lru_cache.h"
#include <stdio.h>

lru_cache_t * 
data_lru_cache_construct_wrap(alloc_t * alloc, int size){
	return lru_cache_construct(alloc,
						   size,
						   &hash_function_for_data,
						   &key_cmp_cbf_for_data,
						   &key_free_cbf_for_data,
						   &key_clone_cbf_for_data);
}

data_value_t *
data_lru_cache_get_wrap(alloc_t * alloc,
						lru_cache_t * lru, 
						const data_key_t * key)
{
	if(!lru || !key){
		printf("%s: input error\n", __func__);
		return NULL;
	}
	void * tmp = lru_cache_get(alloc, lru, (const void *)key);
	if(!tmp){
		return NULL;
	}
	return (data_value_t *)tmp;
}

int data_lru_cache_insert_wrap(lru_cache_t * lru, 
							const data_key_t * key,
							const data_value_t * value){
	return lru_cache_insert(lru, 
							(const void *) key, 
							(const void *) value,
							&value_clone_cbf_for_data,
							&value_free_cbf_for_data);
}

void lru_dump_wrap(lru_cache_t * lru){
	lru_dump(lru, &value_to_string_for_data, &key_to_string_for_data);
}
