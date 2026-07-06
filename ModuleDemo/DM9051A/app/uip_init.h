#ifndef _UIP_INIT_H_
#define _UIP_INIT_H_

#include <stdint.h>

extern uint8_t eth_netif_linkup;

void tcpip_init(void);
void tcpip_process(void);

#endif
