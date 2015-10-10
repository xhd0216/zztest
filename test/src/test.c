#include "lru_cache.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test.h"
/*
int key_cmp_cbf(const void * key1, const void * key2){
	char * p1 = (char *) key1;
	char * p2 = (char *) key2;
	while(p1 && *p1 && p2 && *p2){
		if(*p1 != *p2) return 0;
		p1++;
		p2++;
	}
	if(*p1 == 0 && *p2 == 0) return 1;
	return 0;
	
}
void key_free_cbf(void * key){
	free(key);
}
int key_clone_cbf(void ** target, const void * source){
	const char * p = source;
	int size = 0;
	while(p && *p){
		size++;
		p++;
	}
	*target = (void *)malloc(size);
	if(!*target){
		printf("%s: fail to allocate memory for key\n", __FUNCTION__);
		return 0;
	}
	memcpy(*target, source, size);
	return size;
}*/
int simple_hash_function(const void * key){
	int res = 0;
	const char * p = ((const test_key_t *)key)->key;
	while(p && *p){
		res  ^= *p;
		p++;
	}
	return res % HASH_MAP_MAX_BUCKETS;

}

typedef struct{
	char value[21];
} test_value_t;

int main(){
	
	printf("ok, it is done\n");
	//lru_cache_t ** lru_p;
	hash_map_t * hash_map;
	int res = hash_map_init(&hash_map, &simple_hash_function, &key_cmp_cbf, &key_free_cbf, & key_clone_cbf);
	printf("init result: %d\n", res);
	int i = 1;
	test_key_t key;
	test_value_t value;
	test_value_t * vp;
	int ret = 0;
	while(i){
		printf("0: quit\n1: insert\n2: lookup\n3: delete\nyour choice:");
		scanf("%d", &i);
		switch(i){
		case 1:
			printf("key:");
			scanf("%19s", key.key);
			printf("got key: %s\nvalue:", key.key);
			scanf("%19s", value.value);
			if(value.value[0] == '0') i=0;
			ret = hash_map_insert(hash_map, (const void *)(&key), (const void *)(&value), &value_clone_cbf, &value_free_cbf); 
			if(!ret){
				printf("insert (%s, %s) failed", key.key, value.value);
			}
			break;
		case 3:
			printf("key:");
			scanf("%19s", key.key);
			ret = hash_map_delete_entry(hash_map, (const void *) &key);
			if(!ret){
				printf("delete failed\n");
			}
			break;
		case 2:
			printf("key:");
			scanf("%19s", key.key);
			ret = hash_map_lookup(hash_map, (const void *) &key, (void **)(&vp));
			if(!ret || !vp){
				printf("key does not exist\n");
			}
			else{
				printf("key=%s, value=%s\n", key_to_string((const void *)&key), value_to_string((const void *)vp));
			}
			break;
		default:
			i = 0;
			break;
		}
		hash_map_dump(hash_map, &key_to_string, &value_to_string);
	}
	hash_map_fini(hash_map);
	printf("quit program\n");
	return 0;
}
