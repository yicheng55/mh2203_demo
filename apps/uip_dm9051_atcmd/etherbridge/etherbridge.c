/*******************************************
 * etherbridge.c
 *
 *******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "at_port.h"
#include "app_call.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "uip_keepalive.h"
#include "dm9051_uip.h" /* dm9051_uip_output()，取代 AT32 tapdev_send() */
#include "udp_printf.h"

/* tapdev_send(): 原本呼叫 AT32 tapdev.c 把目前 uip_buf/uip_len 送出；
 * MH2203 版改走既有 dm9051_uip adapter 的輸出 API (adapter 本身不動)。 */
#define tapdev_send() dm9051_uip_output(uip_buf, uip_len)

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define BUFD ((u8_t *)&uip_buf[UIP_LLH_LEN + UIP_TCPIP_HLEN])
#define UDPBUFD ((u8_t *)&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])
	
/*** AT command *****************/
uint16_t u_txlen = 0;
uint8_t uartTXbuf1[TransData_Size]; // 1 length byte + 1 ID byte + 128 bytes payload
#ifdef ATCMD_UART_TX_DOUB_BUF
uint8_t uartTXbuf2[TransData_Size]; // 1 length byte + 1 ID byte + 128 bytes payload
BUF_NO Full_TXBuf_Now; // AT UART TX use dauble buffer full change buffer
#endif //ATCMD_UART_TX_DOUB_BUF

BUF_NO Free_Buf_Now;
BUF_NO Full_Buf_Now;
boolean Buf_Ok; 

char reconn_addr[64]; //remote connect address
char reconn_msg_tmp[] = {"Config Mode~~~!!!"};

struct tcp_bridge_state_st {
  char state;
  char served;
} tcp_bridge_state = { 0, 0 };
struct uip_conn *tcp_bridge_get_pending_tx_conn(void)
{
	uint8_t c;
	if (!atcmd_flag || (gt_u32comRbytes == 0) || !tcp_connected)
		return NULL;

	/* TCP 透傳目前採單一送出目標：
	 * 只回傳第一個符合條件的 ESTABLISHED conn，避免同一筆 UART 資料
	 * 被多個 client 重複送出；若之後要支援多 client，這裡需改成輪詢或
	 * 明確的分發策略。
	 */
	for(c = 0; c < UIP_CONNS; c++) {
		if ((uip_conns[c].tcpstateflags == UIP_ESTABLISHED) &&
		    (uip_conns[c].lport == HTONS(at_type.t_lport)))
			return &uip_conns[c];
	}
	return NULL;
}
static uint8_t udp_bridge_tx_index;

static uint8_t udp_bridge_conn_is_data(const struct uip_udp_conn *conn)
{
	if (conn == NULL)
		return 0;
	if (conn->lport != HTONS(at_type.u_lport))
		return 0;
	if (HTONS(conn->lport) == UDP_PRINTF_PORT)
		return 0;
	return (conn->rport != 0) ? 1u : 0u;
}

static uint8_t count_udp_bridge_tx_conn(void)
{
	uint8_t c, n = 0;
	for(c = 0; c < UIP_UDP_CONNS; c++) {
		if (udp_bridge_conn_is_data(&uip_udp_conns[c]))
			n++;
	}
	return n;
}

struct uip_udp_conn *udp_bridge_get_pending_tx_conn(void)
{
	uint8_t c, i, start;
	if (!atcmd_flag || (gt_u32comRbytes == 0))
		return NULL;
	if ((udp_connected & UPDATE_UDP_SEND) != UPDATE_UDP_SEND)
		return NULL;
	/* UDP 透傳是輪詢式分發：符合條件的 conn 會依序輪到，
	 * 方便同一筆 UART 資料送到多個 UDP 目的端；下一次會從
	 * udp_bridge_tx_index 之後繼續找。
	 */
	start = udp_bridge_tx_index;
	for(i = 0; i < UIP_UDP_CONNS; i++) {
		c = (uint8_t)((start + i) % UIP_UDP_CONNS);
		if (udp_bridge_conn_is_data(&uip_udp_conns[c])) {
			udp_bridge_tx_index = (uint8_t)((c + 1u) % UIP_UDP_CONNS);
			return &uip_udp_conns[c];
		}
	}
	return NULL;
}

/*
 * uart_tx(): Move ethernet data to UART1 buffer
 */
void uart_tx(uint8_t flag) 
{
#ifdef ATCMD_USART_TX_DMA	
	
#ifdef ATCMD_UART_TX_DOUB_BUF
			if(Full_TXBuf_Now == BUF_NO0){ //如果 BUF1 空闲，将 DMA 接收数据赋值给 BUF1 
				if(flag == UIP_PROTO_TCP) {
					memcpy(uartTXbuf1, BUFD, u_txlen);
					//ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)BUFD;	
				}else if(flag == UIP_PROTO_UDP){
					memcpy(uartTXbuf1, UDPBUFD, u_txlen);
					//ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)UDPBUFD;	
				}
				ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)uartTXbuf1;	
				Full_TXBuf_Now = BUF_NO1; 
			}else{
				if(flag == UIP_PROTO_TCP) {
					memcpy(uartTXbuf2, BUFD, u_txlen);
					//ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)BUFD;
				}else if(flag == UIP_PROTO_UDP){
					memcpy(uartTXbuf2, UDPBUFD, u_txlen);
					//ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)UDPBUFD;
				}
				ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)uartTXbuf2;
				Full_TXBuf_Now = BUF_NO0; 
			}
			
			//Buf_Ok = TRUE;
