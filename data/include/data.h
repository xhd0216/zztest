#include "data_key.h"
#include "data_value.h"
#include "lru_cache.h"
int lru_cache_init_wrap(lru_cache_t ** lru){
	return lru_cache_init_wrap(lru, 
							   &hash_function_for_data,
							   &key_cmp_cbf_for_data,
							   &key_clone_cbf_for_data);
}

int lru_cache_get_wrap(lru_cache_t * lru, const data_key_t * key,
						data_value_t ** target){
	if(!lru || !key || !target){
		printf("%s: input error\n", __FUNCTION);
		return 0;
	}
	void * tmp = 0;
	int res = lru_cache_get(lru, (const void *)key, &tmp);
	if(!res || !tmp){
		*target = 0;
		return 0;
	}
	*target = (data_value_t *)tmp;
	return 1;
}

int lru_cache_insert_wrap(lru_cache_t * lru, const data_key_t * key,
							const data_value_t * value){
	return lru_cache_insert_wrap(lru, (const void *) key, 
								(const void *) value,
								&value_clone_cbf_for_data,
								&value_free_cbf_for_data);

}
void lru_dump_wrap(lru_cache_t * lru){
	lru_dump(lru, &value_to_string_for_data, &key_to_string_for_data);
}
