#include "includes.h"
#include "uip.h"
#include "app_call.h"
#include "uip_init.h"
//#include "System_Init.h"
#include "stdio.h"
#include "at32f4xx.h"
#include "DM9051.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "httpd.h"

uint8_t EthernetInitDoneFlag = 0;
char dhs_staus_msg[64];

void tcp_appcall(void)
{
	if((uip_conn->lport == HTONS(at_type.t_lport)) || (uip_conn->rport == HTONS(at_type.tcp_rport))){
		tcp_bridge_appcall();
	}
	
	/* Local Port */
	switch(uip_conn->lport)
	{
#ifdef HTTP_SERVER_SUPPORT
		case HTONS(80):
				httpd_appcall();
			break;
#endif //HTTP_SERVER_SUPPORT

//
//		This is a varible value.
//		case HTONS(at_type.t_lport):
//			break;
//

		default:
			break;
	}
	/* Remote Port */
	/*switch(uip_conn->rport)
	{
		default:
			break;
	}*/
}

void udp_appcall(void)
{	
	if((uip_udp_conn->lport == HTONS(at_type.u_lport)) || (uip_udp_conn->rport == HTONS(at_type.udp_rport))){
		udp_bridge_appcall();
	}
	
	/* UDP Remote Port	*/
  switch (uip_udp_conn->rport){
		 case HTONS(53): //Transmite UDP listen port
       resolv_appcall();
     break;
		default:
			break;
  }
	/* UDP Local Port	*/
	switch (uip_udp_conn->lport){
#if DHCPC_EN
		 case HTONS(67): //Transmite UDP listen port
        dhcpc_appcall();  //kj.ui.uiuiu...
      break;
		case HTONS(68): //Received UDP listen port
				dhcpc_appcall();
			break;
#endif //DHCPC_EN
		default:
			break;
	}
}

void
uip_log(char *m)
{
  printf("uIP log: %s\n", m);
}

void dns_client_init(void)
{
	uip_ipaddr_t ipaddr;

	resolv_init();
	
	uip_ipaddr(ipaddr, at_type.dns_saddr[0], (at_type.dns_saddr[0] >> 8), at_type.dns_saddr[1], (at_type.dns_saddr[1] >> 8));
	
	resolv_conf(ipaddr);
	resolv_query(at_type.dns_srvname);
}

void EthernetInit_Done(void)
{
//	#if 1
//	display_udp_conn("before tcp/udp restart all");
//	#endif
//	printf("TCP/UDP Restart All\r\n");

//	printf(" role %u\r\n", at_type.role);

	tcp_init();
	udp_init();
	
#if 1
//	printf("at_type.dns_mode = %u, at_type.role = %u\r\n", at_type.dns_mode, at_type.role);
	
//	if((at_type.dns_mode == 1) && ((at_type.role == ROLE_UDP_DCLIENT) || 
//			(at_type.role == ROLE_UDP_SCLIENT))){
//		printf("dns_client_init() udp\r\n");
//		dns_client_init();
//	} else
#endif
	if((at_type.dns_mode == 1) && ((at_type.role == ROLE_TCP_DCLIENT) || 
			(at_type.role == ROLE_TCP_SCLIENT) ||
			(at_type.role == ROLE_UDP_DCLIENT) || 
			(at_type.role == ROLE_UDP_SCLIENT))){
//		printf("dns_clnt_init() tcp\r\n");
//		printf("DNSC Start...\r\n");
		atcmd_resp_dnsc();
		print_resp_dnsc();
		dns_client_init();
	}else{
		EthernetInitDoneFlag = 1;
	}
}

void
resolv_found(char *name, uint16_t *ipaddr)
{
  uint8_t n;
  if(ipaddr == NULL) {
		sprintf(dhs_staus_msg,"Host '%s' not found.", name); //"\r\n"
//		strcpy(at_tmpstr, dhs_staus_msg);
		
//		sprintf(at_tmpstr,"Host '%s' not found.", name); //"\r\n"
//		strcpy(dhs_staus_msg, at_tmpstr);
		
		_printf("%s\r\n", dhs_staus_msg);
		atcmd_resp_cmd(dhs_staus_msg);
	#if 1
		/* This is very important, giveup to start a bridge_init(), Joseph 
		 */
		EthernetInitDoneFlag = 1;
	#endif
  } else {
	#if 0 //Jos, Don't auto change to 'at_type.tcp_raddr','at_type.tcp_rport'. Avoid certain un-need case.
		//We recommanded manual change by user.
		// udp_raddr[]/or tcp_raddr[].
		at_type.tcp_raddr[0] = ((htons(ipaddr[0]) & 0xff) << 8) + (htons(ipaddr[0]) >> 8);
		at_type.tcp_raddr[1] = ((htons(ipaddr[1]) & 0xff) << 8) + (htons(ipaddr[1]) >> 8);
		at_type.tcp_rport = at_type._dns_srvport;
	#endif

		sprintf(dhs_staus_msg,"'%s' = %d.%d.%d.%d", name, //" : %d\r\n"
			htons(ipaddr[0]) >> 8,
			htons(ipaddr[0]) & 0xff,
			htons(ipaddr[1]) >> 8,
			htons(ipaddr[1]) & 0xff); //(at_type._dns_srvport);

		at_type.dns_srvname_found = 1;
//		at_type.srvname_saddr[0] = ipaddr[0] & 0xff;
//		at_type.srvname_saddr[0] |= ipaddr[0] & 0xff00;
		at_type.srvname_saddr[0] = ipaddr[0];
		at_type.srvname_saddr[1] = ipaddr[1];
			
		printf("Found name %s\r\n", dhs_staus_msg);

		EthernetInitDoneFlag = 1;
    /*    webclient_get("www.sics.se", 80, "/~adam/uip");   */
  }
  
  //delete
  n = check_udp_conn("dns.end");
  delete_udp_r_conn(53);
  printf("dns.finish, now %d udp.conn\r\n", check_udp_conn("dns.x"));
  printf("\r\n");
}

#ifdef WEB_CLIENT
void
smtp_done(unsigned char code)
{
  printf("SMTP done with code %d\n", code);
}
void
webclient_closed(void)
{
  printf("Webclient: connection closed\n");
}
void
webclient_aborted(void)
{
  printf("Webclient: connection aborted\n");
}
void
webclient_timedout(void)
{
  printf("Webclient: connection timed out\n");
}
void
webclient_connected(void)
{
  printf("Webclient: connected, waiting for data...\n");
}
void
webclient_datahandler(char *data, uint16_t len)
{
  printf("Webclient: got %d bytes of data.\n", len);
}
#endif //WEB_CLIENT