#else
			if(flag == UIP_PROTO_TCP) {
				memcpy(uartTXbuf1, BUFD, u_txlen);
				//ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)BUFD;
			}else if(flag == UIP_PROTO_UDP){
				memcpy(uartTXbuf1, UDPBUFD, u_txlen);
				//ATCMD_USART_DMA_TX_Channel->CMAR = (uint32_t)UDPBUFD;
			}
#endif //ATCMD_UART_TX_DOUB_BUF	
		
		AT32_CMD_USART_DMA_TX_Channel->CNDTR = u_txlen;
		USART_DMACmd(AT32_CMD_USART, USART_DMAReq_Tx, ENABLE);//Enable DMA RX 
		DMA_Cmd(AT32_CMD_USART_DMA_TX_Channel, ENABLE);
		
		while(!DMA_GetFlagStatus(AT32_CMD_USART_DMA_TX_TC_FLAG));
		
		DMA_ClearFlag(AT32_CMD_USART_DMA_TX_GL_FLAG);
		DMA_Cmd(AT32_CMD_USART_DMA_TX_Channel, DISABLE);
		USART_DMACmd(AT32_CMD_USART, USART_DMAReq_Tx, DISABLE);//Enable DMA RX 
		u_txlen = 0;
#else	
	uint16_t i;
		
	if(flag == UIP_PROTO_TCP){
		memcpy(uartTXbuf1, BUFD, u_txlen);		// move ethernet rx data from uip_buf to vartxBuffer.
	}else if(flag == UIP_PROTO_UDP) {
		memcpy(uartTXbuf1, UDPBUFD, u_txlen);
	}
	
	for(i=0; i < u_txlen; i++){
		u_writeuart1(uartTXbuf1[i]);
	}
#endif //ATCMD_USART_TX_DMA
}

