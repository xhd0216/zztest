#include "zalloc.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include "queue.h"
#include "srvr_memory.h"

#define SERVER_PATH_NAME "./server_file/_server_file_"
#define MSG_LENGTH 1024
#define MX_LISTEN 10
#define NUM_OF_WORKING_THREADS 4

#define MAX_QUEUE 64
#define MAX_LRU_ENTRY 1024

/*shared memory*/
alloc_t * alloc = NULL;
lru_cache_t * lru = NULL;
int b_end = 0;/*signal that ends the process*/
queue_t * q = NULL;

pthread_cond_t cond;
pthread_mutex_t mutex;



#define SIGNAL_T SIGINT
void sig_handler(int sig){
	printf("got signal %d\n", sig);
	pthread_mutex_lock(&mutex);
	b_end = 1;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
	unlink(SERVER_PATH_NAME);
	(void)signal(sig, SIG_DFL);
}
int sock_queue_is_empty(){
	return !q && q->count > 0?0:1;
}
//typedef struct sock_arg_s{
//	int sock;
//}sock_arg_t;

int send_all(int socket, const void *buffer, size_t length)
{
    const void *ptr = buffer;//(const char*) buffer;
    while (length > 0)
    {
        int i = write(socket, ptr, length);
		//printf("send() returns %d\n", i);
        if (i < 1) {
			perror("error sending msg");
			printf("error sending msg: %s\n", strerror(errno));
			return 0;
		}
        ptr += i;
        length -= i;
    }
    return 1;
}

void * worker(void * arg){
	int index = *(int *)arg;
	//sock_arg_t * s = 0;
	int * msgsock;
	int rval;
	char buf[MSG_LENGTH];
	char resp_buf[128];
	if(!q){
		printf("%s exit: queue not initialized\n", __FUNCTION__);
		goto thread_exit;
	}

	sprintf(resp_buf, "thread %d got some msg\n", index);
	while(!b_end){
		/* check for end signal*/
		/* wait for connection*/
		pthread_mutex_lock(&mutex);
		while(!b_end &&  sock_queue_is_empty()){
			pthread_cond_wait(&cond,&mutex);
		}
		if(b_end){
			printf("thread %d got end signal, quit\n", index);
			pthread_mutex_unlock(&mutex);
			goto thread_exit;
		}
		msgsock = (int *)queue_pop(q, NULL); 
		pthread_mutex_unlock(&mutex);
		rval = 1;
		
		bzero(buf, sizeof(buf));
		//pthread_mutex_lock(&mutex);
		rval = read(*msgsock, buf, MSG_LENGTH);
		//pthread_mutex_unlock(&mutex);
		if (rval < 0) {
			printf("thread %d: msg read error, errno=%d, msgsock=%d\n", index, errno, *msgsock);
		} else if(rval == 0) {
			printf("thread %d: read the end of file, msgsock=%d\n", index, *msgsock);
		} else {
			printf("thread %d: msg read --->  %s\n", index, buf);
			if (strcmp(buf, "END") == 0){
				printf("thread %d got end signal, msgsock=%d\n", index, *msgsock);
				b_end = 1;
				break;
			}
			//	pthread_mutex_lock(&mutex);
			int ret = send(*msgsock, (const void *) resp_buf, 
								strlen(resp_buf), 0);
			//	pthread_mutex_unlock(&mutex);
			if(ret){
				printf("thread %d: msg send\n", index);
			} else {
				printf("thread %d: failed to send msg\n", index);
			}
		}
		close(*msgsock);
		zfree(alloc, msgsock, sizeof(int));
		msgsock = NULL;
	}
thread_exit:
	printf("thread %d exits\n", index);
	pthread_exit(NULL);
}
void * queue_node_clone_int(alloc_t * alloc, const void * k)
{
	int * m;
	if (!alloc) {
		m = (int *)zalloc(alloc, sizeof(int));
	} else {
		m = (int *)malloc(sizeof(int));
	}
	*m = *(const int *)k;
	return (void *)m;
}

