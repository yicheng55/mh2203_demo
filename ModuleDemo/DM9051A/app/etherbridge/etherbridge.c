#include "includes.h"
#include "uip_init.h"
#include "stdio.h"
#include "app_call.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "uip_keepalive.h"
#include "at_port.h"

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define BUFD ((u8_t *)&uip_buf[UIP_LLH_LEN + UIP_TCPIP_HLEN])
#define UDPBUFD ((u8_t *)&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])

uint16_t u_txlen = 0;
uint8_t uartTXbuf1[TransData_Size];

BUF_NO Free_Buf_Now = BUF_NO0;
BUF_NO Full_Buf_Now = BUF_NO0;
boolean Buf_Ok;
uint8_t EthernetInitDoneFlag = 0;

char reconn_addr[64];
char reconn_msg_tmp[] = {"Config Mode~~~!!!"};

struct tcp_bridge_state_st {
  char state;
  char served;
} tcp_bridge_state = { 0, 0 };

void uart_tx(uint8_t flag)
{
	uint16_t i;

	if(flag == UIP_PROTO_TCP){
		memcpy(uartTXbuf1, BUFD, u_txlen);
	}else if(flag == UIP_PROTO_UDP) {
		memcpy(uartTXbuf1, UDPBUFD, u_txlen);
	}

	for(i=0; i < u_txlen; i++){
		atp_uart_putc(uartTXbuf1[i]);
	}
}

uint8_t get_udp_conn(void)
{
	uint8_t c, n = 0;
	for(c = 0; c < UIP_UDP_CONNS; c++) {
		if (uip_udp_conns[c].lport)
			n++;
	}
	return n;
}

uint8_t check_udp_conn(char *head)
{
	uint8_t c;
	for(c = 0; c < UIP_UDP_CONNS; c++) {
		if (uip_udp_conns[c].lport)
			printf("%s:check.upd_conn[%u] %d:%d:%d:%d rport %u lport %u (%d/%d)\r\n", head, c,
				uip_ipaddr1(uip_udp_conns[c].ripaddr), uip_ipaddr2(uip_udp_conns[c].ripaddr),
				uip_ipaddr3(uip_udp_conns[c].ripaddr), uip_ipaddr4(uip_udp_conns[c].ripaddr),
				HTONS(uip_udp_conns[c].rport), HTONS(uip_udp_conns[c].lport),
				c, UIP_UDP_CONNS);
	}
	return get_udp_conn();
}

uint8_t get_tcp_conn(void)
{
	uint8_t c, n = 0;
	for(c = 0; c < UIP_CONNS; c++){
		if (uip_conns[c].tcpstateflags != UIP_CLOSED)
			n++;
	}
	return n;
}

uint8_t check_tcp_conn(char *head)
{
	uint8_t c;
	for(c = 0; c < UIP_CONNS; c++){
		if (uip_conns[c].tcpstateflags != UIP_CLOSED)
			printf("%s:check.tcp_conn[%u] %d:%d:%d:%d rport %u lport %u (%d/%d)\r\n", head, c,
				uip_ipaddr1(uip_conns[c].ripaddr), uip_ipaddr2(uip_conns[c].ripaddr),
				uip_ipaddr3(uip_conns[c].ripaddr), uip_ipaddr4(uip_conns[c].ripaddr),
				HTONS(uip_conns[c].rport), HTONS(uip_conns[c].lport),
				c, UIP_CONNS);
	}
	return get_tcp_conn();
}

void delete_udp_r_conn(uint16_t rport)
{
	uint8_t c;
	printf("udp.del rport %u all\r\n", rport);
	for(c = 0; c < UIP_UDP_CONNS; c++){
		if ((HTONS(uip_udp_conns[c].rport) == rport)){
			uip_udp_conns[c].lport = 0;
			uip_udp_conns[c].rport = 0;
		}
	}
}

