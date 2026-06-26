
#ifndef __APP_CALL__
#define __APP_CALL__

void DataPrintf(uint8_t *Buff, uint32_t BuffSize);
int SER_PUTC(int ch);

#define TCP_APP_EN      1 //Enable TCP Application, if no any TCP App disable it

#define UDP_APP_EN        1 //Eable UDP Application, if no any UDP App disable it

/*--------------------- TCP APP ---------------------------------------*/
#if TCP_APP_EN
	
 #define WEB_EN				1	//WEBSERVER
 
/* 
 #define WEB_SCRIPT_CODE	1 //1 (0 temp)
 #define WEB_SCRIPT_EXAMPLE	1	//"sys_stats()", used when a page with .shtml, search 'http_shtml'
 #define WEB_FS_TABPAGES	0	//For output (0 to reduce fs size)
 
 #define TEST_404HTML_CFG	0 	// 1 (404-Page, to test it),
 #define HOMEPAGE_SEL_CFG	3	// 0 (ARTARY Page), 1 (MH-Page), 2 (Demo2-Page), 3 (Demo3-Page).
 
#if (TEST_404HTML_CFG == 1) //[Test]
#define HOME_HTML	"/404.html"
#elif (HOMEPAGE_SEL_CFG == 0)
#define HOME_HTML	http_at32led_html
#elif (HOMEPAGE_SEL_CFG == 1)
#define HOME_HTML	data_index_html
#elif (HOMEPAGE_SEL_CFG == 2)
#define HOME_HTML	data_index_shtml
#elif (HOMEPAGE_SEL_CFG == 3)
#define HOME_HTML	data_webMain_shtml
#endif
*/

#include "httpd-home-webserver.h" //connect to #include "httpd.h"
//#include "tcp_app.h"
//#include "http-strings.h"
 
 //#define HOME_HTML		http_index_html
 //#define HOME_HTML		"/index.html"

/*
	//typedef struct httpd_state uip_tcp_appstate_t;
	//typedef int uip_tcp_appstate_t; 
	typedef union
	{
		//struct tcp_state tcp_app;
		//struct httpd_state tcp_app;
		struct httpd_state HTTP_APP;
	}Str_TCP_App_State;

	typedef Str_TCP_App_State uip_tcp_appstate_t;
	*/
	
	typedef struct httpd_state HTTP_Type;
	
	typedef union
	{
		//struct tcp_state tcp_app;
		int tcp_app;
		//struct httpd_state HTTP_APP;
		HTTP_Type HTTP_APP;
	} Str_TCP_App_State;

	/*struct Str_TCP_App_State union
	{
		//struct tcp_state tcp_app;
		struct httpd_state HTTP_APP;
	};*/
	
	typedef Str_TCP_App_State uip_tcp_appstate_t;
	//typedef struct httpd_state uip_tcp_appstate_t;
#else //TCP_APP_EN
	#include "httpd-home-webserver.h" //connect to #include "httpd.h" (added when not 'TCP_APP_EN' 1, jj.)
	typedef struct httpd_state uip_tcp_appstate_t; //typedef int uip_tcp_appstate_t; 
#endif //TCP_APP_EN

/*--------------------- UIP APPCALL ---------------------------------------*/
#ifdef UIP_APPCALL
#undef UIP_APPCALL
#endif //UIP_APPCALL

#define UIP_APPCALL tcp_appcall
  //...............................................tuktkt...20230616

/*--------------------- UDP APP ---------------------------------------*/	
#if UDP_APP_EN
	
 #define DHCPC_EN        0

 #include "dhcpc.h"
 #include "resolv.h"	

//typedef struct dhcpc_state uip_udp_appstate_t;
//typedef int uip_udp_appstate_t;
 typedef union
 {
	int uip_udp_appstate_t;
	struct dhcpc_state dhcpc_app;
 } Str_UDP_App_State;
	
 typedef Str_UDP_App_State uip_udp_appstate_t;
#else //UDP_APP_EN
 typedef int uip_udp_appstate_t; 
#endif //UDP_APP_EN

/*--------------------- UIP UDP APPCALL ---------------------------------------*/	
#ifdef UIP_UDP_APPCALL
#undef UIP_UDP_APPCALL
#endif //UIP_UDP_APPCALL

#define UIP_UDP_APPCALL udp_appcall

void tcp_appcall(void);
void udp_appcall(void);
void debug_appcall(char *dbgHead);
void debug_appcall_after(char *dbgHead);

#endif /* __APP_CALL__ */
