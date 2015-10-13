#include "data_value.h"

int value_clone_cbf_for_data(void ** target, const void * src)
{
	if(!target || !src){
		return 0;
	}
	data_value_t * p = (data_value_t *)malloc(sizeof(data_value_t));
	if(!p){
		printf("%s: not enough space to clone value\n", __FUNCTION__);
		return 0;
	}
	const data_value_t * s = (const data_value_t *)src;
	//TODO:copy two arrays and clone the linked list
}
