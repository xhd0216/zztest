#include "fl_alloc.h"


/* will check if b is aligned */
int chunk_is_in_range(fl_allocator_t * alloc, void * b){
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
fl_allocator_t * fl_allocator_construct(fl_allocator_init_param_t * param)
{
	if (param->size <= (sizeof(free_list_t))) {
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
	/* Note: r will use malloc; not necessarily a user defined one */
	fl_allocator_t * r = (fl_allocator_t *)malloc(sizeof(fl_allocator_t));

	if (r == NULL) {
		printf("%s: cannot allocate memory for allocator\n", __func__);
		return r;
	}
	r->block_size = param->size;
	r->actual_size = param->size - sizeof(free_list_t);
	r->block_num = param->num;
	if (param->alloc == NULL){
		r->begin = (void *)malloc(param->size * param->num);
	} else {
		/* TODO: consider using user-defined allocator? */
		r->begin = (void *)malloc(param->size * param->num);
	}
	if (r->begin = NULL) {
		printf("%s: cannot allocate memory of size %d\n", __func__, param->size * param->num);
		free(r);
		return NULL;
	}
	r->end = r->begin + param->size * param->num;
	r->num_avail = param->num;
	/* build the linked list of available chunks */
	void * i = r->begin;
	void * j = r->begin + r->block_size;
	while(i < r->end){
		((free_list_t *) i)->next = j;
		i = j;
		j = j + sizeof(free_list_t);
	}
	(free_list_t *)(i - sizeof(free_list_t))->next = NULL;
	r->head.next = (free_list_t) r->begin;
	return r;
}

void * fl_alloc(fl_allocator_t * fl){
	/* TODO: add locks */
	if (fl->num_avail == 0 || fl->head.next == NULL){
		printf("%s: fixed list allocator (size %d) is empty\n", __func__, fl->actual_size);
		return NULL;
	}
	free_list_t * tmp = (free_list_t *) fl->head.next;
	fl->head.next = tmp->next;
	tmp->next = NULL;
	return (void *) tmp + sizeof(free_list_t);
}
