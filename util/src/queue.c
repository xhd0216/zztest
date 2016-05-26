#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
queue_t * queue_construct(alloc_t * alloc,
						int max_count){	
	if (!alloc || max_count < 1){
		return NULL;
	}
	queue_t * tmp = (queue_t *)zalloc(alloc, sizeof(queue_t));
	if(!tmp){
		printf("%s: error allocating memory for queue\n", __FUNCTION__);
		return NULL;
	}
	tmp->head = NULL;
	tmp->end = NULL;
	tmp->max_count = max_count;	
	tmp->alloc = alloc;
	/*if (pthread_attr_init(&(tmp->q_lock), NULL) != 0)
    {
        printf("%s: mutex init failed\n", __FUNCTION__);
		queue_destruct(tmp);
        return NULL;
    }*/
	tmp->head = (queue_node_t *)zalloc(tmp->alloc, sizeof(queue_node_t));
	tmp->end = (queue_node_t *)zalloc(tmp->alloc, sizeof(queue_node_t));
	if(!(tmp->head) || !(tmp->end)){
		printf("%s: error allocating memory for queue->head\n", __FUNCTION__);
		queue_destruct(tmp);
		return NULL;
	}
	tmp->head->prev = 0;
	tmp->head->next = tmp->end;
	tmp->end->prev = tmp->head;
	tmp->end->next = 0;
	tmp->head->pVal = 0;
	tmp->end->pVal  = 0;
	tmp->count = 0;
	return tmp;
}
alloc_t * queue_destruct(queue_t * q)
{
	if (!q) return NULL;
	if (q->head && q->end) {
		queue_node_t * n = q->head;
		queue_node_t * tmp;
		while (n) {
			tmp = n->next;
			zfree(q->alloc, n, sizeof(queue_node_t));
			n = tmp;
		}
		/* head and end should be deleted now */
	}
	if (q->head) {
		zfree(q->alloc, q->head, sizeof(queue_node_t));
	}
	if (q->end) {
		zfree(q->alloc, q->end, sizeof(queue_node_t));
	}
	alloc_t * ret = q->alloc;
	zfree(ret, q, sizeof(queue_t));
	return ret;
}
/* if clone is NULL: use p as val of queue node*/
int queue_push(queue_t * q,
			   const void * p,
			   queue_node_clone_cbf clone){
	queue_node_t * tmp = 0;
	if(!q){
		printf("%s: null queue pointer\n", __FUNCTION__);
		return 0;
	}
	if(!p){
		printf("%s: null push pointer\n", __FUNCTION__);
		return 0;
	}
	/* if callback function clone is provided as a param 
	 * it is ok to initialize tmp without lock
     * otherwise, if clone is a param of queue
	 * need to lock it because q->clone_cbf may be changed by others
	 */
	tmp = (queue_node_t *)zalloc(q->alloc, sizeof(queue_node_t));
	if(!tmp){
		printf("%s: insufficient memory to allocate new queue node\n", __FUNCTION__);
		pthread_mutex_unlock(&(q->q_lock));
		return 0;
	}
	pthread_mutex_lock(&(q->q_lock));
	/*have to lock all these*/
	if (q->count >= q->max_count){
		printf("%s: queue is full\n", __FUNCTION__);
		pthread_mutex_unlock(&(q->q_lock));
		/* should free tmp here?*/
		zfree(q->alloc, tmp, sizeof(queue_node_t));
		return 0;
	} 
	if (!clone){
		tmp->pVal = (void *)p;
	} else {
		tmp->pVal = (clone)(q->alloc, p);
		if (!tmp->pVal){
			printf("%s: failed to clone\n", __FUNCTION__);
			zfree(q->alloc, tmp, sizeof(queue_node_t));
			pthread_mutex_unlock(&(q->q_lock));
			return 0;
		}
	}
	tmp->prev = q->end->prev;
	q->end->prev->next = tmp;
	q->end->prev = tmp;
	tmp->next = q->end;
	q->count++;
	pthread_mutex_unlock(&(q->q_lock));
	return 1;
}

static inline int queue_is_empty(queue_t * q){
	if (!q){
		return 1;
	}
	if (!q->head || !q->end || !q->head->next ||
		q->head->next == q->end || q->end->prev == q->head ||
		q->count < 1) {
		return 1;
	}
	return 0;
}
queue_node_t * queue_get_first_entry(queue_t * q){
	if(!q){
		printf("%s: q is null\n", __FUNCTION__);
		return NULL;
	}
	queue_node_t * tmp = 0;
	pthread_mutex_lock(&(q->q_lock));
	if (queue_is_empty(q)) {
		printf("%s: empty queue\n", __FUNCTION__);
		pthread_mutex_unlock(&(q->q_lock));
		return NULL;
	}
	pthread_mutex_unlock(&(q->q_lock));
	return tmp;
}
void queue_free_entry(alloc_t * alloc, queue_node_t * tmp){
	if (tmp->free && alloc) {
		(tmp->free)(alloc, tmp->pVal);
	}
	zfree(alloc, tmp, sizeof(queue_node_t));
}
void * queue_pop(queue_t * q,
				alloc_t * alloc){
	queue_node_t * tmp = queue_get_first_entry(q);
	if (!tmp) return NULL;
	/* remove tmp from queue */
	pthread_mutex_lock(&q->q_lock);
	tmp->prev->next = tmp->next;
	tmp->next->prev = tmp->prev;
	q->count -= 1;
	pthread_mutex_unlock(&q->q_lock);
	void * ret = NULL;
	if (!alloc	|| !tmp->clone) {
		ret = tmp->pVal;
	} else {
		ret = (tmp->clone)(alloc, tmp->pVal);
	}
	queue_free_entry(q->alloc, tmp);
	return ret;
}
/*
 *	if alloc is NULL, return the value pointer of first element;
 *	otherwise, clone the value, free original one, then return the clone
 */
void * queue_peek(queue_t * q,
				alloc_t * alloc){	
	queue_node_t * tmp = queue_get_first_entry(q);
	if (!tmp) return NULL;
	void * ret = NULL;
	if (!alloc	|| !tmp->clone) {
		ret = tmp->pVal;
	} else {
		ret = (tmp->clone)(alloc, tmp->pVal);
	}
	return ret;
}
