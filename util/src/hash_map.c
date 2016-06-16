#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"

void hash_map_dump(hash_map_t * hm,
				key_to_string_cb_f  key_print,
				value_to_string_cb_f  value_print)
{
	int i = 0;
	while(i < hm->size){
		printf("[bucket %d]===========\n", i);
		hash_map_entry_t * p = hm->table[i];
		if (p && p->next){
			p = p->next;
			while(p){
				printf("\tkey: %s\n", (*key_print)(p->key));
				printf("\tvalue: %s\n", (*value_print)(p->value));
				p = p->next;
			}
		} else {
			printf("bucket is empty\n");
		}
		++i;
	}
	printf("====%d entries in total====\n", hm->entries);
}

void
hash_map_free_entry(hash_map_t * hm, hash_map_entry_t * p) {
	if (!hm || !p || !hm->alloc){
		return;
	}
	if (p->key && hm->key_free) {
		hm->key_free(hm->alloc, p->key);
	}
	if (p->value && p->value_free) {
		p->value_free(hm->alloc, p->value);
	}
	zfree(hm->alloc, p, sizeof(hash_map_entry_t));
}


void hash_map_destruct(hash_map_t * hm)
{
	if (!hm || !hm->alloc) {
		return;
	}
	if (hm->table){
		int i = 0;
		hash_map_entry_t * p;
		hash_map_entry_t * tmp;
		while (i < hm->size){
			p = hm->table[i];
			if (!p) break;
			if (p && p->next){
				p = p->next;
				while(p){
					tmp = p->next;
					hash_map_free_entry(hm, p);
					p = tmp;
				}
			}
			i++;
		}
		zfree(hm->alloc, hm->table, sizeof(hash_map_entry_t *) * hm->size);
	}
	if (hm) 
		zfree(hm->alloc, hm, sizeof(hash_map_t)); 
	//XXX: because hm->alloc == lru->alloc, hm->alloc will be destructed in lru_destruct
}

hash_map_t * hash_map_construct(alloc_t * alloc,
				int sz,
				hash_map_function  f,
				key_cmp_cb_f  key_c,
				data_free_cb_f  key_f,
				data_clone_cb_f  key_cl)
{
	if (!alloc || sz < 1 || !f || !key_c || !key_f || !key_cl){
		/* invalid parameter */
		printf("%s: invalid parameter, return NULL\n", __func__);
		return NULL;
	}
	/* XXX: hm is allocated by zalloc, so when destruct, destruct hm only, 
	 * do NOT destruct hm->alloc */
	hash_map_t * hm = (hash_map_t *)zalloc(alloc, sizeof(hash_map_t));
	//printf("%s line %d: hm=%p\n", __func__, __LINE__, hm);
	if(!hm) {
		printf("error %s: zalloc error\n", __func__);
		return 0;
	}
	int i = 0;
	hm->size = sz;
	hm->alloc = alloc;
	//printf("%s line %d: before alloc: hm->table=%p\n", __func__, __LINE__, hm->table);
	hm->table = (hash_map_entry_t **)zalloc(hm->alloc,
											sizeof(hash_map_entry_t *) * sz);
	//printf("%s line %d: hm->table=%p\n", __func__, __LINE__, hm->table);
	if (!hm->table) {
		printf("error %s: zalloc failed\n", __func__);
		hash_map_destruct(hm);
		return NULL;
	} else {
		printf("%s: hashmap table = %p\n", __func__, hm->table);
	}
	while(i < sz){
		//printf("loop #%d\n", i);
		hm->table[i] = (hash_map_entry_t *)zalloc(hm->alloc, 
												  sizeof(hash_map_entry_t));
		if(0 == hm->table[i]){
			printf("%s: cannot allocate memory for buckets\n", __func__);
			hash_map_destruct(hm);
			return NULL;
		}
		//printf("%s: hm->table[%d]=%p\n", __func__, i, hm->table[i]);
		//memset((void *)hm->table[i], 0, sizeof(hash_map_entry_t));
		hm->table[i]->key = NULL;
		hm->table[i]->value = NULL;
		hm->table[i]->value_free = NULL;
		hm->table[i]->value_clone = NULL;
		hm->table[i]->prev = NULL;
		hm->table[i]->next = NULL;
		i++;
		//printf("%s: do you see this?\n", __func__);
		fflush(stdout);
	}
	//printf("%s: hashmap table initialized\n", __func__);
	hm->hash_f = f;
	hm->key_cmp = key_c;
	hm->key_free = key_f;
	hm->key_clone = key_cl;
	printf("%s: hash_map_t constructed\n", __func__);
	fflush(stdout);
	return hm;
}

