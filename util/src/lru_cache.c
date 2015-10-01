#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hash_value_clone_cb(void ** target, const void * pointer){
	if(!target) return 0;
	*target = (void *)pointer;
	return sizeof(lru_entry_t*);
}

hash_map_entry_t * 
lru_insert_to_hash_map(hash_map_t * hashmap,
					const void * key,
					int key_size,
					const lru_entry_t * point){
	if(!hashmap ||key_size < 1|| !key || !point){
		return 0;
	}
	hash_map_entry_t * res = (hash_map_entry_t *) malloc(sizeof(hash_map_entry_t));
	if(!res){
		printf("%s: fail to allocate memory\n", __FUNCTION__);
		return 0;
	}
	/*key copy*/
	memcpy(res->key, key, key_size);
	res->key_size = key_size;
	res->value = (void *)point;
	
	int buc = hashmap->hash_f(key, key_size);
	hash_map_entry_t * p = hashmap->table[buc];
	while(p->next){
		p = p->next;
	}
	res->next = p->next;
	if(p->next) p->next->prev = res;
	p->next = res;
	res->prev = p;
	return res;
}

int lru_cache_init(lru_cache_t ** lru,
				hash_map_function hashfunction){
	if(!lru) return 0;
	*lru = (lru_cache_t *) malloc(sizeof(lru_cache_t));
	if(!*lru){
		printf("init: fail to allocate memory for lru\n");
		return 0;
	}
	lru_cache_t * lp = *lru;
	lp->head = (lru_entry_t*)malloc(sizeof(lru_entry_t));
	lp->tail = (lru_entry_t*)malloc(sizeof(lru_entry_t));
	if(!((*lru)->head) || !((*lru)->tail)){
		printf("init: fail to allocate memory\n");
		free(*lru);
		*lru = 0;
		return 0;
	}
	int res = hash_map_init(&((*lru)->hashmap), hashfunction);
	if(!res || !((*lru)->hashmap)){
		printf("init: fail to allocate memory for hashmap\n");
		free((*lru)->head);
		free((*lru)->tail);
		free(*lru);
		*lru =0;
		return 0;
	}
	(*lru)->head->prev = 0;
	(*lru)->head->next = (*lru)->tail;
	(*lru)->tail->prev = (*lru)->head;
	(*lru)->tail->next = 0;
	return 1;
}


int lru_cache_insert(lru_cache_t * lru,
					const void * key,
					int key_size,
					const void * value,
					//int value_size,
					lru_value_copy_cb_f value_clone){

	if(!lru || !key || key_size == 0 || !value) {
		return 0;
	}
	int s = 0;
	lru_entry_t * p;
	s = hash_map_lookup(lru->hashmap, key, key_size,  (void **)(&p), hash_value_clone_cb);
	if(!p || !s){
		printf("%s: key doesn't exist, create new one\n", __FUNCTION__);
		p = (lru_entry_t *) malloc(sizeof(lru_entry_t));
		if(!p){
			printf("%s: error allocating memory\n", __FUNCTION__);
			return 0;
		}
		p->next = 0;
		p->prev = 0;
		p->value = 0;	
		hash_map_entry_t * resp = lru_insert_to_hash_map(lru->hashmap,
									key, key_size,
									(const lru_entry_t *) p);
		if(resp == 0){
			printf("%s: error inserting to hash map\n", __FUNCTION__);
			free(p);
			return 0;
		}
		p->pointer_back = resp;		
	}
	/* copy value */
	if(p->value){
		free(p->value);/*delete original content -- should have call back*/
	}
	/*p->value = (void *) malloc(value_size);
	if(p->value ==0){
		printf("%s: fail to allocate memory\n", __FUNCTION__);
		free(p);
		return 0;
	}*/
	p->value = 0;
	int res = value_clone((void **)(&(p->value)), value);
	if(!res || !(p->value)){
		/* fails to clone value to p */
		printf("%s: fail to clone value\n", __FUNCTION__);
		free(p);
		return 0;
	}
	//p->value_size = value_size;
	/* move pointer to the front of list */
	if(p->prev) p->prev->next = p->next;
	if(p->next) p->next->prev = p->prev;
	p->next = lru->head->next;
	lru->head->next->prev = p;
	lru->head->next = p;
	p->prev = lru->head;
	return 1;
}

int lru_cache_get(lru_cache_t * lru,
				  const void * key,
				  int size,
				  void ** value,
				  lru_value_copy_cb_f value_clone){
	if(!lru || !key || size<1 || !value) {
		return 0;
	}
	int s = 0;
	lru_entry_t * p = 0;
	s=hash_map_lookup(lru->hashmap, key, size,  (void **)(&p), hash_value_clone_cb);
	if(!p || !s){
		printf("%s: key doesn't exist\n", __FUNCTION__);
		return 0;
	}
	/* move pointer to the front of list */
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = lru->head->next;
	lru->head->next->prev = p;
	lru->head->next = p;
	p->prev = lru->head;
	/* copy value then exit */
	int res = value_clone(value, (const void *)(p->value));
	if(res<1 || !*value){
		printf("%s: cannot allocate memory\n", __FUNCTION__);
		return 0;
	}
	//memcpy(*value, p->value, p->value_size);
	return 1;
}
