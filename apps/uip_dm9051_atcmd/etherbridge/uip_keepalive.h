#ifndef _UIP_KEEPALIVE_H_
#define _UIP_KEEPALIVE_H_

#include "stdint.h"
#include "uip_arp.h"

//#define KA_SEND_PERIOD    1   //in second. send Keepalive in every KA_SEND_PERIOD seconds
//#define NO_DATA_PERIOD    5 	//in second. if no data exchange between server and client time is longer than NO_DATA_PERIOD, 
															//the keepalive tick will start.
														
/********************************************************************************************/
struct keepalive_function_type
{
	struct uip_eth_addr ethaddr;
	
	uint32_t ka_no_data_tick; //no data trans, timer counter
	uint32_t ka_noack_tick;   //trans no ACK timer counter
	uint8_t  ka_tick_flag;    //timer start counter flag
	uint8_t  ka_send_count;   //trans counter
	
	uint8_t ka_ackno_tmp[4];  //last ack number temp buffer
	uint8_t ka_seqno_tmp[4];  //last sequence number temp buffer
};

extern struct keepalive_function_type ka_type; //keepalive struct

void keepalive_init(void);
void keepalive_update_send_seq(void);
void keepalive_update_recv_seq(void);
void keepalive_trans_function(void);

#endif /* _UIP_KEEPALIVE_H_ */
