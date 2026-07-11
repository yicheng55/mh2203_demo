#include "string.h"
#include "includes.h"
#include "uip_init.h"
#include "tapdev.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "uip_keepalive.h"

#define BUFH  ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define BUFL  ((struct ethip_hdr *)&uip_buf[0])
	
struct keepalive_function_type ka_type;

#if 0
/**
 * keepalive_update_send_seq(). This function uip_process response ACK copy sequence & ack number to tmp for keepalive use
 */
void keepalive_update_send_seq(void)
{
		if((BUFL->ethhdr.src.addr[0] == 0) && (BUFL->ethhdr.src.addr[1] == 0) && (BUFL->ethhdr.src.addr[2] == 0) && 
						(BUFL->ethhdr.src.addr[3] == 0) && (BUFL->ethhdr.src.addr[4] == 0) && (BUFL->ethhdr.src.addr[5] == 0)){
		}else{
			//if receive packet source MAC Address != 0, copy last packet source MAC address
			memcpy(ka_type.ethaddr.addr, BUFL->ethhdr.src.addr, 6);
		}
		
		if(((BUFH->ackno[0] == 0) && (BUFH->ackno[1] == 0) && (BUFH->ackno[2] == 0) && (BUFH->ackno[3] == 0))
			|| ((BUFH->seqno[0] == 0) && (BUFH->seqno[1] == 0) && (BUFH->seqno[2] == 0) && (BUFH->seqno[3] == 0)))
		{
		}else{
			//if receive packet source sequence number && ack number != 0, copy last packet sequence number & ack number
			memcpy(ka_type.ka_ackno_tmp, BUFH->seqno, sizeof(BUFH->seqno));
			memcpy(ka_type.ka_seqno_tmp, BUFH->ackno, sizeof(BUFH->ackno));
		}
}

/** 
 * keepalive_update_recv_seq(). This function active send date copy sequence & ack number to tmp for keepalive use
 */
void keepalive_update_recv_seq(void)
{
		if((BUFL->ethhdr.src.addr[0] == 0) && (BUFL->ethhdr.src.addr[1] == 0) && (BUFL->ethhdr.src.addr[2] == 0) && 
						(BUFL->ethhdr.src.addr[3] == 0) && (BUFL->ethhdr.src.addr[4] == 0) && (BUFL->ethhdr.src.addr[5] == 0)){
		}else{
			//if receive packet source MAC Address != 0, copy last packet source MAC address
			memcpy(ka_type.ethaddr.addr, BUFL->ethhdr.src.addr, 6);
		}
		
		if(((BUFH->ackno[0] == 0) && (BUFH->ackno[1] == 0) && (BUFH->ackno[2] == 0) && (BUFH->ackno[3] == 0))
			|| ((BUFH->seqno[0] == 0) && (BUFH->seqno[1] == 0) && (BUFH->seqno[2] == 0) && (BUFH->seqno[3] == 0)))
		{
		}else{
			//if receive packet source sequence number && ack number != 0, copy last packet sequence number & ack number
			memcpy(ka_type.ka_seqno_tmp, BUFH->seqno, sizeof(BUFH->seqno));
			memcpy(ka_type.ka_ackno_tmp, BUFH->ackno, sizeof(BUFH->ackno));
		}
}
#endif

/** 
 * keepalive_function(). This function Add TCP/IP header, prepare send keepalive packet
 */
//void keepalive_trans_function(void)
//{
//			uint8_t c;
//	
//			if(ka_type.ka_tick_flag == 1){
//				ka_type.ka_no_data_tick++;
//			
//				if(ka_type.ka_no_data_tick == at_type.keepalive_no_data_period) {
//					for(c = 0; c < UIP_CONNS; ++c) {
//						if((uip_conns[c].lport == HTONS(at_type.t_lport)) || (uip_conn[c].rport == HTONS(at_type.tcp_rport))){
//							//uip_periodic(c);
//							uip_poll_conn(c);   //Stone add for keepalive sequence error
//						}
//					}

//					//keepalive_trans_function();
//					ka_type.ka_noack_tick = 0;	
//					//keepalive_update_recv_seq();
//					uip_send_keepalive();
//							
//					/*memcpy(BUFL->ethhdr.dest.addr, ka_type.ethaddr.addr, 6);
//					memcpy(BUFL->ethhdr.src.addr, uip_ethaddr.addr, 6);
//					uip_buf[12] = HTONS(0x0800);
//					uip_len += 14;*/

//					uip_arp_out();
//					tapdev_send();
//					printf("1. tapdev_send\r\n");
//					ka_type.ka_send_count++;
//					ka_type.ka_noack_tick = 0;
//					
//				}else if(ka_type.ka_no_data_tick > at_type.keepalive_no_data_period){
//					ka_type.ka_noack_tick++;
//					
//					if(ka_type.ka_noack_tick == at_type.keepalive_send_period){
//						for(c = 0; c < UIP_CONNS; ++c) {
//							if((uip_conns[c].lport == HTONS(at_type.t_lport)) || (uip_conn[c].rport == HTONS(at_type.tcp_rport))){
//								//uip_periodic(c);
//								uip_poll_conn(c);   //Stone add for keepalive sequence error
//							}
//						}
//						//keepalive_trans_function();
//						ka_type.ka_noack_tick = 0;
//	
//						//keepalive_update_recv_seq();
//				
//						uip_send_keepalive();
//									
//						uip_arp_out();
//						tapdev_send();
//						printf("2. tapdev_send uip_len %x \r\n", uip_len);
//						ka_type.ka_send_count++;
//					}
//				}
//			}
//}

void keepalive_init(void)
{
#if 0
		// Init keepalive type
		if(ka_type.ka_send_count >= at_type.keepalive_send_count){ 
			//printf("ka_send_count\r\n");
		}else{
			//printf("1 ka_type.ka_tick_flag = %d\r\n",ka_type.ka_tick_flag );
			ka_type.ka_tick_flag = 0;
			//printf("2 ka_type.ka_tick_flag = %d\r\n",ka_type.ka_tick_flag );
		}
#endif
		ka_type.ka_no_data_tick = 1;
#if 0
		ka_type.ka_noack_tick = 1;
		ka_type.ka_send_count = 1;
#endif		

		//at_type.keepalive_no_data_period = 15;
		at_type.keepalive_no_data_period = 100;
#if 0
		at_type.keepalive_send_period = 3;
		at_type.keepalive_send_count = 3;
#endif
}
