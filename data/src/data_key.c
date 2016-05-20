#include "lru_cache.h"
#include "data_key.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * key_to_string_for_data(const void * key)
{
	data_key_t * k = (data_key_t *)key;
	if(!k || !(k->key_name) || !k->key_name[0]){
		printf("%s: no key\n", __func__);
		return NULL;
	}
	return k->key_name;
}

int hash_function_for_data(const void * key)
{
	const data_key_t * k = (const data_key_t *) key;
	int res = 0;
	const char * c = k->key_name;
	while(c && *c){
		res = res << 1;
		res ^= *c;
		c++;
	}
	c = k->key_time;
	while(c && *c){
		res = res << 1;
		res ^= (*c << 1) + *c;
		c++;
	}
	res -= k->update_period;
	return res;
}
int key_cmp_cbf_for_data(const void * key1,
						 const void * key2)
{
	const data_key_t * k1 = (const data_key_t *) key1;
	const data_key_t * k2 = (const data_key_t *) key2;
	if (!k1->key_name || !k1->key_time || !k2->key_name || !k2->key_time)
	{
		return 0;
	}
	if(k1->update_period != k2->update_period){
		return 0;
	}
	if (strcmp(k1->key_name, k2->key_name) != 0 ||
		strcmp(k1->key_time, k2->key_time) != 0)
	{
		return 0;
	}
	return 1;
}
void * key_clone_cbf_for_data(alloc_t * alloc, const void * source)
{
	if(!alloc || !source){
		printf("%s: input error\n", __func__);
		return 0;
	}
	data_key_t * p = (data_key_t *)zalloc(alloc, sizeof(data_key_t));
	if(!p){
		printf("%s: no memory space for new key\n", __FUNCTION__);
		return NULL;
	}	
	const data_key_t * s = (const data_key_t *) source;
	if (!strncpy(p->key_name, s->key_name, DATA_KEY_NAME_MAX_SIZE) ||
		!strncpy(p->key_time, s->key_time, DATA_KEY_TIME_MAX_SIZE))
	{
		printf("%s: string copy failed\n", __func__);
		key_free_cbf_for_data(alloc, p);
		return NULL;
	} 
	p->update_period = s->update_period;
	return p;
}
void key_free_cbf_for_data(alloc_t * alloc, void * key)
{
	zfree(alloc, key, sizeof(data_key_t));
}