hash_map_entry_t * 
hash_map_lookup_entry(hash_map_t * hm,
					const void * key){
	if(!hm || !key){
		return 0;
	}
	int buc = hm->hash_f(key) % hm->size;
	if(buc < 0 || buc >= hm->size){
		printf("%s: invalid bucket number %d\n", __func__, buc);
		return 0;
	}
	hash_map_entry_t * p = hm->table[buc];
	if(!p){
		printf("%s: hashmap->table[%d] is NULL\n", __func__, buc);
		return 0;
	}
	p = p->next;
	while(p){
		if((*(hm->key_cmp))(key, p->key)){
			return p;
		}
		p=p->next;
	}
	return NULL;
}
void * hash_map_lookup_value(hash_map_t * hm,
					alloc_t * alloc,
					const void * key)
{
	if(!hm || !key){
		return 0;
	}
	hash_map_entry_t * p = hash_map_lookup_entry(hm, key);
	if(!p || !p->value_clone){
		printf("%s: cannot find entry\n", __func__);
		return NULL;
	}
	if (!alloc) return p->value;
	void * res = (*(p->value_clone))(alloc, p->value);
	if(!res){
		printf("%s: fail to clone value\n", __func__);
	}
	return res;
}
int hash_map_remove_key(hash_map_t * hm,
						const void * key)
{
	hash_map_entry_t * p = hash_map_lookup_entry(hm, key);
	if(!p){
		printf("%s: cannot find entry\n", __FUNCTION__);
		return 0;
	}
	p->prev->next = p->next;
	if(p->next) p->next->prev = p->prev;
	/* free hash_map_entry_t*/
	hash_map_free_entry(hm, p);
	return 1;
}


/* insert a (key-value) pair;
 * if key already exists, update the value 
 * return 1 or 0
 */
int
hash_map_insert(hash_map_t * hashmap,
				const void * key,
				const void * value,
				data_clone_cb_f clone,
				data_free_cb_f value_f)
{
	if(!hashmap || !key || !value || !clone || !(hashmap->key_cmp) || !value_f){
		return 0;
	}
	int replace = 0;
	hash_map_entry_t * new_node = hash_map_lookup_entry(hashmap, key);
	if(new_node){
		printf("%s: entry already exists, update it\n", __FUNCTION__);
		replace = 1;
		/* if the key already exists, discard old value */
		if (new_node->value_free && new_node->value) {
			(*(new_node->value_free))(hashmap->alloc, new_node->value);
		}
		/* we keep the old key here. Assuming transitivity of key_cmp_cb_f*/
	} else {
		printf("%s: inserting new entry\n", __FUNCTION__);
		new_node = (hash_map_entry_t *)zalloc(hashmap->alloc, sizeof(hash_map_entry_t));
		if(!new_node){
			printf("%s: error allocating memory\n", __FUNCTION__);
			return 0;
		}
		/*clone key*/
		new_node->key = (*(hashmap->key_clone))(hashmap->alloc, key);
		if (!new_node->key) {
			printf("%s: error cloning key\n", __func__);
			zfree(hashmap->alloc, new_node, sizeof(hash_map_entry_t));
			return 0;
		}
	}
	new_node->value = (*clone)(hashmap->alloc, value);
	if(!(new_node->value)){
		printf("%s: error cloning memory for value\n", __FUNCTION__);
		hash_map_free_entry(hashmap, new_node);
		return 0;
	}
	new_node->value_clone = clone;
	new_node->value_free = value_f;
	if(replace) return 1;
	int buc = (*(hashmap->hash_f))(key) % hashmap->size;
	if (buc >= hashmap->size || buc < 0){
		printf("%s: bucket number not valid: %d\n", __FUNCTION__, buc);
		hash_map_free_entry(hashmap, new_node);
		return 0;
	}
	hash_map_entry_t * p = hashmap->table[buc];
	if(p->next){
		p->next->prev = new_node;
	}
	new_node->next = p->next;
	p->next = new_node;
	new_node->prev = p;
	return 1;
}
