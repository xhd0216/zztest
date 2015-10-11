#include<string.h>

#define KEY_SIZE 20
#define VALUE_SIZE 20
typedef struct{
	char key[KEY_SIZE+1];
}test_key_t;


int key_cmp_cbf(const void * key1, const void * key2){
	if(!key1 || !key2) return 0;
	const test_key_t * p1 = (const test_key_t *) key1;
	const test_key_t * p2 = (const test_key_t *) key2;
	const char * c1 = p1->key;
	const char * c2 = p2->key;
	int ret = strcmp(c1, c2);
	return ret == 0?1:0;
}

void key_free_cbf(void * key){
	if(key) free((test_key_t *) key);
}
void value_free_cbf(void * v){
	if(v) free( v);
}	
int key_clone_cbf(void ** target, const void * source){
	*target = (void *)malloc(sizeof(test_key_t));
	if(!*target){
		printf("%s: failed to allocate mem for key\n", __FUNCTION__);
		return 0;
	}
	memcpy((*target), source, sizeof(test_key_t));
	return 1;

}
int value_clone_cbf(void ** target, const void * source){
	*target = (void *)malloc(VALUE_SIZE);
	if(!*target){
		printf("%s: failed to allocate mem for value\n", __FUNCTION__);
		return 0;
	}
	memcpy(*target, source, VALUE_SIZE);
	return 1;
}
char * key_to_string(const void * k){
	return ((test_key_t *) k)->key;
}
char * value_to_string(const void * v){
	return (char *)v;
}
