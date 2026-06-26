//#include "includes.h"
#include <stdio.h>
#include "uip.h"

#if 0
	//#include "app_call.h"
	#include "tcp_app.h"
	#include "udp_app.h"
#endif

void tcp_appcall(void)
{
	/* Local Port */     ////测试服务端数据的收发
	switch(uip_conn->lport)
	{
		#if WEB_EN
		case HTONS(80):
			//printf("[Listen 80]\r\n");
			//debug_appcall("[Listen 80]");
			httpd_appcall();
			break;
		#endif
		case HTONS(5100):
			debug_appcall("[Listen 5100]");
#if 0
			tcp_client_server_appcall();
#endif
			break;
		case HTONS(5001):
			debug_appcall("[Listen 5001]");
			break;	
		default:
			break;
	}
	/* Remote Port */ ////测试客户端数据的收发测试
	switch(uip_conn->rport)
	{
		case HTONS(5002):
			debug_appcall("[TCP.Client rport 5002]");
#if 0
			tcp_client_server_appcall();
#endif
			debug_appcall_after("[TCP.to rport 5002]");
			break;
		default:
			break;
	}
}

#if UIP_UDP
void udp_appcall(void)
{	
	/* UDP Remote Port	*/ // 67 68 用于DHCP
 switch (uip_udp_conn->rport){
	#if DHCPC_EN
	/* need uncommented to use and/or need debug.
	 */
	//switch (uip_udp_conn->rport){}
    case HTONS(67): //Transmite UDP listen port
		dhcpc_appcall();
		break;
	#endif
	
	#if 0
	case HTONS(1500):
		udp_send_appcall();
		break;
	#endif
	
	case HTONS(53): //Transmite UDP listen port
       resolv_appcall();
		break;
	default:
		break;
  }
	/* UDP Local Port	*/
	switch (uip_udp_conn->lport){
		case HTONS(1600): //Received UDP listen port
			/*
			 * can not here!
			dhcpc_appcall();
			 */
			printf("udp_recv listenport %d\r\n", 1600);
#if 0
		    udp_recv_appcall();
#endif
			break;

		case HTONS(1800):
			break;
		default:
			break;
	}
}
#endif
