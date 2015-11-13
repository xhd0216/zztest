#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
int queue_init(queue_t ** qp,
			   int max_count){
	//		   queue_node_clone_cbf * ccbf,
	//		   queue_node_destroy_cbf * dcbf){
	if(!qp){
		printf("%s: queue pointer null\n", __FUNCTION__);
		return 0;
	}
	*qp = (queue_t *)malloc(sizeof(queue_t));
	if(!*qp){

		printf("%s: error allocating memory for queue\n", __FUNCTION__);
		return 0;
	}
	queue_t * tmp = *qp;
	if (pthread_mutex_init(&(tmp->q_lock), NULL) != 0)
    {
        printf("%s: mutex init failed\n", __FUNCTION__);
        return 0;
    }
	tmp->head = (queue_node_t *)malloc(sizeof(queue_node_t));
	tmp->end = (queue_node_t *)malloc(sizeof(queue_node_t));
	if(!(tmp->head) || !(tmp->end)){
		printf("%s: error allocating memory for queue->head\n", __FUNCTION__);
		if(tmp->head) free(tmp->head);
		if(tmp->end) free(tmp->end);
		free(tmp);
		return 0;
	}
	tmp->head->prev = 0;
	tmp->head->next = tmp->end;
	tmp->end->prev = tmp->head;
	tmp->end->next = 0;
	tmp->head->pVal = 0;
	tmp->end->pVal  = 0;
	tmp->count = 0;
	tmp->max_count = max_count;
	//*qp->clone_cb = ccbf;
	//*qp->destroy_cb = ddcbf;
	return 1;
}

/* if clone is NULL: use p as val of queue node*/
int queue_push(queue_t * q,
			   const void * p,
			   queue_node_clone_cbf * clone){
	queue_node_t * tmp = 0;
	if(!q){
		printf("%s: null queue pointer\n", __FUNCTION__);
		return 0;
	}
	if(!p){
		printf("%s: null push pointer\n", __FUNCTION__);
		return 0;
	}
	pthread_mutex_lock(&(q->q_lock));
	/*have to include all these in lock*/
	if (q->count >= q->max_count){
		printf("%s: queue is full\n", __FUNCTION__);
		goto error;
	} 
	tmp = (queue_node_t *) malloc(sizeof(queue_node_t));
	if(!tmp){
		printf("%s: insufficient memory to allocate new queue node\n", __FUNCTION__);
		goto error;
	}
	if (!clone){
		tmp->pVal = p;
	} else {
		int ret = (*clone)((void **)&(tmp->pVal), p);
		if (ret<1 || !(tmp->pVal)){
			printf("%s: failed to clone\n", __FUNCTION__);
			free(tmp);
			goto error;
		}
	}
	tmp->prev = q->end->prev;
	tmp->prev->next = tmp;
	q->end->prev = tmp;
	tmp->next = q->end;
	pthread_mutex_unlock(&(q->q_lock));
	return 1;
error:
	pthread_mutex_unlock(&(q->q_lock));
	return 0;
}
