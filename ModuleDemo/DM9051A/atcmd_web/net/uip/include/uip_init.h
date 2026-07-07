#ifndef _UIP_INIT_H_
#define _UIP_INIT_H_

#include "stdint.h"

extern struct timer dhcp_timer;
extern uint8_t eth_netif_linkup;
//extern uint8_t check_DM9051_link;

#define DHCPC_EN        1

void dhcpc_configured(const struct dhcpc_state *s);

void tcpip_init(void);
void tcpip_process(void);
#endif /* _UIP_INIT_H_ */
