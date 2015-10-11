#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void lru_cache_fini(lru_cache_t *lru){
	if(!lru){
		return;
	}
	lru_entry_t * p = lru->head;
	lru_entry_t * next;
	while(p){
		next = p->next;
		p->value_free(p->value);
		free(p);
		p = next;
	}
	hash_map_fini(lru->hashmap);
	

}
void lru_dump(lru_cache_t * lru, value_to_string_cb_f vts, key_to_string_cb_f kts){
	if(!lru){
		printf("empty string\n");
		return;
	}
	lru_entry_t * p = lru->head->next;
	hash_map_entry_t * pb;
	while(p && p!=lru->tail){
		pb = p->pointer_back;
		printf("===========\n");
		if(!pb){
			printf("Error: cannot find hash map entry\n");
		}
		else if(!pb->key){
			printf("Error: cannot find key\n");
		}
		else{
			printf("key: %s\n", kts((const void *)pb->key));
		}
		if(!p->value){
			printf("Error: Cannot find value\n");
		}
		else{
			printf("value: %s\n", vts((const void *)p->value));
		}
		p = p->next;
	}
	printf("============\n");
}
int hash_value_clone_cb(void ** target, const void * pointer){
	if(!target) return 0;
	*target = (void *)pointer;
	return 1;
}


int lru_delete_auto(lru_cache_t * lru){
	if(!lru || !lru->tail || !lru->head || lru->tail->prev == lru->head){
		return 0;
	}
	lru_entry_t * p = lru->tail->prev;
	p->prev->next = lru->tail;
	lru->tail->prev = p->prev;
	hash_map_entry_t * pb = p->pointer_back;
	/* should assert pb->value == p*/
	if(pb){
		if(pb->prev){
			pb->prev->next = pb->next;
			if(pb->next){
				pb->next->prev = pb->prev;
			}
		}
		/* free pb->key*/
		(*(lru->hashmap->key_free))(pb->key);
		/* do NOT free pb->value, because pb->value == p*/
		free(pb);
	}
	(p->value_free)(p->value);
	free(p);
	return 1;
}

void lru_free_nothing(void * foo){
	/* this function should do nothing! */
	printf("%s: we do nothing here -- we don't free the lru entry\n", __FUNCTION__);
}
/*make sure key does not exist in hashmap!!!*/
hash_map_entry_t * 
lru_insert_to_hash_map(hash_map_t * hashmap,
					const void * key,
					const lru_entry_t * point){
	if(!hashmap || !key || !point){
		return 0;
	}
	int buc = (*(hashmap->hash_f))(key);
	hash_map_entry_t * p = hashmap->table[buc];
	if(!p){
		/*error in hashmap->table[buc]*/
		printf("%s: hashmap->table[%d] is NULL\n", __FUNCTION__, buc);
		return 0;
	}
	hash_map_entry_t * res = (hash_map_entry_t *) malloc(sizeof(hash_map_entry_t));
	if(!res){
		printf("%s: fail to allocate memory\n", __FUNCTION__);
		return 0;
	}
	/* key clone */
	(*(hashmap->key_clone))(&(res->key), key);
	printf("%s: key cloned\n", __FUNCTION__);
	/* important here!!!*/
	res->value = (void *)point;
	(res->value_clone) = &hash_value_clone_cb;
	(res->value_free) = &lru_free_nothing;
 
	res->next = p->next;
	if(p->next) p->next->prev = res;
	p->next = res;
	res->prev = p;
	return res;
}
int lru_cache_init(lru_cache_t ** lru,
				hash_map_function hashfunction,
				key_cmp_cb_f key_cmp,
				key_free_cb_f key_free,
				key_clone_cb_f key_clone){
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
	int res = hash_map_init(&((*lru)->hashmap), hashfunction, key_cmp, key_free, key_clone);
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
					const void * value,
					value_clone_cb_f value_clone,
					value_free_cb_f value_free){

	if(!lru || !key  || !value) {
		return 0;
	}
	int s = 0;
	lru_entry_t * p;
	s = hash_map_lookup(lru->hashmap, key,  (void **)(&p));
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
									key,
									(const lru_entry_t *) p);
		if(resp == 0){
			printf("%s: error inserting to hash map\n", __FUNCTION__);
			free(p);
			return 0;
		}
		p->pointer_back = resp;		
	}
	void * new_value = 0;
	int res = value_clone((void **)(&(new_value)), value);
	if(!res || !(new_value)){
		/* fails to clone value to p */
		printf("%s: fail to clone value\n", __FUNCTION__);
		free(p);
		return 0;
	}
	printf("%s: value cloned\n", __FUNCTION__);
	/* copy value */
	if(p->value){
		printf("%s: discard old value\n", __FUNCTION__);
		p->value_free(p->value);
	}
	p->value = new_value;
	p->value_free = value_free;
	p->value_clone = value_clone;
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
				  void ** value){
	if(!lru || !key ||  !value) {
		return 0;
	}
	int s = 0;
	lru_entry_t * p = 0;
	s=hash_map_lookup(lru->hashmap, key, (void **)(&p));
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
	int res = p->value_clone(value, (const void *)(p->value));
	if(res<1 || !*value){
		printf("%s: cannot allocate memory\n", __FUNCTION__);
		return 0;
	}
	//memcpy(*value, p->value, p->value_size);
	return 1;
}
