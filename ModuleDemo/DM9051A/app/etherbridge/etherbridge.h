#ifndef __ETHERBRIDGE_H__
#define __ETHERBRIDGE_H__

#include "atcommand.h"

#define BUF_NO0 0
#define BUF_NO1 1

extern uint16_t u_txlen;
extern uint8_t uartTXbuf1[];

void uart_tx(uint8_t flag);
void bridge_init(void);
void tcp_bridge_appcall(void);
void udp_bridge_appcall(void);
void bridge_clear_state(void);
void recv_cmd_auto_disconnect(uint8_t state);
uint8_t check_tcp_conn(char *head);
uint8_t get_tcp_conn(void);
uint8_t check_udp_conn(char *head);
uint8_t get_udp_conn(void);
void delete_udp_conn(uint16_t lport, uint16_t rport);
void delete_udp_r_conn(uint16_t rport);

#endif
