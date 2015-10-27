#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH_NAME "_server_file_"
#define MSG_LENGTH 1024

/*shared memory*/
lru_cache_t * lru = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * thread_main(void * arg){
	int sock, msgsock, rval;
	struct sockaddr_un server;
	char buf[MSG_LENGTH];
	int b_end = 0;
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
	printf("thread %s is running, socket has name %s\n", __FUNCTION__, server.sun_path);
	/* listen, max 10 connections waiting*/
	listen(sock, 10);
	while(1){
		printf("waiting for connection...\n");
		msgsock = accept(sock, 0, 0);
		if(msgsock == -1){
			printf("received msg error.\n");
		}
		else{
			rval = 1;
			while(rval > 0){
				bzero(buf, sizeof(buf));
				rval = read(msgsock, buf, MSG_LENGTH);
				if(rval<0){
					printf("msg read error\n");
				}
				else if(rval == 0){
					printf("ending connection\n");
				}
				else{
					printf("-->%s\n", buf);
					if(strncmp(buf, "END", 3) == 0){
						b_end = 1;
					}
				}
			}
		}
		close(msgsock);
		if(b_end) break;
	}
	close(sock);
	/*unlink file, otherwise, other socket cannot connect*/
	unlink(SERVER_PATH_NAME);
	printf("thread %s quits\n", __FUNCTION__);
	return 0;
}

int main(int argc, char * argv[]){
	pthread_t t1;
	int res = 0;
	if(argc == 2){
		if(!strcmp(argv[1], "start")){
			pthread_mutex_lock(&mutex);
			res = lru_cache_init_wrap(&lru);
			pthread_mutex_unlock(&mutex);
			if(!lru || !res){
				printf("%s: failed to initialize cache, quit program\n", __FUNCTION__);
				return 0;
			}
			pthread_create(&t1, NULL, &thread_main, NULL);
			pthread_join(t1, NULL);
			printf("thread joined\n");
		}		
	}else{
		printf("invalid num. of arguments\n");
	}
	return 0;

}
