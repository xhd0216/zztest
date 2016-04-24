/*********************************
 *
 * define the free list structure
 *
 ********************************/

 begin
/
+---------------------------+
|______|_block|______|______|
|______|______|______|______|
|______|______|______|______|
|                           |
|                           |
+---------------------------+

struct free_list_s{
	struct free_list_s * prev;
	struct free_list_s * next;
	void * data;
};
typedef struct free_list_s free_list_t;

typedef struct free_list_allocator_s{
	int block_size; /* size of each block */
	int block_num;  /* number of blocks in the region */
	void * begin;   /* beginning of the whole block */
	free_list_t * head; /* head of list of available memory block */
	int num_avail;  /* number of available blocks */
}fl_allocator_t;

fl_allocator_t * fl_allocator_construct(int size, int num);
void fl_allocator_destruct(fl_allocator_t *);
void * fl_alloc(fl_allocator_t *);
void fl_free(void *);