void delete_udp_conn(uint16_t lport, uint16_t rport)
{
	uint8_t c;
	uint8_t d = 0, n = 0;
	n = check_udp_conn("s");
	printf("ops: del udp conn: rport %u lport %u\r\n", rport, lport);
	for(c = 0; c < UIP_UDP_CONNS; c++){
		if(((lport == 0) && (rport== 0)) ||
		   ((HTONS(uip_udp_conns[c].lport) == lport) && (HTONS(uip_udp_conns[c].rport) == rport))){
			printf("del.upd_conn[%u] rport %u lport %u\r\n", c, HTONS(uip_udp_conns[c].rport), HTONS(uip_udp_conns[c].lport));
			uip_udp_conns[c].lport = 0;
			uip_udp_conns[c].rport = 0;
			if ((lport == 0) && (rport== 0)) {
				printf("udp del all %u of %u\r\n", ++d, n);
				continue;
			} else {
				printf("udp del %u of %u\r\n", ++d, n);
				break;
			}
		}
	}
	n = check_udp_conn("e");
}

void recv_cmd_auto_disconnect(uint8_t state)
{
		uint8_t c;
		void *tmp_buf;

		if(state == UIP_PROTO_TCP){
			tmp_buf = uip_appdata;
		}else if(state == ROLE_UDP_SCLIENT){
			tmp_buf = g_u8RecData;
		}

		if((!strcasecmp(tmp_buf, "+++")) || (!strcasecmp(tmp_buf, "+++\r\n")))
		{
				if(tcp_connected == TRUE){
					check_tcp_conn("tcp.still");
					if (at_type.role == 0) {
						for(c = 0; c < UIP_CONNS; c++){
							if(uip_conns[c].lport == HTONS(at_type.t_lport)) {
								if(uip_conns[c].tcpstateflags == UIP_ESTABLISHED) {
									printf("[SERVER REQ] an appcall to uip_close, tcp_conn[%u] %d:%d:%d:%d rport %u lport %u\r\n", c,
											uip_ipaddr1(uip_conns[c].ripaddr), uip_ipaddr2(uip_conns[c].ripaddr),
											uip_ipaddr3(uip_conns[c].ripaddr), uip_ipaddr4(uip_conns[c].ripaddr),
											HTONS(uip_conns[c].rport), HTONS(uip_conns[c].lport));
									tcp_bridge_state.state = 1;
								}
							}
						}
					}
					if (at_type.role == 1 || at_type.role == 2) {
						for(c = 0; c < UIP_CONNS; c++){
							if(uip_conns[c].rport == HTONS(at_type.tcp_rport)) {
								if(uip_conns[c].tcpstateflags == UIP_ESTABLISHED) {
									printf("client To appcall to uip_close, tcp_conn[%u] %d:%d:%d:%d rport %u lport %u\r\n", c,
											uip_ipaddr1(uip_conns[c].ripaddr), uip_ipaddr2(uip_conns[c].ripaddr),
											uip_ipaddr3(uip_conns[c].ripaddr), uip_ipaddr4(uip_conns[c].ripaddr),
											HTONS(uip_conns[c].rport), HTONS(uip_conns[c].lport));
									tcp_bridge_state.state = 1;
								}
							}
						}
					}
					if (get_tcp_conn() == 0) {
						tcp_connected = FALSE;
						printf("[AFTER-APP-CALL] [be] 0 tcp conn / NO-WAY \n");
					} else
						printf("[AFTER-APP-CALL] [will to close] %u tcp conn\n", get_tcp_conn());

				}else if (udp_connected & UPDATE_UDP_SEND){
					if (at_type.role == 3) {
						delete_udp_conn(0, 0);
					}
					else
						delete_udp_conn(at_type.u_lport, at_type.udp_rport);
					if (check_udp_conn("the+++") == 0) {
						udp_connected = UPDATE_UDP_NULL;
						printf("[NO-WAY] 0 udp conn\n");
					} else {
						printf("still %u udp conn\n", check_udp_conn("+++"));
						return;
					}
				}

				memset(g_u8RecData, 0, RecData_Size);
				atcmd_flag = FALSE;
				keepalive_init();
				atcmd_resp_cmd("Quit OK");
				strcpy((char *)reconn_addr, reconn_msg_tmp);
				printf("+++ discnnt\r\n");
		}

		printf("[stage.1] .role %u\r\n", at_type.role);
		printf("\r\n");
}

