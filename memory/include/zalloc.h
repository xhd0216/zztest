#ifndef FIXED_LIST_ALLOCATOR_H
#define FIXED_LIST_ALLOCATOR_H
#include "fl_alloc.h" 

typedef struct zalloc_s{
	fl_allocator_t** mem_lists; /* an array of fl allocators */
	int 			 n_lists;   /* number of fl allocators, also, the array size of mem_lists */
	int 			 flag;      /* out of memory, etc */
	int 			 mem_used;  /* total extra memory used by malloc */
	void *           allocator; //external allocator, used when no fl mem to use
}alloc_t;


/*-----------------------
 * function definitions
 *-----------------------*/
alloc_t * zalloc_construct(
				void * allocator,
				fl_allocator_init_param_t **, 
				int num);
void * zalloc_destruct(void * allocator, alloc_t *);
void * zalloc(alloc_t *, int size);
void zfree(alloc_t *, void *, int);
void * zcalloc(alloc_t *, int size);
#endif /* FIXED_LIST_ALLOCATOR_H */



