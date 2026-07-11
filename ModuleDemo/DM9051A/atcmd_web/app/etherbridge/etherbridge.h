/****************************************
 * etherbridge.h
 *
 ****************************************/
#ifndef __ETH_BRIDGE_H__
#define __ETH_BRIDGE_H__

#include "atcommand.h"
//#include "connopts.h"

typedef enum {BUF_NO0 = 0, BUF_NO1 = 1}BUF_NO; 

extern BUF_NO Free_Buf_Now; 
extern BUF_NO Full_Buf_Now;
extern boolean Buf_Ok; 
extern char reconn_addr[64];

//-----------------------------------------------------------------------
//void udp_bridge_discovery_arp(void);

void bridge_init(void);
void tcp_bridge_appcall(void);
void udp_bridge_appcall(void);
void senddata(const void *data, int len);
void recv_cmd_auto_disconnect(uint8_t state);

uint8_t check_udp_conn(char *head);
void delete_udp_r_conn(uint16_t rport);

#endif		//__ETH_BRIDGE_H__
