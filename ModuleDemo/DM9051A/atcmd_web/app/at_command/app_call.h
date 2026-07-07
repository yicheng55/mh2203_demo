
#ifndef __APP_CALL__
#define __APP_CALL__

#include "stdint.h"

#define CFG_TCP_APP_ENABLE         //Enable TCP Application, if no any TCP App disable it
#define CFG_UDP_APP_ENABLE         //Eable UDP Application, if no any UDP App disable it

#ifdef CFG_TCP_APP_ENABLE
#define HTTP_SERVER_SUPPORT
#endif //CFG_TCP_APP_ENABLE

/*--------------------- TCP APP ---------------------------------------*/
#ifdef CFG_TCP_APP_ENABLE

#ifdef HTTP_SERVER_SUPPORT
#include "httpd.h"
#endif //HTTP_SERVER_SUPPORT

	typedef union
	{
#ifdef HTTP_SERVER_SUPPORT
		struct httpd_state httpd_app;
#endif //HTTP_SERVER_SUPPORT
	}Str_TCP_App_State;

	typedef Str_TCP_App_State uip_tcp_appstate_t;
#else
	typedef int uip_tcp_appstate_t; 
#endif //CFG_TCP_APP_ENABLE
	
void tcp_appcall(void);
#ifndef UIP_APPCALL
#define UIP_APPCALL tcp_appcall
#endif //UIP_APPCALL

/*--------------------- UDP APP ---------------------------------------*/	
#ifdef CFG_UDP_APP_ENABLE
	
//.#define DHCPC_EN        1

#include "dhcpc.h"
#include "resolv.h"	

	typedef union
	{
		struct dhcpc_state dhcpc_app;
	}Str_UDP_App_State;
	
	typedef Str_UDP_App_State uip_udp_appstate_t;
#else
	typedef int uip_udp_appstate_t; 
#endif //CFG_UDP_APP_ENABLE.
	
void udp_appcall(void);
#ifndef UIP_UDP_APPCALL
#define UIP_UDP_APPCALL udp_appcall
#endif //UIP_UDP_APPCALL

	
extern uint8_t EthernetInitDoneFlag;
extern char dhs_staus_msg[64];
	
void dns_client_init(void);
void EthernetInit_Done(void);
#endif /* __APP_CALL__ */
