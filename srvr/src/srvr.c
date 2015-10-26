#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*shared memory*/
lru_cache_t * lru = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * thread_main(void * arg){
	printf("thread %s is running\n", __FUNCTION__);
	while(1){
		break;	

	}
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
