#include "lru_cache.h"
#include "data_key.h"

int hash_function_for_data(const void * key){
	const data_key_t * k = (const data_key_t *) key;
	int res = 0;
	char * c = k->key_name;
	while(c && *c){
		res = res << 1;
		res ^= *c;
		c++;
	}
	c = k->key_time;
	while(c && *c){
		res = res << 1;
		res ^= (*c << 1 + *c);
		c++;
	}
	res -= k->update_period;
	return res % HASH_MAP_MAX_BUCKETS;
}
int key_cmp_function_for_data(const void * key1,
							  const void * key2){
	const data_key_t * k1 = (const data_key_t *) key1;
	const data_key_t * k2 = (const data_key_t *) key2;
	char * c1 = k1->key_name;
	char * c2 = k2->key_name;
	if(k1->update_period != k2->update_period){
		return 0;
	}
	int count = 0;
	while(count < DATA_KEY_NAME_MAX_SIZE && c1 && c2 && *c1 && *c2){
		if(*c1 != *c2) return 0;
		c1++;
		c2++;
		count++;
	}
	if(count <= DATA_KEY_NAME_MAX_SIZE && *c1 != *c2){
		return 0;
	}
	c1 = k1->key_time;
	c2 = k2->key_time;
	count = 0;
	while(count < DATA_KEY_TIME_SIZE && c1 && c2 && *c1 && *c2){
		if(*c1 != *c2) return 0;
		c1++;
		c2++;
		count++;
	}
	if(count <= DATA_KEY_TIME_SIZE && *c1 != *c2){
		return 0;
	}
	return 1;
}
int key_clone_function_for_data(void ** target, const void * source){
	if(!target || !source){
		printf("%s: input error\n", __FUNCTION__);
		return 0;
	}
	data_key_t * p = (data_key_t *)malloc(sizeof(data_key_t));
	if(!p){
		printf("%s: no memory space for new key\n", __FUNCTION__);
		return 0;
	}	
	const data_key_t * s = (const data_key_t *) source;
	if(!strncpy(p->key_name, s->key_name, DATA_KEY_NAME_MAX_SIZE)){
		printf("%s: string copy failed\n", __FUNCTION__);
		free(p);
		return 0;
	} 
	if(!strncpy(p->key_time, s->key_time, DATA_KEY_TIME_SIZE)){
		printf("%s: string copy failed\n", __FUNCTION__);
		free(p);
		return 0;
	}
	p->update_period = s->update_period;
	*target = (void *)p;
	return 1;
}
void key_free_function_for_data(void * key){
	free((data_key_t *)key);
}
int hash_map_init_wrap(hash_map_t ** hm){
	return hash_map_init(hm, 
						 &hash_function_for_data, 
						 &key_cmp_function_for_data,
						 &key_free_function_for_data,
						 &key_clone_function_for_data);
}


