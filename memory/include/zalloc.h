#include "fl_alloc.h" 

typedef struct zalloc_s{
	fl_allocator_t ** mem_lists; /* an array of fl allocators */
	int n_lists; /* number of fl allocators, also, the array size of mem_lists */
	int flag; /* out of memory, etc */
}zalloc_t;

zalloc_t * zalloc_construct(fl_allocator_init_param_t **, int num);
void zalloc_destruct();


/* TODO: allocate an array of memories, but, how to implement 
 * p=zalloc( , 10, 64);
 * p = p+1;
 * ??? 
 */
//void * zalloc(zalloc_t *, int num, int size);

/* now, we only support allocating one memory */
void * zalloc(zalloc_t *, int size);
void zfree(void *);




