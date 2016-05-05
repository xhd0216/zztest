/*************************************
 *
 * copyright: xhd0216@gmail.com
 *
 ************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fl_alloc.h"



/* extra allocation function, input allocator could be NULL */
void * extra_alloc_func(void * allocator, int * size)
{
	void * ret = (void *)malloc(*size);
	return ret;
	/* error is handled outside this function */
}
void extra_free_func(void * allocator, void * v)
{
	free(v);
}

/* will check if b is aligned */
int chunk_is_in_range(fl_allocator_t * alloc, void * b){
	printf("%s: validating address, begin=%p, end=%p, addr=%p, size=%d\n",
			__func__, alloc->begin, alloc->end, b, alloc->block_size);
	if (b >= alloc->begin && b < alloc->end){
		if ((b - alloc->begin) & alloc->block_size){
			return ALLOC_ERR_ADDR_ALIGN_ERR;
		} else {
			return ALLOC_OK;
		}
	}
	return ALLOC_ERR_ADDR_OUT_OF_RANGE;
}

/* TODO: consider using user-defined allocator rather than malloc, need to indicate in param */
fl_allocator_t * 
fl_allocator_construct(fl_allocator_init_param_t * param)
{
	if (param->size < (sizeof(free_list_t))) {
		/* requested size is too small, not efficient */
		printf("%s: block size too small\n", __func__);
		return NULL;
	}
	/* validate the parameters */
	if ((param->size ^ (param->size-1)) != 0){
		/* block size is NOT a power of 2 */
		printf("%s: chunk size is not a power of 2\n", __func__);
		return NULL;	
	}
	if ((param->num ^ (param->num-1)) != 0) {
		/* block number is NOT a power of 2 */
		printf("%s: chunk number is not a power of 2\n", __func__);
		return NULL;
	}
	if (param->min_size >= param->size) {
		return NULL;
	}
	/* Note: r will use malloc; not necessarily a user defined one */
	fl_allocator_t * r = (fl_allocator_t *)malloc(sizeof(fl_allocator_t));

	if (r == NULL) {
		printf("%s: cannot allocate memory for allocator\n", __func__);
		return r;
	}
	//if (param->alloc == NULL){
	//	r->begin = (void *)malloc(param->size * param->num);
	//} else {
		/* TODO: consider using user-defined allocator? 
		   and, don't mess up with extra allocator	*/
		r->begin = (void *)malloc(param->size * param->num);
	//}
	if (r->begin == NULL) {
		printf("%s: cannot allocate memory of size %d\n", __func__, param->size * param->num);
		free(r);
		return NULL;
	}
	r->end = r->begin + param->size * param->num;
	r->num_avail = param->num;
	r->block_size = param->size;
	r->min_size = param->min_size > 1? param->min_size:1;
	r->block_num = param->num;
	r->extra_size = 0;
	r->extra_size_max = param->extra_size_max;
	r->alloc = param->alloc;
	r->extra_malloc = param->eaf;
	r->extra_free = param->eff;
	/* build the linked list of available chunks */
	void * i = r->begin;
	void * j = r->begin + r->block_size;
	while(j < r->end){
		((free_list_t *) i)->next = j;
		i = j;
		j = j + sizeof(free_list_t);
	}
	((free_list_t *)i)->next = NULL;
	r->head.next = (free_list_t *) r->begin;
	return r;
}
void fl_allocator_destrcut(fl_allocator_t * fl)
{
	if (!fl) return;
	if (fl->begin) free(fl->begin); /* TODO: what if we did not use malloc to allocate fl->begin ? */
	free(fl);
}


void * fl_alloc(fl_allocator_t * fl, int size){
	/* TODO: add locks */
	if (size > fl->block_size || size < fl->min_size) {
		printf("%s: requested size is %d, does not fit in a block\n", __func__, size);
		return NULL;
	}

	if (fl->num_avail == 0 || fl->head.next == NULL){
		printf("%s: fixed list allocator (size %d) is empty\n", __func__, fl->block_size);
		if(fl->extra_malloc && fl->extra_size + size <= fl->extra_size_max){
			int s = size;
			void * ret = fl->extra_malloc(fl->alloc, &s);
			if (ret != NULL) {
				fl->extra_size += s;
				return ret;
			}
		}
		return NULL;
	}
	void * tmp = fl->head.next;
	fl->head.next = ((free_list_t *)tmp)->next;
	//tmp->next = NULL;
	memset(tmp, 0, fl->block_size);
	fl->num_avail -= 1;

	return (void *) tmp;
}

void fl_free(fl_allocator_t * fl, void * v)
{
	if (!fl || !fl->begin) {
		return;
	}
	int ret = chunk_is_in_range(fl, v);
	if (ret == ALLOC_ERR_ADDR_OUT_OF_RANGE) {
		/* in extra allocator */
		if (fl->extra_free) {
			fl->extra_free(fl->alloc, v);
		}
		return;
	} else if (ret != ALLOC_OK) {
		/* error */
		return;
	}
	free_list_t * t = (free_list_t *) v;
	t->next = fl->head.next;
	t = fl->head.next = t;
}
