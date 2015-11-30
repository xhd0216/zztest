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

#define SERVER_PATH_NAME "./server_file/_server_file_"
#define MSG_LENGTH 1024
#define MX_LISTEN 10
#define NUM_OF_WORKING_THREADS 4

#define MAX_QUEUE 64




/*shared memory*/
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
typedef struct sock_arg_s{
	int sock;
}sock_arg_t;

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

#if 0
void * worker(void * arg){
	int index = *(int *)arg;
	sock_arg_t * s = 0;
	int msgsock;
//	int rval;
//	char buf[MSG_LENGTH];
	char resp_buf[128];

	sprintf(resp_buf, "thread %d got some msg\n", index);
	while(!b_end){
		pthread_mutex_lock(&mutex);
		while(!b_end && q->count == 0){
			pthread_cond_wait(&cond,&mutex);
		}
		if(b_end){
			printf("thread %d got end signal, quit\n", index);
			pthread_mutex_unlock(&mutex);
			break;
		}
		s = queue_pop(q); 
//		printf("thread %d: got connection %p\n", index, s);
		pthread_mutex_unlock(&mutex);
		msgsock = s->sock;
//		printf("thread %d: got sock: %d\n", index, msgsock);
		//rval = 1;
		//while(rval>0){
	/*		bzero(buf, sizeof(buf));
			//pthread_mutex_lock(&mutex);
			rval = read(msgsock, buf, MSG_LENGTH);
			//pthread_mutex_unlock(&mutex);
			if (rval < 0) {
				printf("thread %d: msg read error, errno=%d, s=%p\n", index, errno, s);
			} else if(rval == 0) {
				printf("thread %d: read the end of file, s=%p\n", index, s);
			} else {
				printf("thread %d: msg read --->  %s\n", index, buf);
				if (strcmp(buf, "END") == 0){
					printf("thread %d got end signal, s=%p\n", index, s);
					b_end = 1;
					break;
				}
			//	pthread_mutex_lock(&mutex);
				int ret = send_all(msgsock, (const void *) resp_buf, 
									strlen(resp_buf));
			//	pthread_mutex_unlock(&mutex);
				if(ret){
					printf("thread %d: msg send\n", index);
				} else {
					printf("thread %d: failed to send msg\n", index);
				}
				
			}*/
		//	rval = -1;
		//}
		close(msgsock);
		free(s);
		s = 0;
	}
	printf("thread %d exits\n", index);
	pthread_exit(NULL);
}
#endif
int main(int argc, char * argv[]){
	int res = 0;
	struct sigaction act;
	int b_end = 0;
	int sock, msgsock;
	struct sockaddr_un server;
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

		/*initialize share memory*/
		res = lru_cache_init_wrap(&lru);
	
		if (!lru || !res){
			printf("%s: failed to initialize cache, quit program\n",
					 __FUNCTION__);
			return 0;
		}
		
		res = queue_init(&q, MAX_QUEUE);
		if(!res || !q){
			printf("%s: failed to initialize queue, quit program\n", 
					__FUNCTION__);
			return 0;
		}
		/*create socket*/
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sock < 0){
			printf("%s: opening stream socket error: %d\n", 
					__FUNCTION__, sock);
			return 0;
		}
		printf("the connected socket is %d\n", sock);
		server.sun_family = AF_UNIX;
		strcpy(server.sun_path, SERVER_PATH_NAME);
		if(bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_un))){
			printf("%s: error in binding stream socket\n", __FUNCTION__);
			return 0;
		}
		chmod(SERVER_PATH_NAME, S_IRWXU|S_IRWXG|S_IROTH|S_IWOTH);
		
		printf("program is running, socket name: %s\n", server.sun_path);
		/* listen, max 10 connections waiting*/
		listen(sock, MX_LISTEN);


		/*initialize mutex and cond*/
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		
		int count = 1;
		char resp_buf[128];
		while(!b_end){
			printf("waiting for connection...\n");
			msgsock = accept(sock, 0, 0);
			if (msgsock == -1){
				printf("received msg error.\n");
			} else {
				/* received msg*/
				int rval;
				char buf[MSG_LENGTH];
				sprintf(resp_buf, "handle number: %d", count);
				rval = read(msgsock, buf, MSG_LENGTH);
				if (rval < 0) {
					printf("msg read error, count=%d, errno=%d\n", count, errno);
				} else if(rval == 0) {
					printf("read the end of file, count=%d\n", count);
				} else {
					printf("count %d: msg read --->  %s\n", count, buf);
					if (strcmp(buf, "END") == 0){
						printf("thread %d got end signal\n", count);
						b_end = 1;
						break;
					}
					int ret = send_all(msgsock, (const void *) resp_buf, 
									strlen(resp_buf));
					if(ret){
						printf("count %d: msg send\n", count);
					} else {
						printf("count %d: failed to send msg\n", count);
					}
				}
			}
			count++;
		}
		close(sock);
		/*unlink file, otherwise, other socket cannot connect*/
		unlink(SERVER_PATH_NAME);
		printf("main thread quits\n");
		
		pthread_attr_destroy(&attr);
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}
	return 0;

}
