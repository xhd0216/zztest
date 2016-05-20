#include "data_value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void * value_clone_cbf_for_data(alloc_t * alloc, const void * src)
{
	if(!alloc || !src){
		printf("%s:%d error: input NULL\n", __FUNCTION__, __LINE__);
		return 0;
	}
	data_value_t * p = (data_value_t *)zalloc(alloc, sizeof(data_value_t));
	if(!p){
		printf("%s: not enough space to clone value\n", __FUNCTION__);
		return NULL;
	}
	memcpy(p, src, sizeof(data_value_t));
	return (void *)p;
}

void value_free_cbf_for_data(alloc_t * alloc, void * value){
	if(!value) return;
	zfree(alloc, value, sizeof(data_value_t));
}

/* TODO:need to rewrite for display format */
/* consider not to use this function, pass a extern callback to display value */
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
