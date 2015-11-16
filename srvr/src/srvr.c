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
#define NUM_OF_WORKING_THREAD 4

#define MAX_QUEUE 64


#define SIGNAL_T SIGINT
void sig_handler(int sig){
	printf("got signal %d\n", sig);
	b_end = 1;
	unlink(SERVER_PATH_NAME);
	//(void)signal(sig, SIG_DFL);
}


/*shared memory*/
lru_cache_t * lru = NULL;
b_end = 0;/*signal that ends the process*/
queue_t * q = NULL;
pthread_cond_t cond;
pthread_mutex_t mutex;

typedef struct sock_arg_s{
	int sock;
}sock_arg_t;
void * worker(void * arg){
	pthread_mutex_lock(&mutex);
	while(!b_end){
		pthread_cond_wait(cond, mutex);
		printf("%s: got signal\n", __FUNCTION__);
	}
	pthread_mutex_unlock(&mutex);
}
int send_all(int socket, const void *buffer, size_t length)
{
    const void *ptr = buffer;//(const char*) buffer;
    while (length > 0)
    {
        int i = write(socket, ptr, length);
		printf("send() returns %d\n", i);
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
int main(int argc, char * argv[]){
	pthread_t t1;
	int res = 0;
	struct sigaction act;
	int b_end = 0;
	int sock, msgsock, rval;
	struct sockaddr_un server;
	char buf[MSG_LENGTH];
	
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
		while(!b_end){
			printf("waiting for connection...\n");
			msgsock = accept(sock, 0, 0);
			if (msgsock == -1){
				printf("received msg error.\n");
			} else {
				/* received msg*/
#if 0
				rval = 1;
				while(rval > 0){
					bzero(buf, sizeof(buf));
					rval = read(msgsock, buf, MSG_LENGTH);
					if (rval<0){
						printf("msg read error\n");
					} else if (rval == 0){
						printf("ending connection\n");
					} else {
						printf("msg read-->%s\n", buf);
						if(strcmp(buf, "END") == 0){
							b_end = 1;
							break;
						}
						const char * resp_buf = "got some msg\n";
						int ret = send_all(msgsock, (const void *)resp_buf, strlen(resp_buf));
						if (ret) {
							printf("msg sent\n");
						} else {
							printf("failed to send msg\n");
						}
					}
					rval = -1;
				}
				close(msgsock);
#endif
			}
			if(b_end) break;
		}
		close(sock);
		/*unlink file, otherwise, other socket cannot connect*/
		unlink(SERVER_PATH_NAME);
		printf("thread %s quits\n", __FUNCTION__);
	}		
	return 0;

}
