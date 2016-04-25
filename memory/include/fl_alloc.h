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
	struct free_list_s * prev;
	struct free_list_s * next;
	void * data;
};
typedef struct free_list_s free_list_t;

typedef struct free_list_allocator_s{
	int block_size; /* size of each block, MUST be a power of 2 */
	int block_num;  /* number of blocks in the region */
	void * begin;   /* beginning of the whole block */
	void * end;     /* end of the whole block */
	free_list_t * head; /* head of list of available memory block */
	int num_avail;  /* number of available blocks */
}fl_allocator_t;

typedef struct fl_allocator_param_s{
	int size;
	int num;
}fl_allocator_init_param_t;

fl_allocator_t * fl_allocator_construct(fl_allocator_init_param_t *);
void fl_allocator_destruct(fl_allocator_t *);
void * fl_alloc(fl_allocator_t *);
void fl_free(void *);
