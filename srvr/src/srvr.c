#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
void * thread_main(void * arg){
	printf("thread %s is running\n", __FUNCTION__);

	printf("thread %s quits\n", __FUNCTION__);
	return 0;
}

int main(int argc, char * argv[]){
	pthread_t t1;
	if(argc == 2){
		if(!strcmp(argv[1], "start")){
			pthread_create(&t1, NULL, &thread_main, NULL);
			pthread_join(t1, NULL);
			printf("thread joined\n");
		}		
	}else{
		printf("invalid num. of arguments\n");
	}
	return 0;

}
