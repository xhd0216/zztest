#define DATA_KEY_NAME_MAX_SIZE 20
#define DATA_KEY_TIME_SIZE 20



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
	char key_time[DATA_KEY_TIME_SIZE];
	data_update_period update_period;
}data_key_t;


