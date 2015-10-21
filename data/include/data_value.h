typedef enum{
	rp_zero = 0,
	rp_open,
	rp_high,
	rp_low,
	rp_close,
	rp_ma_5,
	rp_ma_10,
	rp_ma_20,
	rp_ma_30,
	rp_ma_40,
	rp_ma_50,
	rp_ma_60,
	rp_ma_100,
	rp_ma_200,
	rp_ema_5,
	rp_ema_10,
	rp_ema_12,
	rp_ema_20,
	rp_ema_26,
	rp_ema_30,
	rp_ema_40,
	rp_ema_50,
	rp_ema_60,
	rp_ema_100,
	rp_ema_200,
	rp_bollinger_20,
	rp_macd_line,
	rp_macd_signal,
	rp_macd_diff,
	rp_rsi,
	rp_cci,
	rp_MAX_PARAM
}regular_parameter;
//#define PARAMETER(name) rp_##name##

//#define rp_MAX_PARAM 256
#define PARAMETER_NAME_MAX_LEN 15
typedef struct kv_pair_s{
	char k[PARAMETER_NAME_MAX_LEN];
	double v;
	struct kv_pair_s * next;
	struct kv_pair_s * prev;
}kv_pair_t;

typedef struct data_value_s{
	double parameters[rp_MAX_PARAM];
	int has_parameters[rp_MAX_PARAM];
	kv_pair_t * head; 
						/*head of non-regular parameters
						*linked list 
						*/
}data_value_t;

int value_clone_cbf_for_data(void **, const void *);
void value_free_cbf_for_data(void *);
char * value_to_string_for_data(const void *);

