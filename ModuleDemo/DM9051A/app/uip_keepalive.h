#ifndef _UIP_KEEPALIVE_H_
#define _UIP_KEEPALIVE_H_

#include <stdint.h>
#include "uip_arp.h"

struct keepalive_function_type
{
    struct uip_eth_addr ethaddr;
    uint32_t ka_no_data_tick;
    uint32_t ka_noack_tick;
    uint8_t  ka_tick_flag;
    uint8_t  ka_send_count;
    uint8_t ka_ackno_tmp[4];
    uint8_t ka_seqno_tmp[4];
};

extern struct keepalive_function_type ka_type;

void keepalive_init(void);
void keepalive_update_send_seq(void);
void keepalive_update_recv_seq(void);
void keepalive_trans_function(void);

#endif
