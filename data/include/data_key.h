#include "zalloc.h"
#define DATA_KEY_NAME_MAX_SIZE 20
#define DATA_KEY_TIME_MAX_SIZE 20




typedef enum data_update_period_e{
	UPDATE_MONTHLY,
	UPDATE_WEEKLY,
	UPDATE_DAILY,
	UPDATE_HOURLY,
	UPDATE_EVERY_30_MIN,
	UPDATE_EVERY_15_MIN,
	UPDATE_EVERY_5_MIN
}data_update_period;
typedef struct data_key_s{
	char key_name[DATA_KEY_NAME_MAX_SIZE];
	char key_time[DATA_KEY_TIME_MAX_SIZE];
	data_update_period update_period;
}data_key_t;

int key_cmp_cbf_for_data(const void *, const void *);
void * key_clone_cbf_for_data(alloc_t *, const void *);
void key_free_cbf_for_data(alloc_t *, void *);
int hash_function_for_data(const void *);
char * key_to_string_for_data(const void *);
