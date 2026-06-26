
//#include "includes.h"
#include <string.h>
#include  <stdio.h>
#include "uip.h"
#include "tcp_app.h"

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
	
void tcp_client_server_appcall(void)
{
 	struct tcp_state *s_tcp = (struct tcp_state *)&uip_conn->appstate;
	static int connected_count = 0;
	// -----------------------------------------
	// listen 5100: uip_conn->lport == 5100
	//
	
	// -----------------------------------------
	// connect to 5002: uip_conn->rport == 5002
	//
	if(uip_connected()){
		u8_t raddr[16];
		connected_count = 1;
		sprintf((char *)raddr, "%d.%d.%d.%d",uip_ipaddr1(BUF->srcipaddr), uip_ipaddr2(BUF->srcipaddr),
				uip_ipaddr3(BUF->srcipaddr), uip_ipaddr4(BUF->srcipaddr));
		printf("Connected lport:%d, rip:%s, rport:%d \r\n", HTONS(uip_conn->lport), raddr, HTONS(uip_conn->rport));
	
		//[not this way!]
		if (uip_newdata() && uip_len){
			s_tcp->textlen = uip_len;
			memcpy(&s_tcp->textptr, uip_appdata, uip_len);
			//printf("Connected, uip_newdata() && uip_len = %d\r\n",uip_len);
		}
		
		if (1){
			char clientdata[] = {
				'h','8','h','d','8', //'g','h','d','8','h',
				'A','B','C','D','E', //'F','G','H','I','J',
				'9','0','5','1', 0,
			};
			uip_send(clientdata, 14);
			printf("After connected send uip_sappdata = %s\r\n", clientdata);
			printf("After connected send uip_slen = %d\r\n", 14);
		}
		return;
	}

	////测试是否发生对应的错误
	if(uip_aborted() | uip_timedout()){
		s_tcp->textptr = 0;
		s_tcp->textlen = 0;
		uip_abort();
		printf(";_appcall, uip_abort() assign uip_flags = %02x\r\n", UIP_ABORT);
	}
	if (uip_acked()){
	}
	
	if (uip_newdata()){
		if (uip_len) { //newdata[Print for display]
			((char *)uip_appdata)[uip_len] = 0;
			printf("Receive.Len: %d\r\n", uip_len);
			printf("Receive.Rx: %s\r\n", (char *)uip_appdata);
		}
		
		//memcpy(uip_appdata, 0, sizeof(uip_appdata));
		
		/*if(uip_datalen() > tcp_buffer_size)
			uip_len = tcp_buffer_size;
		*/
		s_tcp->textlen = uip_len;
		//printf("uip_len = %d\r\n",uip_len);
		memcpy(&s_tcp->textptr, uip_appdata, uip_len);
	}
	
	if (uip_rexmit() ||
		uip_newdata() || 
		uip_acked()){
		if (uip_newdata()){
			if (uip_len) { //newdata[Echo function]
				printf("Client.do.Echo: %s\r\n", (char *)uip_appdata);
				uip_send(uip_appdata, uip_len);
			}
		}
	} else if (uip_poll()){
		if (uip_closed()){
			//[Redo connect to tcp server.]
			tcp_client_init();
			connected_count = 0;
		}
	} else if (uip_closed()){ //[JJAdd]
		if (connected_count){
			printf(";_appcall ;track.uip-closed %d\r\n", connected_count);
			connected_count++;
			if (connected_count>2){
				tcp_client_init();
				connected_count = 0;
			}
		}
	}
 
	//[closed]
	if (uip_closed()){
		if (!uip_poll()){
		}
	}
	
	//[clean]
	if(uip_newdata() || uip_poll() || uip_rexmit() || uip_acked()){
		if(s_tcp->textlen > 0){
			//uip_send(s_tcp->textptr, s_tcp->textlen);
			//printf("s_tcp->textptr %s,s_tcp->textlen %d\r\n",s_tcp->textptr,s_tcp->textlen);
			s_tcp->textlen = 0;
		}
	}
}	//uip_closed, uip_abort


void tcp_server_init(void)
{
  uip_listen(HTONS(TCP_LISTEN_PORT));
}

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

void tcp_client_init()
{
	uip_ipaddr_t ipaddr;
	
	printf("[tcp client][connect] to rport %d\r\n", TCP_Remote_PORT);
	
	uip_ipaddr(&ipaddr,TCP_Remote_IP0, TCP_Remote_IP1, TCP_Remote_IP2, TCP_Remote_IP3);
	uip_conn = uip_connect(&ipaddr,htons(TCP_Remote_PORT));
 // printf("uip_conn->rport -- %d  --- %d\r\n",uip_conn->rport,uip_conn->lport);
	tcp_conn_list(NULL, 3);
}


