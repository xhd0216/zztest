#include "fl_alloc.h" 

typedef struct zalloc_s{
	fl_allocator_t ** mem_lists; /* an array of fl allocators */
	int n_lists; /* number of fl allocators, also, the array size of mem_lists */
	int flag; /* out of memory, etc */
	int mem_used; /* total memory used by malloc */
				  /* fixed list memory usage tracked by each list */
}alloc_t;

/* zalloc_s should contain a list of fl_allocator_t
 * each fl_allocator's size range does not overlap with each other.
 */

alloc_t * zalloc_construct(
				void * allocator,// allocator for memory, if NULL, use malloc
				fl_allocator_init_param_t **, 
				int num);
void zalloc_destruct(void * allocator, alloc_t *);


/* XXX: we do not support allocating array like:
 * p=zalloc( , 10, 64);
 * p = p+1;
 * ??? 
 */
//void * zalloc(zalloc_t *, int num, int size);

/* now, we only support allocating one memory */
void * zalloc(alloc_t *, int size);
void zfree(alloc_t *, void *, int);