void bridge_clear_state(void)
{
	if (tcp_bridge_state.state || tcp_bridge_state.served)
		printf("clear_state ON tcp_bridge_state.state %u, tcp_bridge_state.served %u\r\n", tcp_bridge_state.state, tcp_bridge_state.served);

	if (tcp_bridge_state.state && tcp_bridge_state.served) {
		tcp_bridge_state.state = tcp_bridge_state.served = 0;
		if (get_tcp_conn() == 0) {
			tcp_connected = FALSE;
			printf("[SERVED-APP-CALL] [now is] 0 tcp conn\n");
		} else
			printf("[SERVED-APP-CALL] [now still has] %u tcp conn\n", check_tcp_conn("tcp.still"));
	}
}

void tcp_bridge_appcall(void)
{
	if(uip_connected()) {
		atcmd_resp_establish();
		tcp_connected = TRUE;
		Buf_Ok = FALSE;
		sprintf((char *)reconn_addr, "TCP Connect:%d.%d.%d.%d:%d",uip_ipaddr1(BUF->srcipaddr), uip_ipaddr2(BUF->srcipaddr),
			uip_ipaddr3(BUF->srcipaddr), uip_ipaddr4(BUF->srcipaddr), HTONS(BUF->srcport));
	}

	if (tcp_bridge_state.state) {
		if (at_type.role == 0) {
			if (uip_conn->lport == HTONS(at_type.t_lport)) {
				uip_conn->tcpstateflags = UIP_CLOSED;
				uip_abort();
				printf("[HERE] server appcall set stateflags UIP_CLOSED, uip_conn-> %d:%d:%d:%d rport %u lport %u\r\n",
											uip_ipaddr1(uip_conn->ripaddr), uip_ipaddr2(uip_conn->ripaddr),
											uip_ipaddr3(uip_conn->ripaddr), uip_ipaddr4(uip_conn->ripaddr),
											HTONS(uip_conn->rport), HTONS(uip_conn->lport));
				tcp_bridge_state.served = 1;
			}
		}
		if (at_type.role == 1 || at_type.role == 2) {
			if(uip_conn->rport == HTONS(at_type.tcp_rport)) {
				uip_abort();
				printf("[HERE] client appcall do uip_abort, uip_conn-> %d:%d:%d:%d rport %u lport %u\r\n",
											uip_ipaddr1(uip_conn->ripaddr), uip_ipaddr2(uip_conn->ripaddr),
											uip_ipaddr3(uip_conn->ripaddr), uip_ipaddr4(uip_conn->ripaddr),
											HTONS(uip_conn->rport), HTONS(uip_conn->lport));
				tcp_bridge_state.served = 1;
			}
		}
		return;
	}

	if(uip_closed() || uip_aborted()) {
		if(tcp_connected == TRUE){
		  if (get_tcp_conn() == 0) {
			printf("tcp 0 conn, end\n");
			strcpy((char *)reconn_addr, reconn_msg_tmp);
			keepalive_init();
			atcmd_resp_cmd("Bridge connection closed...\r\n");
			memset(g_u8RecData, 0, sizeof(g_u8RecData));
			atcmd_flag = FALSE;
			tcp_connected = FALSE;
		  } else
			printf("tcp %u conn, no end\n", check_tcp_conn("tcp.still-not-closed"));
		}
	}

	if(uip_timedout()){
		if((at_type.role == 1) || (at_type.role == 2)){
		 atcmd_resp_connect_timeout(&at_type.tcp_raddr, at_type.tcp_rport);
		} else {
		 atcmd_resp_connect_timeout(&at_type.udp_raddr, at_type.udp_rport);
		}
	}

	if(uip_newdata()) {
		if(uip_datalen() > TransData_Size)
				uip_len = TransData_Size;

		u_txlen = uip_len;
		if(u_txlen > 0){
			uart_tx(UIP_PROTO_TCP);
			u_txlen = 0;
		}
		keepalive_init();
	}

	if(atcmd_flag && tcp_connected) {
		if((at_type.trans_len != 0) && gt_u32comRbytes >= at_type.trans_len){
			uip_send(g_u8RecData, at_type.trans_len);
			atcmd_flag = FALSE;
			gt_u32comRbytes = 0;
		}else if(at_type.trans_len == 0){
			uip_send(g_u8RecData, gt_u32comRbytes);
			gt_comeDataUsart2++;
			printf("tcp send %u/%u\n", gt_comeDataUsart2, get_tcp_conn());
			if (get_tcp_conn() == gt_comeDataUsart2) {
				check_tcp_conn("tcp.sent.to");
				memset(g_u8RecData, 0, RecData_Size);
				gt_u32comRbytes = 0;
				atcmd_flag = FALSE;
			}
		}
	}
}

