#include "data.h"
#include <stdio.h>
#include <stdlib.h>

void read_key(data_key_t ** kp){
	if(!kp) return;
	if(!*kp){
		key_free_cbf_for_data(*kp);
	}
	*kp = (data_key_t *)malloc(sizeof(data_key_t));
	if(!*kp){
		printf("%s: failed to allocate memory\n", __FUNCTION__);
		return;
	}
	printf("input key name:");
	scanf("%s", (*kp)->key_name);
	(*kp)->update_period = UPDATE_DAILY;

}
void read_value(data_value_t ** vp){
	if(!vp) return;
	if(!*vp){
		printf("%s: free data value\n", __FUNCTION__);
		value_free_cbf_for_data(*vp);
	}
	*vp = (data_value_t *)malloc(sizeof(data_value_t));
	if(!*vp){
		printf("%s: failed to allocate memory\n", __FUNCTION__);
	}	
	printf("input data:");
	scanf("%lf", &((*vp)->parameters[0]));
	(*vp)->has_parameters[0] = 1;

}

int main(){
	printf("good\n");
	lru_cache_t * lru = 0;
	lru_cache_init_wrap(&lru);
	if(!lru){
		printf("lru init fail\n");
		return 0;
	}	
	else{
		printf("lru inited\n");
	}
	data_value_t * data = 0;
	data_key_t * key = 0;
	int i = 1;
	while(i){

		printf("0 quit\n1 insert data\n2 get data\n3 delete\n4 display all\nyour choice: ");
		scanf("%d", &i);
		switch(i){
		case 1:
			read_key(&key);
			printf("got keys\n");
			read_value(&data);
			lru_cache_insert_wrap(lru, key, data);
			break;
		case 2:
			read_key(&key);
			printf("got key\n");
			lru_cache_get_wrap(lru, key, &data);
			printf("got data\n");
			break;
		case 3:
			lru_delete_auto(lru);
			break;
		case 4:
			lru_dump_wrap(lru);
			break;
		default:
			i = 0;
			break;
		}
	}
	return 0;
}
