#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"

void hash_map_dump(hash_map_t * hm,
				key_to_string_cb_f  key_print,
				value_to_string_cb_f  value_print){
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


#if 0
void hash_map_free_entry(hash_map_entry_t * p, data_free_cb_f  key_f){
	if(p){
		if(p->key){
			(key_f)(p->key);
		}
		if(p->value && p->value_free){
			p->value_free(p->value);
		}
		free(p);
	}
}

void hash_map_fini(hash_map_t * hm){
	if(!hm) return;
	int i = 0;
	hash_map_entry_t * p;
	hash_map_entry_t * next;
	while(i < HASH_MAP_MAX_BUCKETS){
		p = hm->table[i];
		while(p){
			next = p->next;
			hash_map_free_entry(p, hm->key_free);
			p = next;
		}
		i++;
	}
	free(hm);

}
#endif

void hash_map_destruct(hash_map_t * hm)
{
	if (hm->table){
		int i = 0;
		hash_map_entry_t * p;
		hash_map_entry_t * tmp;
		while (i < hm->size){
			p = hm->table[i];
			if (p && p->next){
				p = p->next;
				while(p){
					tmp = p->next;
					/* free the key */
					if (hm->key_free) hm->key_free(hm->alloc, p->key);
					/* free the value */
					if (p->value_free) p->value_free(hm->alloc, p->value);
					/* free the entry */
					z_free(p);
					p = tmp;
				}
			}
			i++;
		}
	}
	/* hm is allocated by malloc */
	/* we do NOT free alloc here */
	free(hm);
}

hash_map_t * hash_map_construct(alloc_t * alloc,
				int sz,
				hash_map_function  f,
				key_cmp_cb_f  key_c,
				data_free_cb_f  key_f,
				data_clone_cb_f  key_cl)
{
	if (!alloc || sz < 1 || !f || !key_c || !key_f || !key_clone){
		/* invalid parameter */
		return NULL;
	}
	hash_map_t * hm = (hash_map_t *)malloc(sizeof(hash_map_t));
	if(!hm) return 0;
	int i = 0;
	hm->size = sz;
	hm->alloc = alloc;
	hm->table = (hash_map_entry_t **)z_alloc(hm->alloc,
														sizeof(hash_map_entry_t *) * sz);
	if (!hm->table) {
		free(hm);
		return NULL;
	}
	while(i < sz){
		//hm->table[i] = (hash_map_entry_t *)malloc(sizeof(hash_map_entry_t));
		hm->table[i] = (hash_map_entry_t *)z_alloc(hm->alloc, 
														sizeof(hash_map_entry_t));
		if(hm->table[i] == 0){
			return 0;
		}
		memset(hm->table[i], 0, sizeof(hash_map_entry_t));
		hm->table[i]->prev = 0;
		hm->table[i]->next = 0;
		i++;
	}
	(*hm)->hash_f = f;
	(*hm)->key_cmp = key_c;
	(*hm)->key_free = key_f;
	(*hm)->key_clone = key_cl;
	return hm;
}

hash_map_entry_t * 
hash_map_lookup_entry(hash_map_t * hm,
					const void * key){
	if(!hm || !key){
		return 0;
	}
	int buc = hm->hash_f(key);
	if(buc < 0 || buc >= hm->size){
		printf("%s: invalid bucket number %d\n", __FUNCTION__, buc);
		return 0;
	}
	hash_map_entry_t * p = hm->table[buc];
	if(!p){
		printf("%s: hashmap->table[%d] is NULL\n", __FUNCTION__, buc);
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
int hash_map_lookup(hash_map_t * hm,
					const void * key,
					void ** value){
	if(!hm || !key || !value){
		return 0;
	}
	hash_map_entry_t * p = hash_map_lookup_entry(hm, key);
	if(!p){
		printf("%s: cannot find entry\n", __FUNCTION__);
		return 0;
	}
	int res = (*(p->value_clone))(value, p->value);
	if(!res){
		printf("%s: fail to clone value\n", __FUNCTION__);
	}
	return res;
}
int hash_map_delete_entry(hash_map_t * hm,
						const void * key){
	hash_map_entry_t * p = hash_map_lookup_entry(hm, key);
	if(!p){
		printf("%s: cannot find entry\n", __FUNCTION__);
		return 0;
	}
	p->prev->next = p->next;
	if(p->next) p->next->prev = p->prev;
	/* free hash_map_entry_t*/
	hash_map_free_entry(p, hm->key_free);
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
				value_clone_cb_f clone,
				value_free_cb_f value_f){
	if(!hashmap || !key || !value || !clone || !(hashmap->key_cmp) || !value_f){
		return 0;
	}
	void * new_value = 0;
	int res = 0;
	int is_new = 0;
	hash_map_entry_t * new_node;
	res = (*clone)(&(new_value), value);
	if(!(new_value) || !res){
		printf("%s: error cloning memory for value\n", __FUNCTION__);
		return 0;
	}
	new_node = hash_map_lookup_entry(hashmap, key);
	if(new_node){
		printf("%s: entry already exists, update it\n", __FUNCTION__);
		is_new = 1;
		/* if the key already exists, discard old value */
		(*(new_node->value_free))(new_node->value);
		/* we keep the old key here. Assuming transitivity of key_cmp_cb_f*/
	}else{
		printf("%s: inserting new entry\n", __FUNCTION__);
		new_node = (hash_map_entry_t *) malloc(sizeof(hash_map_entry_t));
		if(!new_node){
			printf("%s: error allocating memory\n", __FUNCTION__);
			return 0;
		}
		/*clone key*/
		(*(hashmap->key_clone))(&(new_node->key), key);
	}
	new_node->value = new_value;
	new_node->value_clone = clone;
	new_node->value_free = value_f;
	if(is_new) return 1;
	int buc = (*(hashmap->hash_f))(key);
	if(!hash_map_bucket_number_valid(buc)){
		printf("%s: bucket number not valid: %d\n", __FUNCTION__, buc);
		hash_map_free_entry(new_node, hashmap->key_free);
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
