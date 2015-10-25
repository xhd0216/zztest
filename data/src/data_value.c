#include "data_value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int value_clone_cbf_for_data(void ** target, const void * src)
{
	if(!target || !src){
		printf("%s:%d error: input NULL\n", __FUNCTION__, __LINE__);
		return 0;
	}
	data_value_t * p = (data_value_t *)malloc(sizeof(data_value_t));
	if(!p){
		printf("%s: not enough space to clone value\n", __FUNCTION__);
		return 0;
	}
	printf("%s:%d src addr %p, cloned addr %p\n", 
			__FUNCTION__, __LINE__, src, (void *) p);
	const data_value_t * s = (const data_value_t *)src;
	int b_success = 1;
	p->head = 0;
	kv_pair_t * h = s->head;
	kv_pair_t * prev = 0;
	while(h){
		kv_pair_t * tmp = (kv_pair_t *)malloc(sizeof(kv_pair_t));
		if(!tmp){
			printf("%s: no momory\n", __FUNCTION__);
			b_success = 0;
			break;
		}
		memcpy(tmp, h, sizeof(kv_pair_t));
		if(!p->head){
			p->head = tmp;
		}
		else{
			prev->next = tmp;
		}
		prev = tmp;
		h = h->next;
	}
	if(!b_success){
		/*memory allocation failed, clean up*/
		printf("%s:%d: mem alloc failed\n", __FUNCTION__, __LINE__);
		value_free_cbf_for_data(p);
		return 0;
	}
	memcpy(p->parameters, s->parameters, sizeof(double)*rp_MAX_PARAM);
	memcpy(p->has_parameters, s->has_parameters, sizeof(int) * rp_MAX_PARAM);
	*target = (void *)p;
	return 1;
}

void value_free_cbf_for_data(void * value){
	if(!value) return;
	data_value_t * p = (data_value_t *)value;
	kv_pair_t * h = p->head;
	kv_pair_t * next;
	while(h){
		next = h->next;
		free(h);
		h= next;
	}
	free(p);
}

char * value_to_string_for_data(const void * value){
	const data_value_t * p = (const data_value_t *)value;
	char * str = (char *) malloc(sizeof(char) * 30);
	if(!str){
		return "get value failed";
	}
	int res = sprintf(str, "parameter: %lf", p->has_parameters[0]?p->parameters[0]:0.0);
	if(res < 1){
		return "get value error";
	}
	return str;
}
