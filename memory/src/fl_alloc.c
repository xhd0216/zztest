#include "fl_alloc.h"

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
	

}
