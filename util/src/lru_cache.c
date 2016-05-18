#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* XXX: need to destruct lru->alloc outside this function */
alloc_t * lru_cache_destruct(lru_cache_t *lru){
	if(!lru){
		return;
	}
	lru_entry_t * p = lru->head;
	lru_entry_t * next;
	while(p){
		next = p->next;
		p->value_free(lru->alloc, p->value);
		zfree(lru->alloc, p, sizeof(lru_entry_t));
		p = next;
	}
	hash_map_destruct(lru->hashmap);
	alloc_t * res = lru->alloc;
	zfree(lru->alloc, lru, sizeof(lru_cache_t));
	return res;
}
void lru_dump(lru_cache_t * lru, value_to_string_cb_f vts, key_to_string_cb_f kts){
	if(!lru){
		printf("error cache parameter\n");
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


int lru_remove_least_used(lru_cache_t * lru){
	if(!lru || !lru->tail || !lru->head || lru->tail->prev == lru->head){
		return 0;
	}
	lru_entry_t * p = lru->tail->prev;
	p->prev->next = lru->tail;
	lru->tail->prev = p->prev;
	hash_map_entry_t * pb = p->pointer_back;
	/* should assert pb->value == p*/
	if (!pb || pb->value != p) {
		printf("%s: error: records in linkedlist and hashmap don't match\n", __func__);
	}
	if(pb){
		if(pb->prev){
			pb->prev->next = pb->next;
			if(pb->next){
				pb->next->prev = pb->prev;
			}
		}
		hash_map_free_entry(lru->hashmap, pb);
	}
	(p->value_free)(lru->alloc, p->value);
	zfree(lru->alloc, p, sizeof(lru_entry_t));
	return 1;
}

void lru_free_nothing(alloc_t * a, void * foo){
	/* this function should do nothing! */
	printf("%s: we do nothing here -- we don't free the lru entry\n", __FUNCTION__);
}

void * hash_value_clone_cb(alloc_t * alloc, const void * pointer){
	if(!alloc) return 0;
	/* very tricky here
	 * the goal of hash value clone cbf is to clone the content of pointer (*pointer) to a (void *)v and let *target=v
	 * but here, the hash entry value is a pointer to lru_entry_t
	 */	
	return (void *) pointer;
}

#if 0
// we should use int hash_map_insert

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
	hash_map_entry_t * res = (hash_map_entry_t *)zalloc(hash->alloc, 
														sizeof(hash_map_entry_t));
	if(!res){
		printf("%s: fail to allocate memory\n", __FUNCTION__);
		return 0;
	}
	/* key clone */
	(*(hashmap->key_clone))(&(res->key), key);
	printf("%s: key cloned\n", __FUNCTION__);
	/* important here!!!*/
	/* hashmap entry's value is the pointer to lru entry*/
	(res->value) = (void *)point;
	(res->value_clone) = &hash_value_clone_cb;
	(res->value_free) = &lru_free_nothing;
 
	res->next = p->next;
	if(p->next) p->next->prev = res;
	p->next = res;
	res->prev = p;
	return res;
}
#endif /*zero*/
lru_cache_t * lru_cache_construct(alloc_t * alloc,
				int sz,
				hash_map_function hashfunction,
				key_cmp_cb_f key_cmp,
				data_free_cb_f key_free,
				data_clone_cb_f key_clone){
	if (!alloc || sz<1 || hashfunction || key_cmp || key_free || key_clone) {
		printf("%s: invalid argument\n", __func__);
		return NULL;
	}
	/* lru_cache_t is allocated by zalloc, like hash_map_construct */
	lru_cache_t * ret = (lru_cache_t *) zalloc(alloc, sizeof(lru_cache_t));
	if(!ret){
		printf("init: fail to allocate memory for lru\n");
		return ret;
	}
	ret->alloc = alloc;
	ret->head = (lru_entry_t*)zalloc(ret->alloc, sizeof(lru_entry_t));
	ret->tail = (lru_entry_t*)zalloc(ret->alloc, sizeof(lru_entry_t));
	if(!ret->head || !ret->tail){
		printf("init: fail to allocate memory\n");
		lru_cache_destruct(ret);
		return NULL;
	}
	//XXX: hashmap and lru will use the same alloc
	ret->hashmap = hash_map_construct(ret->alloc, sz, hashfunction, key_cmp, key_free, key_clone);
	if(!ret->hashmap){
		printf("init: fail to allocate memory for hashmap\n");
		lru_cache_destruct(ret);
		return NULL;
	}
	ret->head->prev = NULL;
	ret->head->next = ret->tail;
	ret->tail->prev = ret->head;
	ret->tail->next = NULL;
	return ret;
}


int lru_cache_insert(lru_cache_t * lru,
					const void * key,
					const void * value,
					data_clone_cb_f value_clone,
					data_free_cb_f value_free)
{
	if(!lru || !key  || !value) {
		return 0;
	}
	lru_entry_t * p;
	int existed = 0;
	hash_map_entry_t * hashnode = hash_map_lookup_entry(lru->hashmap, key);
	if(!hashnode){
		printf("%s: key doesn't exist, create new one\n", __FUNCTION__);
		p = (lru_entry_t *) zalloc(lru->alloc, sizeof(lru_entry_t));
		if(!p){
			printf("%s: error allocating memory\n", __FUNCTION__);
			return 0;
		}
		p->next = 0;
		p->prev = 0;
		p->value = 0;
		int resh = hash_map_insert(lru->hashmap, 
									key, p, 
									hash_value_clone_cb, //consider NULL?
									lru_free_nothing);   //consider NULL?	
		if(resh == 0){
			printf("%s: error inserting to hash map\n", __FUNCTION__);
			zfree(lru->alloc, p, sizeof(lru_entry_t));
			return 0;
		}
		p->pointer_back = hash_map_lookup_entry(lru->hashmap, key);	
		printf("%s: debug: return addr in hashmap: %p\n",
			__FUNCTION__, (void *)p->pointer_back);
	}else{
		existed = 1;
		p = (lru_entry_t *)hashnode->value;
	}
	void * new_value = value_clone(lru->alloc, value);
	if(!(new_value)){
		/* fails to clone value to p */
		printf("%s: fail to clone value\n", __FUNCTION__);
		if (!existed) zfree(lru->alloc,p, sizeof(lru_entry_t));
		return 0;
	}
	printf("%s: value cloned\n", __FUNCTION__);
	/* copy value */
	if(p->value){
		printf("%s: discard old value\n", __FUNCTION__);
		p->value_free(lru->alloc, p->value);
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

void * lru_cache_get(alloc_t * alloc,//could be NULL
				  lru_cache_t * lru,
				  const void * key){
	if(!lru || !key) {
		return 0;
	}
	void * v = 0;
	lru_entry_t * p = 0;
	v=hash_map_lookup_value(lru->hashmap, NULL, key);
	if(!v){
		printf("%s: key doesn't exist\n", __FUNCTION__);
		return 0;
	}
	p = (lru_entry_t *)(v);
	/* move pointer to the front of list */
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = lru->head->next;
	lru->head->next->prev = p;
	lru->head->next = p;
	p->prev = lru->head;
	if (!alloc) return p->value;
	/* copy value then exit */
	void * res = p->value_clone(alloc, p->value);
	if(!res){
		printf("%s: cannot clone value\n", __func__);
		return NULL;
	}
	return res;
}
