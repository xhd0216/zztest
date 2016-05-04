#include "zalloc.h"

alloc_t * zalloc_construct(
			void * allocator,
			fl_allocator_init_param_t ** params,
			int num);
{
	if (num < 1) {
		return NULL;
	}
	if (!allocator){
		/* use malloc */
		alloc_t * ret = (alloc_t *)malloc(sizeof(alloc_t));
		if (!ret){
			return NULL;
		}
		ret->mem_lists = (fl_allocator_t **)malloc(sizeof(fl_allocator_t *) * num);
		memset(ret->mem_lists, 0, sizeof(fl_allocator_t *) * num);
		if (!ret->mem_lists){
			free(ret);
			return NULL;
		}
		ret->n_lists = num;
		ret->flag = 0;
		ret->mem_used = 0;
		int i = 0;
		while (i < num){
			/* make sure that fl_allocator uses malloc as extra memory allocator */
			params[i]->alloc = allocator;
			params[i]->extra_malloc = NULL;
			params[i]->extra_free = NULL;
			ret->mem_lists[i] = fl_allocator_construct(params[i]);
			if (!ret->mem_lists[i]){
				/* failed to construct fixed list, destruct the fl */
				zalloc_destruct(NULL, ret);
				return NULL;
			}
			i++;
		}
	} else {
		/* TODO: use allocator to get memory */
		return NULL;
	}	
}

void zalloc_destruct(void * allocator, alloc_t * za){
	if (!allocator) {
		/* allocator is malloc */	
		int j =0;
		while (j < za->n_lists) {
			if (za->mem_lists[j]){
				fl_allocator_destruct(za->mem_lists[j]);
			}
			j++;
		}
		free(za);
	} else {
		/* if allocator is not NULL */
	}
}

void * zalloc(alloc_t * za, int size){
	int i = 0;
	void * ret = NULL;
	while ( i < za->n_lists){
		if (size <= za->mem_lists[i]->block_size &&
			size >= za->mem_lists[i]->min_size) {
			ret = fl_alloc(za->mem_lists[i], size);
			if (ret) return ret;
			else break;
		}
		i++;
	}
	/* XXX: currently we use malloc */
	ret = (void *)malloc(size);
	if (ret) {
		za->mem_used += size;
	}
	return ret;
}
/* need to use size here */
void zfree(alloc_t *, void * p, int size){
	int i = 0;
	while(i < za->n_lists){
		/* need to check range, not only the size */
		int r = chunk_is_in_range(za->mem_lists[i], p);
		if (r == ALLOC_OK) {
			fl_free(za->mem_lists[i], p);
			return;
		} else if (r == ALLOC_ERR_ADDR_ALIGN_ERR) {
			return;
		}
		/* else, p in not in range */
		i++;
	}
	/* XXX: now assume it is malloc */
	free(p);
	za->mem_used -= size;
}