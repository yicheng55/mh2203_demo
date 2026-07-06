#include "uip_keepalive.h"

struct keepalive_function_type ka_type;

void keepalive_init(void)
{
	ka_type.ka_no_data_tick = 1;
	at_type.keepalive_no_data_period = 100;
}

void keepalive_update_send_seq(void)
{
}

void keepalive_update_recv_seq(void)
{
}

void keepalive_trans_function(void)
{
}
