#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"

void hash_map_dump(hash_map_t * hm,
				key_to_string_cb_f  key_print,
				value_to_string_cb_f  value_print){
	int i = 0;
	while(i < HASH_MAP_MAX_BUCKETS){
		printf("[%d]===========\n", i);
		hash_map_entry_t * p = hm->table[i];
		if(!p || !p->next){
			++i;
			continue;
		}
		p = p->next;
		while(p){
			printf("key: %s\n", (key_print)(hm->table[i]->key));
			printf("value: %s\n", (value_print)(hm->table[i]->value));
			p = p->next;
		}
		++i;
	}
}



void hash_map_free_entry(hash_map_entry_t * p, key_free_cb_f  key_f){
	if(p){
		if(p->key){
			(key_f)(p->key);
		}
		if(p->value){
			p->value_free(p->value);
		}
		free(p);
	}
}

int hash_map_init(hash_map_t ** hm,
				hash_map_function  f,
				key_cmp_cb_f  key_c,
				key_free_cb_f  key_f,
				key_clone_cb_f  key_cl){
	if(!hm) return 0;
	*hm = malloc(sizeof(hash_map_t));
	if(!*hm) return 0;
	int i = 0;
	while(i < HASH_MAP_MAX_BUCKETS){
		(*hm)->table[i] = malloc(sizeof(hash_map_entry_t));
		if((*hm)->table[i] == 0){
			return 0;
		}
		memset((*hm)->table[i], 0, sizeof(hash_map_entry_t));
		(*hm)->table[i]->prev = 0;
		(*hm)->table[i]->next = 0;
		i++;
	}
	(*hm)->hash_f = f;
	(*hm)->key_cmp = key_c;
	(*hm)->key_free = key_f;
	(*hm)->key_clone = key_cl;
	return 1;
}
int hash_map_bucket_number_valid(int b){
	return (b >=0 && b < HASH_MAP_MAX_BUCKETS)?1:0;
}

hash_map_entry_t * 
hash_map_lookup_entry(hash_map_t * hm,
					const void * key){
	if(!hm || !key){
		return 0;
	}
	int buc = (*(hm->hash_f))(key);
	if(!hash_map_bucket_number_valid(buc)){
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
			break;
		}
		p=p->next;
	}
	return p;
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