#if 1
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
		if (uip_conns[c].tcpstateflags != UIP_CLOSED) //if (uip_conns[c].lport)
			n++;
	}
	return n;
}
uint8_t check_tcp_conn(char *head)
{
	uint8_t c;
	for(c = 0; c < UIP_CONNS; c++){
		if (uip_conns[c].tcpstateflags != UIP_CLOSED) //if (uip_conns[c].lport)
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

//	for(c = 0; c < UIP_UDP_CONNS; c++) {
//		if (uip_udp_conns[c].lport) {
//			n++;
//		}
//	}

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

//	if (c == UIP_UDP_CONNS) {
//		uint8_t n = 0;
//		for(c = 0; c < UIP_UDP_CONNS; c++)
//			if (uip_udp_conns[c].lport)
//				n++;
//		printf("-[WARN]- trans_level fail del udp conn, in total %u connected\r\n", n);
//	}

//	for(c = 0; c < UIP_UDP_CONNS; c++) {
//		if (uip_udp_conns[c].lport) {
//			n++;
//			printf("del, stil %u udpconn\n", n);
//		}
//	}
//	return n;
}
#endif

//void tcp_disconnect_callback() {
//    // Handle TCP disconnect event
//    printf("TCP connection disconnected.\n");
//    // Perform any cleanup or additional actions if needed
//}
//void disconnect_tcp_connection() {
//    // Close the TCP connection using uIP function
//    uip_close();
//}

/**
 * recv_cmd_auto_disconnect() : TCP/UDP receive +++ clear connection table.
 */
void recv_cmd_auto_disconnect(uint8_t state)
{
		uint8_t c;
		void *tmp_buf;
	
		if(state == UIP_PROTO_TCP){
			tmp_buf = uip_appdata;
		}else if(state == ROLE_UDP_SCLIENT){
#ifdef ATCMD_UART_RX_DOUB_BUF
			if(Free_Buf_Now == BUF_NO0){
				tmp_buf = g_u8RecData1;
			}else if(Free_Buf_Now == BUF_NO1){
				tmp_buf = g_u8RecData;
			}
#else
			tmp_buf = g_u8RecData;
#endif //ATCMD_UART_RX_DOUB_BUF
		}
		
		//receive +++ clear tcp connection table
		if((!strcasecmp(tmp_buf, "+++")) || (!strcasecmp(tmp_buf, "+++\r\n"))/* || (!strcasecmp(tmp_buf, "disconn"))*/)
		{
				if(tcp_connected == TRUE){ //current tcp connection
					check_tcp_conn("tcp.still");
					//effect as: disconnect all, when tcp-server
					//
					// To enum every UIP_CONNS, because 
					//  tcp server role 0, may multi [PC1 client connected][PC2 client connected]
					//  tcp client role 1/2, only connect to [PC1 server]
					//
					if (at_type.role == 0) {
						for(c = 0; c < UIP_CONNS; c++){ // clear tcp connect table
							if(uip_conns[c].lport == HTONS(at_type.t_lport)) {
								if(uip_conns[c].tcpstateflags == UIP_ESTABLISHED) {
//									uip_conns[c].tcpstateflags = UIP_CLOSED;
									printf("[SERVER REQ] an appcall to uip_close, tcp_conn[%u] %d:%d:%d:%d rport %u lport %u\r\n", c,
											uip_ipaddr1(uip_conns[c].ripaddr), uip_ipaddr2(uip_conns[c].ripaddr),
											uip_ipaddr3(uip_conns[c].ripaddr), uip_ipaddr4(uip_conns[c].ripaddr), 
											HTONS(uip_conns[c].rport), HTONS(uip_conns[c].lport));
									tcp_bridge_state.state = 1; //Todo [send [RST, ACK], 20240702]
								}
							}
						}
					}
					if (at_type.role == 1 || at_type.role == 2) {
						for(c = 0; c < UIP_CONNS; c++){ // clear tcp connect table
							if(uip_conns[c].rport == HTONS(at_type.tcp_rport)) {
								if(uip_conns[c].tcpstateflags == UIP_ESTABLISHED) {
//									uip_conns[c].tcpstateflags = UIP_CLOSED;
									printf("client To appcall to uip_close, tcp_conn[%u] %d:%d:%d:%d rport %u lport %u\r\n", c,
											uip_ipaddr1(uip_conns[c].ripaddr), uip_ipaddr2(uip_conns[c].ripaddr),
											uip_ipaddr3(uip_conns[c].ripaddr), uip_ipaddr4(uip_conns[c].ripaddr), 
											HTONS(uip_conns[c].rport), HTONS(uip_conns[c].lport));
									tcp_bridge_state.state = 1; //Todo [send [RST, ACK], 20240702]
								}
							}
						}
					}
#if 0
					//Todo [send [RST, ACK], 20240702]
					// .check how to relate to UIP_TIMER 
					// .Oh, Write the code in tcp_appcall.....
//					uip_conn->tcpstateflags |= UIP_CLOSE;
//					if (uip_conn->tcpstateflags & UIP_CLOSE) {
//						tcp_disconnect_callback();
//						disconnect_tcp_connection(); //,to ,uip_close();
//					}
					for(c = 0; c < UIP_CONNS; c++){ // clear tcp connect table		
						if((uip_conns[c].lport == HTONS(at_type.t_lport)) || (uip_conns[c].rport == HTONS(at_type.tcp_rport))){
							uip_conns[c].lport = 0;
							if(uip_conns[c].tcpstateflags == UIP_ESTABLISHED){
								uip_conns[c].tcpstateflags = UIP_CLOSED;
								//[need send [RST, ACK], 20240702]
							}
						}
					}
#endif
					
					/* only 0 tcp connection, to at-cmd mode */
					if (get_tcp_conn() == 0) {
						tcp_connected = FALSE;
						printf("[AFTER-APP-CALL] [be] 0 tcp conn / NO-WAY \n");
					} else
						printf("[AFTER-APP-CALL] [will to close] %u tcp conn\n", get_tcp_conn());

				}else if (udp_connected & UPDATE_UDP_SEND){ //current udp can send
//					for(c = 0; c < UIP_UDP_CONNS; c++){
//						if((uip_udp_conns[c].lport == HTONS(at_type.u_lport)) || (uip_udp_conns[c].rport == HTONS(at_type.udp_rport))){
//							uip_udp_conns[c].lport = 0;
//							uip_udp_conns[c].rport = 0;
//						}
//					}=
					/* only 0 udp connection, to at-cmd mode */
					if (at_type.role == 3) { //disconnect all, when udp-server
						delete_udp_conn(0, 0); //disconnect all, when udp-server
					}
					else
						delete_udp_conn(at_type.u_lport, at_type.udp_rport);
					if (check_udp_conn("the+++") == 0) {
						udp_connected = UPDATE_UDP_NULL; //NO-WAY
						printf("[NO-WAY] 0 udp conn\n");
					} else {
						printf("still %u udp conn\n", check_udp_conn("+++"));
						return; //goto..escap...
					}
				}
				
#ifdef ATCMD_UART_RX_DOUB_BUF
				memset(g_u8RecData, 0, RecData_Size);
				clear_second_buffer();
#else 
				memset(g_u8RecData, 0, RecData_Size);
#endif //ATCMD_UART_RX_DOUB_BUF
				
				atcmd_flag = FALSE; //for +++							
//				check_DM9051_link = 0;
				keepalive_init();
#if 0
				ka_type.ka_tick_flag = 0;
#endif
				atcmd_resp_cmd("Quit OK");
				
				//Clear remote connect ip & port
				strcpy((char *)reconn_addr, reconn_msg_tmp);
//				printf("+++ discnnt tcp_connected/udp_connected %x/%x \r\n", tcp_connected, (udp_connected & UPDATE_UDP_CONNECTED));
				printf("+++ discnnt\r\n");
		}
		
		printf("[stage.1] .role %u\r\n", at_type.role);
		printf("\r\n");
}

/*
 * bridge_appcall: procees TCP packet 
 */
void bridge_clear_state(void)
{
	if (tcp_bridge_state.state || tcp_bridge_state.served)
		printf("clear_state ON tcp_bridge_state.state %u, tcp_bridge_state.served %u\r\n", tcp_bridge_state.state, tcp_bridge_state.served);
		
	if (tcp_bridge_state.state && tcp_bridge_state.served) {
		//[send [RST, ACK], 20240702] all done
		tcp_bridge_state.state = tcp_bridge_state.served = 0;
		
					/* only 0 tcp connection, to at-cmd mode */
					if (get_tcp_conn() == 0) {
						tcp_connected = FALSE; //THIS-WAY
						printf("[SERVED-APP-CALL] [now is] 0 tcp conn\n");
					} else
						printf("[SERVED-APP-CALL] [now still has] %u tcp conn\n", check_tcp_conn("tcp.still"));
	}
}

void tcp_bridge_appcall(void) 
{		
	if(uip_connected()) {
		atcmd_resp_establish(); //atcmd_resp_cmd("Bridge connection established...\r\n");
		tcp_connected = TRUE;
		Buf_Ok = FALSE; //no data ready
//		check_DM9051_link = 1; //already connect don't check network line
#if 0
		if(at_type.keepalive_mode){
//			if(ka_type._ka_tick_flag == 0){//connect clear keepalive flag and trigger ka_type._ka_tick_flag
//				ka_type._ka_tick_flag = 1;
//			}
			ka_type.ka_tick_flag = 1;
		}
#endif
		sprintf((char *)reconn_addr, "TCP Connect:%d.%d.%d.%d:%d",uip_ipaddr1(BUF->srcipaddr), uip_ipaddr2(BUF->srcipaddr),
			uip_ipaddr3(BUF->srcipaddr), uip_ipaddr4(BUF->srcipaddr), HTONS(BUF->srcport));
	}

	if (tcp_bridge_state.state) { //Todo [send [RST, ACK], 20240702]
		if (at_type.role == 0) {
			if (uip_conn->lport == HTONS(at_type.t_lport)) {
//				.uip_close(); //tcp_disconnect_callback(),disconnect_tcp_connection()
//				.printf("here server appcall do uip_close, uip_conn-> %d:%d:%d:%d rport %u lport %u\r\n",
//				-uip_abort(); //.client_aborted
//				-printf("[HERE] server appcall do uip_abort, uip_conn-> %d:%d:%d:%d rport %u lport %u\r\n",
//											uip_ipaddr1(uip_conn->ripaddr), uip_ipaddr2(uip_conn->ripaddr),
//											uip_ipaddr3(uip_conn->ripaddr), uip_ipaddr4(uip_conn->ripaddr), 
//											HTONS(uip_conn->rport), HTONS(uip_conn->lport));
				uip_conn->tcpstateflags = UIP_CLOSED;
				uip_abort(); //.uip_close();
				/* Develop milestone, here NOT progresive to [FIN, ACK] out,
				 * But the test with test assistant got no found the issue,
				 * and could pass the test.
				 */
				printf("[HERE] server appcall set stateflags UIP_CLOSED, uip_conn-> %d:%d:%d:%d rport %u lport %u\r\n",
											uip_ipaddr1(uip_conn->ripaddr), uip_ipaddr2(uip_conn->ripaddr),
											uip_ipaddr3(uip_conn->ripaddr), uip_ipaddr4(uip_conn->ripaddr), 
											HTONS(uip_conn->rport), HTONS(uip_conn->lport));
				tcp_bridge_state.served = 1; //bridge_clear_state();
			}
		}
		if (at_type.role == 1 || at_type.role == 2) {
			if(uip_conn->rport == HTONS(at_type.tcp_rport)) {
				//uip_close(); //tcp_disconnect_callback(),disconnect_tcp_connection()
				uip_abort(); //.client_aborted
				printf("[HERE] client appcall do uip_abort, uip_conn-> %d:%d:%d:%d rport %u lport %u\r\n",
											uip_ipaddr1(uip_conn->ripaddr), uip_ipaddr2(uip_conn->ripaddr),
											uip_ipaddr3(uip_conn->ripaddr), uip_ipaddr4(uip_conn->ripaddr), 
											HTONS(uip_conn->rport), HTONS(uip_conn->lport));
				tcp_bridge_state.served = 1; //bridge_clear_state();
			}
		}
		return;
	}

	if(uip_closed() || uip_aborted()) {
		if(tcp_connected == TRUE){
		  /* only 1 tcp connection (multi-enter tobe 0 tcp connection), to at-cmd mode */
		  if (get_tcp_conn() == 0) {
			printf("tcp 0 conn, end\n");
			strcpy((char *)reconn_addr, reconn_msg_tmp);
			
			keepalive_init();
#if 0
			ka_type.ka_tick_flag = 0;
#endif
			
			atcmd_resp_cmd("Bridge connection closed...\r\n");
			
#ifdef ATCMD_UART_RX_DOUB_BUF
			memset(g_u8RecData, 0, sizeof(g_u8RecData));
			clear_second_buffer();
#else
			memset(g_u8RecData, 0, sizeof(g_u8RecData));
#endif //ATCMD_UART_RX_DOUB_BUF
			atcmd_flag = FALSE; //for all clear
//			check_DM9051_link = 0;
			tcp_connected = FALSE;
		  } else
			printf("tcp %u conn, no end\n", check_tcp_conn("tcp.still-not-closed"));
		}/*else{
			tcp_init();
			_EthernetInit_Done();
		}*/
	}
	
	if(uip_timedout()){
//		atcmd_resp_cmd("Bridge connection timeout...\r\n");
		if((at_type.role == 1) || (at_type.role == 2)){
		 atcmd_resp_connect_timeout(&at_type.tcp_raddr, at_type.tcp_rport);
//		 sprintf(at_tmpstr, "Bridge connection %d.%d.%d.%d timeout...",
//			uip_ipaddr1(at_type.tcp_raddr),uip_ipaddr2(at_type.tcp_raddr),uip_ipaddr3(at_type.tcp_raddr),uip_ipaddr4(at_type.tcp_raddr));
//		  atcmd_resp_cmd(at_tmpstr);
		} else {
		 atcmd_resp_connect_timeout(&at_type.udp_raddr, at_type.udp_rport);
//		 sprintf(at_tmpstr, "Bridge connection %d.%d.%d.%d timeout...",
//			uip_ipaddr1(at_type.udp_raddr),uip_ipaddr2(at_type.udp_raddr),uip_ipaddr3(at_type.udp_raddr),uip_ipaddr4(at_type.udp_raddr));
//		  atcmd_resp_cmd(at_tmpstr);
		}

		//tcp_init();
		//_EthernetInit_Done();
//		check_DM9051_link = 0;
	}
	
	if(uip_newdata()) {		//Ethernet data income
		if(uip_datalen() > TransData_Size)
				uip_len = TransData_Size;

#if 0
		//[Frank recommand! TCP data mode]
		recv_cmd_auto_disconnect(UIP_PROTO_TCP);
#endif
		
		u_txlen = uip_len;
		if(u_txlen > 0){
			uart_tx(UIP_PROTO_TCP);
			u_txlen = 0; // clear tx lenght
			
			//At-cmd, echo.baseline.
			//[will no need for custom's board, for only +++ sent.]
			#if 0
			//previous alrady! recv_cmd_auto_disconnect(UIP_PROTO_TCP);
			if((!strcasecmp(uartTXbuf1, "+++")) || (!strcasecmp(uartTXbuf1, "+++\r\n")))
				atcmd_baseline();
			#endif
		}

		//we think, when server respnse data to us, means connection is working.
		//Note: We send data to server is not right thing to trigger keepalive here!
		keepalive_init(); //ka_type._ka_no_data_tick= 0; //keepalive renew 
	}
	
	//--- UART1 income data ---
	if(atcmd_flag && tcp_connected) { //TCP
		if((at_type.trans_len != 0) && gt_u32comRbytes >= at_type.trans_len){
			uip_send(g_u8RecData, at_type.trans_len);	
			atcmd_flag = FALSE; //for on trans
			gt_u32comRbytes = 0;
		}else if(at_type.trans_len == 0){
#ifdef ATCMD_UART_RX_DOUB_BUF
			if(Buf_Ok == TRUE){
				if(Full_Buf_Now == BUF_NO0) //如果 BUF1 空闲，将 DMA 接收数据赋值给 BUF1 
				{ 
					uip_send(g_u8RecData, g_u32comRbytes);
					//printf("0 = %d\r\n",g_u32comRbytes);
					memset(g_u8RecData, 0, g_u32comRbytes);
					g_u32comRbytes = 0;
					Full_Buf_Now = BUF_NO1;
				}else{
					uip_send(g_u8RecData1, g_u32comRbytes1);
					//printf("1 = %d\r\n",g_u32comRbytes1);
					memset(g_u8RecData1, 0, g_u32comRbytes1);
					g_u32comRbytes1 = 0;
					Full_Buf_Now = BUF_NO0;
				}		
				
				if((g_u32comRbytes == 0) && (g_u32comRbytes1 == 0)){
					atcmd_flag = FALSE; //for on trans
					Buf_Ok = FALSE;
				}
			}
#else
			uip_send(g_u8RecData, gt_u32comRbytes);
			gt_comeDataUsart2++;
			
			printf("tcp send %u/%u\n", gt_comeDataUsart2, get_tcp_conn());
			
			if (gt_comeDataUsart2 >= get_tcp_conn()) {
				check_tcp_conn("tcp.sent.to");
				memset(g_u8RecData, 0, RecData_Size);
				gt_u32comRbytes = 0;
				gt_comeDataUsart2 = 0;
				atcmd_flag = FALSE; //for on trans
			}
#endif //ATCMD_UART_RX_DOUB_BUF
		}
	}	
	
}

/*
 * udp_uart_appcall(): process UDP packet for UART.
 */
void udp_bridge_appcall(void)
{
	//uint8_t c;
	//printf("0.0 udp_bridge_appcall uip_flags %x \r\n", uip_flags);
	if(uip_newdata()) {	//Ethernet data income
	
#if 0
	/* old
	 */
	if (uip_ipaddr_cmp(IPBUF->srcipaddr, at_type.udp_raddr))
#else
	/* UDP
	 * role3/role4/role5 only interest local port or in rhost ip and rhost rport. jos.
	 */
	if (at_type.role == 3 ||
		((at_type.role != 3) && uip_ipaddr_cmp(IPBUF->srcipaddr, at_type.udp_raddr)))
#endif
	{
	   if ( UDPBUF->destport == HTONS(at_type.u_lport) || UDPBUF->srcport == HTONS(at_type.udp_rport)) {
			if(uip_datalen() > TransData_Size)
				uip_len = TransData_Size;

			//printf("0. udp_bridge_appcall %x \r\n", uip_len);
			//if((at_type.role == ROLE_UDP_SERVER) && (UDPSrvConnFlag == 1)){
				//UDPSrvConnFlag = 0;
				//printf("destport = %d, srcport = %d, %d\r\n", UDPBUF->destport, UDPBUF->srcport, (u16)(uip_buf[34]<<8) | uip_buf[35]);

				if (!check_udp_conn("[udp_bridge_appcall To UDP insert]:"))
					printf("[udp_bridge_appcall To UDP insert]: upd_conn[0] (First )\r\n"); //this no-way!!

				/* uip_maintain the uip_udp_conns[c],
				 * we need only know if newdata can make 'udp_connected' true
				 */
				//for(c = 0; c < UIP_UDP_CONNS; ++c) {
				do {
					uint16_t pktsrcport = (((u16_t)(uip_buf[34] << 8) | uip_buf[35])); //HTONS()
					uint16_t pktdestport = (((u16_t)(uip_buf[36] << 8) | uip_buf[37])); //HTONS()
					
					printf(".DBG: udp lport %u, pkt srcport %u, destport %u\r\n", at_type.u_lport, pktsrcport, pktdestport);
					
					if (at_type.u_lport == pktdestport) {
						udp_connected |= UPDATE_UDP_CONNECTED; //udp_connected = TRUE;
						printf("Equal : lport %u and packet's destport %u\r\n", at_type.u_lport, pktdestport);
						printf("[Operate]: udp_connected |= UPDATE_UDP_CONNECTED\r\n");
						/* UDP server replies must target the client that sent the first packet. */
						uip_ipaddr_copy(uip_udp_conn->ripaddr, UDPBUF->srcipaddr);
						uip_udp_conn->rport = UDPBUF->srcport;
						
						/* role 3 interest in the connect-in client, jos
						 *
						 * 原本呼叫 uip_arp_update() 搶先把來訪 UDP client 的 IP/MAC 記進 ARP
						 * cache；目的專案的 uip_arp.c 把 uip_arp_update() 宣告成檔案內 static
						 * (見 middlewares/3rd_party/uip/src/uip_arp.c)，不對外匯出，也不修改
						 * 共用的 uip_arp.c 去匯出它。之後若真要送封包給這個 client，
						 * uip_arp_out() 仍會照 uIP 標準流程自動處理 ARP 解析，只是少了這裡的
						 * 搶先快取優化，功能不受影響。 */
					}
				} while(0);
				//}
				
//				for(c = 0; c < UIP_UDP_CONNS; ++c) {
//					//if((uip_udp_conns[c].lport == at_type.u_lport) && (uip_udp_conns[c].rport == 0)) {
//					//Stone add for UDP Server !!!
//					printf("%s:check.upd_conn[%u] rport %u lport %u (%d/%d)\r\n", "TRY",
//						c, HTONS(uip_udp_conns[c].rport), HTONS(uip_udp_conns[c].lport), c, UIP_UDP_CONNS);
//					
//					if(uip_udp_conns[c].lport == HTONS(at_type.u_lport)) 			
//					if(HTONS(uip_udp_conns[c].lport) == at_type.u_lport) 
//					{
//						uint16_t pktsrcport = HTONS(((u16)(uip_buf[34] << 8) | uip_buf[35]));
//					
//						printf("Assign change UDPConn[%u]:%d.%d.%d.%d", c,
//							uip_ipaddr1(uip_udp_conns[c].ripaddr), uip_ipaddr2(uip_udp_conns[c].ripaddr),
//							uip_ipaddr3(uip_udp_conns[c].ripaddr), uip_ipaddr4(uip_udp_conns[c].ripaddr));
//						printf(" to %d.%d.%d.%d\r\n",
//							uip_ipaddr1(UDPBUF->srcipaddr), uip_ipaddr2(UDPBUF->srcipaddr),
//							uip_ipaddr3(UDPBUF->srcipaddr), uip_ipaddr4(UDPBUF->srcipaddr));
//							
//						printf("Assign change rport[%u] %d", c, HTONS(uip_udp_conns[c].rport));
//						printf(" to %d\r\n", HTONS(pktsrcport));
//						
//						printf("[Operate]: udp_connected |= UPDATE_UDP_CONNECTED\r\n");
//						
//						uip_ipaddr_copy(&uip_udp_conns[c].ripaddr, UDPBUF->srcipaddr);
//						uip_udp_conns[c].rport = pktsrcport;
//						udp_connected |= UPDATE_UDP_CONNECTED; //udp_connected = TRUE;
//						
//						/* role 3 interest in the connect-in client, jos
//						 */
//						uip_arp_update(IPBUF->srcipaddr, &IPBUF->ethhdr.src);
//						
//						printf("%s:check.upd_conn[%u] rport %u lport %u, and Done (%d/%d)\r\n", "ADD as",
//							c, HTONS(uip_udp_conns[c].rport), HTONS(uip_udp_conns[c].lport), c, UIP_UDP_CONNS);
//						break;
//					} else {
//						printf("%s:check.upd_conn[%u] rport %u lport %u, goto next (%d/%d)\r\n", "NO touch",
//							c, HTONS(uip_udp_conns[c].rport), HTONS(uip_udp_conns[c].lport), c, UIP_UDP_CONNS);
//					}
//					//}
//				}
		
				sprintf((char *)reconn_addr, "New UDPConn:%d.%d.%d.%d : %d",
						uip_ipaddr1(UDPBUF->srcipaddr), uip_ipaddr2(UDPBUF->srcipaddr),
						uip_ipaddr3(UDPBUF->srcipaddr), uip_ipaddr4(UDPBUF->srcipaddr),  HTONS(UDPBUF->srcport));
			//}
			printf(".[This udp newdata play 'reconn_addr' as: %s]\r\n", reconn_addr);
			printf("\r\n");

#if 0
			//[Frank recommand! UDP data mode]
			recv_cmd_auto_disconnect(UIP_PROTO_TCP); //JJ?TCP, ok instead of UDP in this x_disconnect() ok		
#endif

			u_txlen = uip_len;
			if(u_txlen > 0){
				uart_tx(UIP_PROTO_UDP);
				u_txlen = 0;
			
				//At-cmd, echo.baseline.
				//[will no need for custom's board, for only +++ sent.]
				#if 0
				//previous alrady! recv_cmd_auto_disconnect(UIP_PROTO_TCP); //JJ?TCP,
				if((!strcasecmp(uartTXbuf1, "+++")) || (!strcasecmp(uartTXbuf1, "+++\r\n")))
					atcmd_baseline();
				#endif
			}
	   }
//	   else {
//			printf("UDPBUF->destport == at_type.u_lport || UDPBUF->srcport == at_type.udp_rport %u %u %u %u\r\n",
//				HTONS(UDPBUF->destport), at_type.u_lport, HTONS(UDPBUF->srcport), at_type.udp_rport);
//			printf("UDPBUF->destport == at_type.u_lport || UDPBUF->srcport == at_type.udp_rport %u %u %u %u\r\n",
//				UDPBUF->destport, HTONS(at_type.u_lport), UDPBUF->srcport, HTONS(at_type.udp_rport));
//			printf("[Wrong srcport %d destport %d !]\r\n", 
//				HTONS(UDPBUF->srcport), HTONS(UDPBUF->destport));
//		}
	 }
	 else
	   printf("[Err src ip %d.%d.%d.%d]\r\n", IPBUF->srcipaddr[0] & 0xff, IPBUF->srcipaddr[0] >> 8, 
								IPBUF->srcipaddr[1] & 0xff, IPBUF->srcipaddr[1] >> 8);
	} 
	
	//--- UART2 income data ---
	if(atcmd_flag && ((udp_connected & UPDATE_UDP_SEND) == UPDATE_UDP_SEND)) { //UDP
		printf("[UART->ETH] rx_len=%u dst=%d.%d.%d.%d:%u role=%u\r\n",
			gt_u32comRbytes,
			uip_ipaddr1(uip_udp_conn->ripaddr), uip_ipaddr2(uip_udp_conn->ripaddr),
			uip_ipaddr3(uip_udp_conn->ripaddr), uip_ipaddr4(uip_udp_conn->ripaddr),
			HTONS(uip_udp_conn->rport), at_type.role);
	
	#if 1
//		printf("\r\n[DBG --- UART2 income data --- udp_connected 0x%x ] To Eth-Tx.\r\n", udp_connected);

		recv_cmd_auto_disconnect(ROLE_UDP_SCLIENT);// reveive +++ clear connect table
		if ((udp_connected & UPDATE_UDP_SEND) != UPDATE_UDP_SEND)
			//NEXT, be set to UPDATE_UDP_SEND again.
			; //printf("[DBG --- udp_connected disconnect --- udp_connected 0x%x ]\r\n", udp_connected);
		else
		{
	#endif
	
			check_udp_conn("to send");
//		  	display_udp_conn("to send UART2 data,show udp conn,");
//			printf("[--- UART2 income data --- ] %s\r\n", at_tmpstr);
			#if 1 //need arp_out?
			//if (0)
			if (arp_need()) 
			{
				if (arp_need_limit()) { //1; or 0;
//					printf("[--- UART2 income data --- ] ARP instead, Not send\r\n");
					uip_arp_out();
					tapdev_send();
				} else {
					sprintf(at_tmpstr, "No route, not send"); //"drop, No send"
					atcmd_resp_cmd(at_tmpstr);
					
					arp_need_new(); //give up
					/* No route, we give up this UART2 incoming dada. */
					/* as if(at_type.trans_len == 0) below, near below: */
					memset(g_u8RecData, 0, RecData_Size);
					gt_u32comRbytes = 0;
					atcmd_flag = FALSE; //for udp on trans
				}
			} 
			else 
			{
			#endif
	
				if((at_type.trans_len != 0) && gt_u32comRbytes >= at_type.trans_len){

					printf("[UART->ETH] send(fixed) len=%u\r\n", at_type.trans_len);
					uip_send(g_u8RecData, at_type.trans_len);
					gt_u32comRbytes = 0;
					atcmd_flag = FALSE; //for udp on trans

				}else if(at_type.trans_len == 0){
		#ifdef ATCMD_UART_RX_DOUB_BUF
//					.jk.ku.uiui
//					if(Buf_Ok == TRUE){
//						if(Full_Buf_Now == BUF_NO0) //如果 BUF1 空闲，将 DMA 接收数据赋值给 BUF1 
//						{ 
//							//printf("0 = %x\r\n",g_u32comRbytes);
//							uip_send(g_u8RecData, g_u32comRbytes);
//							memset(g_u8RecData, 0, g_u32comRbytes);
//							g_u32comRbytes = 0;
//							Full_Buf_Now = BUF_NO1;
//						}else{
//							//printf("1 = %x\r\n",g_u32comRbytes1);
//							uip_send(g_u8RecData1, g_u32comRbytes1);
//							memset(g_u8RecData1, 0, g_u32comRbytes1);
//							g_u32comRbytes1 = 0;
//							Full_Buf_Now = BUF_NO0;
//						}		
//						
//						if((g_u32comRbytes == 0) && (g_u32comRbytes1 == 0)){
//							atcmd_flag = FALSE; //for udp on trans
//							Buf_Ok = FALSE;
//						}
//					}
		#else
					check_udp_conn("to send2");

					printf("[UART->ETH] send(stream) len=%u -> %d.%d.%d.%d:%u\r\n",
						gt_u32comRbytes,
						uip_ipaddr1(uip_udp_conn->ripaddr), uip_ipaddr2(uip_udp_conn->ripaddr),
						uip_ipaddr3(uip_udp_conn->ripaddr), uip_ipaddr4(uip_udp_conn->ripaddr),
						HTONS(uip_udp_conn->rport));
					uip_send(g_u8RecData, gt_u32comRbytes);
					gt_comeDataUsart2++;
					printf("udp send %u/%u\n", gt_comeDataUsart2, count_udp_bridge_tx_conn()); //,check_udp_conn("send")
					/* want to send to every udp connection */
					
					printf("my.send.enum:check.upd_conn[.] [%d/%d]\r\n", //%d:%d:%d:%d rport %u lport %u 
						gt_comeDataUsart2, UIP_UDP_CONNS); 
									//uip_ipaddr1(uip_udp_conns[c].ripaddr), uip_ipaddr2(uip_udp_conns[c].ripaddr),
									//uip_ipaddr3(uip_udp_conns[c].ripaddr), uip_ipaddr4(uip_udp_conns[c].ripaddr), 
									//HTONS(uip_udp_conns[c].rport), HTONS(uip_udp_conns[c].lport),
				
					if (gt_comeDataUsart2 >= count_udp_bridge_tx_conn()) { //,check_udp_conn("my.send.enum")
						memset(g_u8RecData, 0, RecData_Size);
						gt_u32comRbytes = 0;
						gt_comeDataUsart2 = 0;
						udp_bridge_tx_index = 0;
						atcmd_flag = FALSE; //for udp on trans
					}
		#endif // ATCMD_UART_RX_DOUB_BUF
				}

			#if 1 //need arp_out?
			}
			#endif
	#if 1
		}
	#endif
	}
}

/**
 * \brief      Initialize the bridge server
 *
 *             This function initializes the bridge server and should be
 *             called at system boot-up.
 */
void bridge_init(void)
{
	/************ Jerry add ***********************/
	if(EthernetInitDoneFlag == 1){
		EthernetInitDoneFlag = 0;

#ifdef UIP_USE_BIGDATA_SIZE		
		if((at_type.role == 0) || (at_type.role == 1) || (at_type.role == 2))
		{
			AT_USART_DMA_RxToggle(2920); //UIP_CONF_BUFFER_SIZE     2974	//420
		}else{
			AT_USART_DMA_RxToggle(1460); //UIP_CONF_BUFFER_SIZE     1536	//420
		}
#endif //UIP_USE_BIGDATA_SIZE
		
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
				atcmd_tcpip_udpconn(ROLE_UDP_SCLIENT); //5
			break;			
			default:
			break;
		}
	}
}
/*---------------------------------------------------------------------------*/