int main(int argc, char * argv[]){
	struct sigaction act;
	int b_end = 0;
	int sock;
	struct sockaddr_un server;
	pthread_t threads[NUM_OF_WORKING_THREADS];
	pthread_attr_t attr;


	
	if(argc != 2){
		printf("invalid num. of arguments\n");
		return 0;
	}
	if (strcmp(argv[1], "start") == 0){
		/* re-define ctrl-c action*/
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGNAL_T);
		act.sa_handler = sig_handler;
		sigaction(SIGNAL_T, &act, 0);
		printf("initializing...\n");
		/*initialize share memory*/
		alloc = server_allocate_memory(NULL);
		if (!alloc) {
			printf("%s: failed to initialize memory allocator, quit program\n",
					__func__);
			goto done_main;
		}
		lru = data_lru_cache_construct_wrap(alloc, MAX_LRU_ENTRY);
	
		if (!lru) { 
			printf("%s: failed to initialize cache, quit program\n",
					 __FUNCTION__);
			goto done_main;
		}
		/* queue for socket connection */
		q = queue_construct(alloc, MAX_QUEUE);
		if (!q) {
			printf("%s: failed to initialize queue, quit program\n", 
					__FUNCTION__);
			goto done_main;
		}
		printf("%s: shared memory initialized\n", __func__);
		/*initialize mutex and cond*/
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		
		int i = 0;
		while(i < NUM_OF_WORKING_THREADS){
			int * j = (int *)malloc(sizeof(int));
			*j = i+1;
			pthread_create(&threads[i], &attr, worker, (void *)j);
			i++;
		}
		/*create socket*/
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sock < 0){
			printf("%s: opening stream socket error: %d\n", 
					__FUNCTION__, sock);
			goto done_main;
		}
		printf("the connected socket is %d\n", sock);
		server.sun_family = AF_UNIX;
		strcpy(server.sun_path, SERVER_PATH_NAME);
		if(bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_un))){
			printf("%s: error in binding stream socket\n", __FUNCTION__);
			goto done_main;
		}
		chmod(SERVER_PATH_NAME, S_IRWXU|S_IRWXG|S_IROTH|S_IWOTH);
		
		printf("program is running, socket name: %s\n", server.sun_path);
		/* listen, max 10 connections waiting*/
		listen(sock, MX_LISTEN);


		

		while(!b_end){
			struct sockaddr sad;
			socklen_t sat;
			printf("waiting for connection...\n");
			int msgsock = accept(sock, &sad, &sat);
			if (msgsock == -1){
				printf("received msg error.\n");
			} else {
				/* received msg*/
				//sock_arg_t * s = (sock_arg_t *)malloc(sizeof(sock_arg_t));
				//if(!s){
				//	printf("%s: not enough memory\n", __FUNCTION__);
				//	continue;
				//}
				//s->sock = msgsock;
				pthread_mutex_lock(&mutex);
				int rr = queue_push(q, &msgsock, queue_node_clone_int);
				printf("pushing sock %d to queue\n", msgsock);
				if (!rr){
					printf("%s: queue is full", __FUNCTION__);
				} else {
					printf("sock %d pushed to queue\n", msgsock);
					pthread_cond_signal(&cond);
				}
				pthread_mutex_unlock(&mutex);
			}
		}
		close(sock);
		/*unlink file, otherwise, other socket cannot connect*/
		unlink(SERVER_PATH_NAME);
		printf("main thread quits\n");
		
		i=0;
		while(i < NUM_OF_WORKING_THREADS){
			pthread_join(threads[i], NULL);
			i++;
		}
		pthread_attr_destroy(&attr);
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}
done_main:
	//destroy shared memories, alloc, lru, queue, etc.
	queue_destruct(q);
	lru_cache_destruct(lru);
	zalloc_destruct(NULL, alloc);	
	return 0;

}
