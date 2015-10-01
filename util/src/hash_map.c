#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"
void hash_map_free_entry(hash_map_entry_t * p){
	if(p){
		if(p->key){
			/* should have call back function to free key */
			free(p->key);
		}
		if(p->value){
			/* should have call back function to free value */
			free(p->value);
		}
		free(p);
	}
}
int hash_map_init(hash_map_t ** hm,
				hash_map_function f){
	if(!hm) return 0;
	*hm = malloc(sizeof(hash_map_t));
	if(!*hm) return 0;
	int i = 0;
	//for(int i = 0; i < HASH_MAP_MAX_BUCKETS; i++){
	while(i < HASH_MAP_MAX_BUCKETS){
		(*hm)->table[i] = malloc(sizeof(hash_map_entry_t));
		if((*hm)->table[i] == 0){
			return 0;
		}
		(*hm)->table[i]->prev = 0;
		(*hm)->table[i]->next = 0;
		i++;
	}
	(*hm)->hash_f = f;
	return 1;
}
int hash_map_bucket_number_valid(int b){
	return (b >=0 && b < HASH_MAP_MAX_BUCKETS)?1:0;
}
int hash_map_key_compare(const void * key_1, int s1,
						 const void * key_2, int s2){
	if(!key_1 || !key_2 || s1 != s2) return 0;
	return (memcmp(key_1, key_2, s1) == 0)?1:0;
} 

hash_map_entry_t * 
hash_map_lookup_entry(hash_map_t * hm,
					const void * key,
					int size){
	if(!hm || !key || size < 1){
		return 0;
	}
	int buc = hm->hash_f(key, size);
	if(hash_map_bucket_number_valid(buc)){
		printf("%s: invalid bucket number %d\n", __FUNCTION__, buc);
		return 0;
	}
	hash_map_entry_t * p = hm->table[buc]->next;
	while(p){
		if(hash_map_key_compare(key, size, p->key, p->key_size)){
			break;
		}
		p=p->next;
	}
	return p;
}
int hash_map_lookup(hash_map_t * hm,
					const void * key,
					int key_size,
					void ** value,
					value_clone_cb_f clone){
	if(!hm || !key || key_size < 1 || !value){
		return 0;
	}
	hash_map_entry_t * p = hash_map_lookup_entry(hm, key, key_size);
	if(!p){
		printf("%s: cannot find entry\n", __FUNCTION__);
		return 0;
	}
	int res = clone(value, p->value);
	if(!res){
		printf("%s: fail to clone value\n", __FUNCTION__);
		*value = 0;
	}
	return res;
}
int hash_map_delete_entry(hash_map_t * hm,
						const void * key,
						int size){
	hash_map_entry_t * p = hash_map_lookup_entry(hm, key, size);
	if(!p){
		printf("%s: cannot find entry\n", __FUNCTION__);
		return -1;
	}
	p->prev->next = p->next;
	if(p->next) p->next->prev = p->prev;
	hash_map_free_entry(p);
	return 1;
}


/* insert a (key-value) pair;
 * if key already exists, update the value 
 * return 1 or 0
 */
int
hash_map_insert(hash_map_t * hashmap,
				const void * key,
				int key_size,
				const void * value,
				value_clone_cb_f clone){
	if(!hashmap || !key || key_size < 1 || !value){
		return 0;
	}
	int res = 0;
	hash_map_entry_t * new_node;
	/* if the key already exists, update it */
	new_node = hash_map_lookup_entry(hashmap, key, key_size);
	if(new_node){
		printf("%s: entry already exists, update it\n", __FUNCTION__);
		void * new_value;
		res = clone(&new_value, value);	
		if(!new_value || res < 1){
			printf("%s: error cloning memory for value, stop update\n", __FUNCTION__);
			return 0;
		}
		free(new_node->value);
		//memcpy(new_value, value, value_size);
		new_node->value = new_value;
		//new_node->value_size = res;
	}else{
		new_node = (hash_map_entry_t *) malloc(sizeof(hash_map_entry_t));
		if(!new_node){
			printf("%s: error allocating memory\n", __FUNCTION__);
			return 0;
		}
		//new_node->value = (void *)malloc(value_size);
		res = clone(&(new_node->value), value);
		if(!(new_node->value) || res < 1){
			printf("%s: error cloning memory for value\n", __FUNCTION__);
			if(new_node->value){
				/*should have better way to free value*/
				free(new_node->value);
			}
			free(new_node);
			return 0;
		}
		//new_node->value_size = res;	
		/*clone key*/
		memcpy(new_node->key, key, key_size);
		new_node->key_size = key_size;
		//memcpy(new_node->value, value, value_size);
	}
	int buc = hashmap->hash_f(key, key_size);
	if(!hash_map_bucket_number_valid(buc)){
		printf("%s: bucket number not valid: %d\n", __FUNCTION__, buc);
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
