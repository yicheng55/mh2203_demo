#include <string.h>
#include  <stdio.h>
#include "uip.h"

void tcp_conn_list(char *HeadStr, u8_t nConnShow)
{
	u8_t c;
	if (HeadStr)
		printf("[uip][MAX_CONNECTIONS %d] tcpstateflags list (%s)\r\n", UIP_CONNS, HeadStr);
	else
		printf("[uip][MAX_CONNECTIONS %d] tcpstateflags list\r\n", UIP_CONNS);
	for(c = 0; c < UIP_CONNS && c < nConnShow; ++c) {
		printf("[uip] uip_conns[%d]->tcpstateflags= 0x%x", c, uip_conns[c].tcpstateflags);
		if (uip_conns[c].tcpstateflags == UIP_CLOSED) printf(" UIP_CLOSED");
		if (uip_conns[c].tcpstateflags == UIP_SYN_SENT) printf(" UIP_SYN_SENT");
		else
		if (uip_conns[c].tcpstateflags == UIP_ESTABLISHED) printf(" UIP_ESTABLISHED");
		else
		if (uip_conns[c].tcpstateflags) printf(" UIP_TCP_CHKING");
		printf("\r\n");
	}
}