void udp_bridge_appcall(void)
{
	if(uip_newdata()) {
	if (at_type.role == 3 ||
		((at_type.role != 3) && uip_ipaddr_cmp(IPBUF->srcipaddr, at_type.udp_raddr)))
	{
	   if ( UDPBUF->destport == HTONS(at_type.u_lport) || UDPBUF->srcport == HTONS(at_type.udp_rport)) {
			if(uip_datalen() > TransData_Size)
				uip_len = TransData_Size;

				if (!check_udp_conn("[udp_bridge_appcall To UDP insert]:"))
					printf("[udp_bridge_appcall To UDP insert]: upd_conn[0] (First )\r\n");

				do {
					uint16_t pktsrcport = (((u16)(uip_buf[34] << 8) | uip_buf[35]));
					uint16_t pktdestport = (((u16)(uip_buf[36] << 8) | uip_buf[37]));

					printf(".DBG: udp lport %u, pkt srcport %u, destport %u\r\n", at_type.u_lport, pktsrcport, pktdestport);

					if (at_type.u_lport == pktdestport) {
						udp_connected |= UPDATE_UDP_CONNECTED;
						printf("Equal : lport %u and packet's destport %u\r\n", at_type.u_lport, pktdestport);
						printf("[Operate]: udp_connected |= UPDATE_UDP_CONNECTED\r\n");
						uip_arp_update(IPBUF->srcipaddr, &IPBUF->ethhdr.src);
					}
				} while(0);

				sprintf((char *)reconn_addr, "New UDPConn:%d.%d.%d.%d : %d",
						uip_ipaddr1(UDPBUF->srcipaddr), uip_ipaddr2(UDPBUF->srcipaddr),
						uip_ipaddr3(UDPBUF->srcipaddr), uip_ipaddr4(UDPBUF->srcipaddr),  HTONS(UDPBUF->srcport));
			}
			printf(".[This udp newdata play 'reconn_addr' as: %s]\r\n", reconn_addr);
			printf("\r\n");

			u_txlen = uip_len;
			if(u_txlen > 0){
				uart_tx(UIP_PROTO_UDP);
				u_txlen = 0;
			}
	   }
	 }

	if(atcmd_flag && (udp_connected & UPDATE_UDP_SEND)) {

		recv_cmd_auto_disconnect(ROLE_UDP_SCLIENT);
		if (!(udp_connected & UPDATE_UDP_SEND))
			;
		else
		{
			check_udp_conn("to send");
			uip_send(g_u8RecData, gt_u32comRbytes);
			gt_comeDataUsart2++;
			printf("udp send %u/%u\n", gt_comeDataUsart2, get_udp_conn());

			printf("my.send.enum:check.upd_conn[.] [%d/%d]\r\n",
				gt_comeDataUsart2, UIP_UDP_CONNS);

			if (get_udp_conn() == gt_comeDataUsart2) {
				memset(g_u8RecData, 0, RecData_Size);
				gt_u32comRbytes = 0;
				atcmd_flag = FALSE;
			}
		}
	}
}

void bridge_init(void)
{
	if(EthernetInitDoneFlag == 1){
		EthernetInitDoneFlag = 0;

		if((at_type.role == 0) || (at_type.role == 1) || (at_type.role == 2))
		{
			atp_uart_rx_clear();
		}else{
			atp_uart_rx_clear();
		}

		strcpy((char *)reconn_addr, reconn_msg_tmp);

		switch(at_type.role)
		{
			case 0:
				atcmd_tcpip_tcplport();
			break;
			case 2:
				atcmd_tcpip_tcpconn();
			break;
			case 3:
				atcmd_tcpip_udplport();
			break;
			case 5:
				atcmd_tcpip_udpconn(ROLE_UDP_SCLIENT);
			break;
			default:
			break;
		}
	}
}
