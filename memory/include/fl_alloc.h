/*********************************
 *
 * define the free list structure
 *
 ********************************/

/*

 begin
/
+---------------------------+
|______|_block|______|______|
|______|______|______|______|
|______|______|______|______|
|______|______|______|______|
|______|______|______|______|_
                              \
       		                   end

*/


#define ALLOC_ERR_FREE_LIST_EMPTY   -1
#define ALLOC_ERR_ADDR_ALIGN_ERR    -2
#define ALLOC_ERR_ADDR_OUT_OF_RANGE -3
#define ALLOC_OK                     1


struct free_list_s{
	void * next;
};
typedef struct free_list_s free_list_t;


/* extra allocating function that when fixed list allocator is full,
   it is called to allocate memory, like malloc.
   parameter: 
		alloc: allocator
		size:  size to allocate
   return value:
		a pointer to memory;
		size of the memory stored in size
*/
typedef void * (*extra_allocator_f)(void * alloc, int * size);
typedef void (*extra_free_f)(void * alloc, void * v);

typedef struct free_list_allocator_s{
	int block_size; /* size of each block, MUST be a power of 2 */
	int block_num;  /* number of blocks in the region */
	void * begin;   /* beginning of the whole block */
	void * end;     /* end of the whole block */
	free_list_t head; /* head of list of available memory block */
	int num_avail;  /* number of available blocks */
	int min_size;   /* minimal size to allocate, 1 by default */
	int extra_size; /* total size that allocated by extra allocator */
	int extra_size_max; /* maximum size that allowed for extra allocate */
	void * alloc;   /* TODO: allocator to extra memory */
	/* note: we don't free alloc when we destruct fl_allocator_t */
	extra_allocator_f extra_malloc; /* function to allocate memory when fl is full */
	extra_free_f      extra_free;
	/* extra allocator could be NULL -- in that case, when fl allocator is full,
	   return NULL, then let the caller decide what to do. That is useful when an
	   allocator which consists of multiple fl allocators and one extra allocator */
}fl_allocator_t;

typedef struct fl_allocator_param_s{
	int size;
	int num;
	int min_size;
	void * alloc; /* extra allocator */
	extra_allocator_f eaf;
	extra_free_f eff;
	int extra_size_max;
}fl_allocator_init_param_t;


int chunk_is_in_range(fl_allocator_t * alloc, void * b);

fl_allocator_t * fl_allocator_construct(fl_allocator_init_param_t *);
void fl_allocator_destruct(fl_allocator_t *);
void * fl_alloc(fl_allocator_t *, int);
void fl_free(fl_allocator_t *, void *);
