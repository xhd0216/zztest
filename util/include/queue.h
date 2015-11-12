/*thread safe queue should have mutex*/
typedef struct queue_node_s{
	struct queue_node_s * next;
	struct queue_node_s * prev;
	void * pVal;
}queue_node_t;

typedef struct queue_s{
	queue_node_t * head;
	queue_node_t * end;
	int count;
	const int max_count;
}queue_t;
/*if pVal has same type, need to define callback to delete popped nodes*/
/*if pVal does not have same type, how to delete them?*/
int queue_init(queue_t **);
int queue_push(queue_t *, const void *);
int queue_pop(queue_t *, void **);
