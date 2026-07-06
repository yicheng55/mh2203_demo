#ifndef __APP_CALL__
#define __APP_CALL__

#include "webserver.h"

#define TCP_APP_EN               1
#define UDP_APP_EN               1

#define DHCPC_EN                 0

typedef int uip_udp_appstate_t;

#define UIP_UDP_APPCALL udp_appcall

void tcp_appcall(void);
void udp_appcall(void);

#endif
