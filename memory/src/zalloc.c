#include "zalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline int 
zalloc_validate_init_params(fl_allocator_init_param_t ** params,
							int num)
{
	int t = 1;
	int last_size = params[0]->size;
	while(t < num){
		if (params[t]->size < last_size) {
			printf("%s: fixed list size should be in increasing order\n", __func__);
			return 0;
		}
		if (params[t]->min_size <= last_size) {
			/* fixed list size ranges should not overlap */
			params[t]->min_size = last_size + 1;
		}
		last_size = params[t]->size;
		t++;
	}
	return 1;
}

alloc_t * zalloc_construct(
			void * allocator,
			fl_allocator_init_param_t ** params,
			int num)
{
	alloc_t * ret = NULL;
	
	/* validate the params */
	if (num < 1) {
		return NULL;
	}
	int rvi = zalloc_validate_init_params(params, num);
	if (!rvi) {
		printf("%s: construction failed: invalid parameters\n", __func__);
		return NULL;
	}
	if (!allocator){
		/* use malloc */
		ret = (alloc_t *)malloc(sizeof(alloc_t));
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
		ret->allocator = NULL;
		int i = 0;
		while (i < num){
			/* make sure that fl_allocator uses NO extra memory allocator */
			params[i]->alloc = NULL;
			params[i]->eaf = NULL;
			params[i]->eff = NULL;
			params[i]->extra_size_max = 0;
			ret->mem_lists[i] = fl_allocator_construct(params[i]);
			if (!ret->mem_lists[i]){
				printf("%s: failed to construct fixed list #%d\n", __func__, i);
				/* failed to construct fixed list, destruct the fl */
				zalloc_destruct(NULL, ret);
				return NULL;
			}
			i++;
		}
	} else {
		/* TODO: use allocator to get memory */
		//return NULL;
	}
	return ret;
}

void * zalloc_destruct(void * allocator, alloc_t * za){
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
		/* TODO:if we use extra allocator... */
	}
	return allocator;
}
void * zcalloc(alloc_t * za, int size)
{
	void * ret = zalloc(za, size);
	if (ret) {
		memset(ret, 0, size);
	}
	return ret;
}
void * zalloc(alloc_t * za, int size)
{	
	void * ret = NULL;
	if (!za) {
		ret = malloc(size);
		goto zalloc_return;
	}
	int i = 0;
	while ( i < za->n_lists){
		if (size <= za->mem_lists[i]->block_size &&
			size >= za->mem_lists[i]->min_size) {
			ret = fl_alloc(za->mem_lists[i], size);
			if (ret) 
				goto zalloc_return;
			else break;
		}
		i++;
	}
	if (!za->allocator) {
		ret = (void *)malloc(size);
		if (ret) {
			// record extra mem used.
			za->mem_used += size;
		}
	} else {
		//TODO
		printf("%s: we don't support extra allocator yet\n", __func__);
	}
zalloc_return:
	return ret;
}

static inline int
fl_alloc_size_match(fl_allocator_t * fl, int size)
{
	if (fl->block_size >= size && fl->min_size <= size) {
		return 1;
	}
	return 0;
}
/* need to use size here */
void zfree(alloc_t * za, void * p, int size)
{
	if(!za) return; 
	int i = 0;
	while(i < za->n_lists){
		/* need to check range, not only the size */
		int r = chunk_is_in_range(za->mem_lists[i], p);
		if (r == ALLOC_OK) {
			fl_free(za->mem_lists[i], p);
			return;
		} else if (r == ALLOC_ERR_ADDR_ALIGN_ERR) {
			return;
		} else if (!fl_alloc_size_match(za->mem_lists[i], size)) {
			/* size does not match */
			return;
		}
		/* else, p in not in range */
		i++;
	}
	/* XXX: now assume it is malloc */
	free(p);
	za->mem_used -= size;
}
