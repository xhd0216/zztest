#include <pthread.h>
#include "zalloc.h"
typedef void * (*queue_node_clone_cbf)(alloc_t *, const void *);
typedef void (*queue_node_destroy_cbf)(alloc_t *, void *);

typedef struct queue_node_s{
	struct queue_node_s * next;
	struct queue_node_s * prev;
	void * pVal;
	queue_node_clone_cbf clone; // could be NULL
	queue_node_destroy_cbf free; //could be NULL
}queue_node_t;


/*thread safe queue should have mutex*/
typedef struct queue_s{
	alloc_t * alloc;
	queue_node_t * head;
	queue_node_t * end;
	int count;
	int max_count;
	pthread_mutex_t q_lock;
	//queue_node_clone_cbf * clone_cb;
	//queue_node_destroy_cbf * destroy_cb;
}queue_t;
/*if pVal has same type, need to define callback to delete popped nodes*/
/*if pVal does not have same type, how to delete them?*/
//int queue_init(queue_t **, int max, queue_node_clone_cbf *, queue_node_destroy_cbf *);
//int queue_init(queue_t **, int max);
queue_t * queue_construct(alloc_t *, int max_count);

/* if cbf = NULL, put the src pointer in queue_node->pVal
 * otherwise, create new memory to clone src*/
/* recommend to use clone = 0*/
int queue_push(queue_t *, const void *, queue_node_clone_cbf *);
void * queue_pop(queue_t *, alloc_t *);
void * queue_peek(queue_t *, alloc_t *);
alloc_t * queue_destruct(queue_t *);
