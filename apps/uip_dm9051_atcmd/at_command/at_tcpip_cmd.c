/* Includes ------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uip.h"
#include "uip_arp.h"
#include "uiplib.h"
#include "app_call.h"

#include "at_port.h"
#include "atcommand.h"
#include "etherbridge.h"

extern uint8_t eth_netif_linkup; /* app_call.c 提供 (取代 AT32 uip_init.c) */

void tcpip_periodic_timer_watch_func(void);
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
uint8_t tcpip_periodic_timer_watch = 0;
uint8_t UDPSrvConnFlag = 0;

void u_writeuart1(char uartchar)
{
	atp_uart_putc(uartchar);
}

void atcmd_baseline(void)
{
		u_writeuart1('\r');
		u_writeuart1('\n');
}

/**
 * atcmd_resp_cmd(char *uartTXstr) : at uart response message.
 * AT command: kaconfig 10 5 5
 */
void atcmd_resp_cmd(const char *uartTXstr)
{
		uint8_t txlen = strlen(uartTXstr);
		uint8_t i;
		
		u_writeuart1('+');
		for(i=0; i<txlen; i++) {
			u_writeuart1(uartTXstr[i]);	
		}
		u_writeuart1('\r');
		u_writeuart1('\n');
}

//[Will created/Wll created]
boolean on_trans_mode(void)
{
	return (boolean)(tcp_connected || (udp_connected & UPDATE_UDP_CONNECTED));
}

boolean on_trans_out_mode(void)
{
	return (boolean)(tcp_connected || ((udp_connected & UPDATE_UDP_SEND) == UPDATE_UDP_SEND));
}

boolean atcmd_on_resp_note(void)
{
	if(on_trans_out_mode())
		return FALSE;
	atcmd_resp_note(); //"[Note]"
	return TRUE;
}

/**
 * atcmd_tcpip_keepalive_config() : Keepalive config function set timer/interval/probes.
 * AT command: kaconfig 10 5 5
 */
#if 0
void atcmd_tcpip_keepalive_config(void)
{
		_sprintf(at_tmpstr, "Keepalive config= %d %d %d", at_type.keepalive_no_data_period, 
												at_type.keepalive_send_period, at_type.keepalive_send_count);
		atcmd_resp_cmd(at_tmpstr);
}
#endif

/**
 * atcmd_tcpip_keepalive() : Keepalive function Enable/Disable.
 * AT command: keepaliv ON/OFF
 */
void atcmd_tcpip_keepalive(uint8_t sw)
{
		at_type.keepalive_mode = sw;
		sprintf(at_tmpstr, "Keepalive Mode= %d", sw);
		atcmd_resp_cmd(at_tmpstr);
}

/**
 * atcmd_dns_srvpath() : DNS server path.
 * AT command: srvpath /~
 */
//void atcmd_dns_srvpath(void)
//{
//	sprintf(at_tmpstr, "DNS Server Path = %s", at_type.dns_srvpath);
//	atcmd_resp_cmd(at_tmpstr);
//}

/**
 * atcmd_dns_srvname() : DNS server name.
 * AT command: srvname google.com
 */
//void atcmd_dns_srvname(void)
//{
//	atcmd_resp_srvname(at_type.dns_srvname);
//}

/**
 * atcmd_tcpip_dns_serip() : Set DNS Server IP.
 * AT command: srvip 8.8.8.8
 */
void atcmd_dns_srvip(void)
{
	sprintf(at_tmpstr, "DNS Server IP= %d.%d.%d.%d",uip_ipaddr1(at_type.dns_saddr), uip_ipaddr2(at_type.dns_saddr), 
																				uip_ipaddr3(at_type.dns_saddr), uip_ipaddr4(at_type.dns_saddr));
	atcmd_resp_cmd(at_tmpstr);
}

/**
 * atcmd_dnsc_mode() : DNS Client function Enable/Disable.
 * AT command: DNSC ON/OFF
 */
void atcmd_dnsc_mode(uint8_t sw)
{
		sprintf(at_tmpstr, "DNS Client mode= %d", sw);
		atcmd_resp_cmd(at_tmpstr);
}
//void dnsc_mode(uint8_t sw)
//{
//		at_type.dns_mode = sw;
//		atcmd_dnsc_mode(sw);
//}
uint8_t atcmd_show_dns_srvname(struct at_funcation *at_xxx)
{
		if (at_xxx->dns_mode && at_xxx->dns_srvname_found) {
			sprintf(at_tmpstr, "DNS Found Name %s:%d", dhs_staus_msg, at_xxx->dns_srvport);
			atcmd_resp_cmd(at_tmpstr);
			return 1;
		}
		return 0;
}

/**
 * atcmd_tcpip_role_err() : Role setup error.
 * Total command:
 */
void atcmd_resp_tcpip_role(uint8_t full)
{
	sprintf(at_tmpstr, "Role= %u", at_type.role); //role_desc_string(at_type.role)

	//sprintf(at_tmpstr, "Role = %u, %s", at_type.role, make_role_msg()); //role_desc_string(at_type.role)
	if (full) {
		strcat(at_tmpstr, ", (");
		strcat(at_tmpstr, role_desc_string(at_type.role)); //make_role_msg()
		strcat(at_tmpstr, ")");
	}
	atcmd_resp_cmd(at_tmpstr);
}

/**
 * atcmd_tcpip_udpconn(): UDP client setup remote server IP:Port.
 * AT command: UDPCONN 192.168.31.22:9000
 */
void atcmd_tcpip_udpconn(uint8_t on_role)
{
		uip_ipaddr_t ipaddr;
		struct uip_udp_conn *conn;
		
		#if 1 //[use dnsc's. srvname's]
		if (at_type.dns_mode && at_type.dns_srvname_found) {
			memcpy(at_type.udp_raddr, at_type.srvname_saddr, sizeof(uip_ipaddr_t ) );
			at_type.udp_rport = at_type.dns_srvport;
		}
		#endif
		
		uip_ipaddr(ipaddr, at_type.udp_raddr[0], at_type.udp_raddr[0] >> 8, at_type.udp_raddr[1], at_type.udp_raddr[1] >> 8);
		conn = uip_udp_new(&ipaddr, HTONS(at_type.udp_rport));
		if (conn == NULL)
			return; //no mes offer, keep clean
			
		uip_udp_bind(conn, HTONS(at_type.u_lport));
		atcmd_show_dns_srvname(&at_type);
		//+UDP Mode UDPSCONN :Remote ip= 192.168.6.50  :port= 8080
		//+UDP Mode UDPSCONN :Local ip=  192.168.6.100 :port= 8080
		//+UDP Mode UDPSCONN :mask=      255.255.255.0
		//+UDP Mode UDPSCONN :gw=        192.168.1.1
	#ifdef UPDATE_UDP
		atcmd_resp_remote("UDP", on_role, at_type.udp_raddr, at_type.udp_rport);
		atcmd_resp_configured("UDP", on_role, at_type.u_lport);
		atcmd_resp_role_start(on_role);
		tcpip_periodic_timer_watch = 1;
		if (eth_netif_linkup)
			tcpip_periodic_timer_watch_func();
		//else
			//give it tcpip_periodic_timer_watch keep as 1, wait link-up happen.
//		_atcmd_flag = //FALSE; //TRUE //because it's dynamic running state
	#endif
}

void tcpip_periodic_timer_watch_func(void) {
	if (tcpip_periodic_timer_watch) {
		tcpip_periodic_timer_watch = 0;
		
		if (eth_netif_linkup) {
			atcmd_resp_trans_ready(3);
			print_trans_ready();
			udp_connected |= UPDATE_UDP_RDY;
			/* UDP client has a configured remote endpoint, so it can send as soon as link is ready. */
			if ((at_type.role == ROLE_UDP_DCLIENT) || (at_type.role == ROLE_UDP_SCLIENT))
				udp_connected |= UPDATE_UDP_CONNECTED;
		} 
//		else {
//			printf("Link down state\r\n");
//			atcmd_resp_cmd("Link down state");
//		}
	}
}

/**
 * atcmd_tcpip_udplport(): UDP server setup listen Port.
 * AT command: UDPLPORT 8000
 */
void atcmd_tcpip_udplport(void)
{
		uip_ipaddr_t ipaddr;
		struct uip_udp_conn *conn;

		uip_ipaddr(ipaddr, 255, 255, 255, 255);
		conn = uip_udp_new(&ipaddr, 0);
		if (conn == NULL)
			return; //no mes offer, keep clean

		uip_udp_bind(conn, HTONS(at_type.u_lport));
		check_udp_conn("UDP server, Listen start");
	#if 1
		uip_gethostaddr(ipaddr);
		atcmd_comp_listen_port("UDP", ipaddr, at_type.u_lport);
		atcmd_resp_cmd(at_tmpstr);
		printf("%s\r\n", at_tmpstr);
		
		atcmd_resp_atcmd_ready();
		atcmd_resp_trans_ready(9);
		udp_connected = UPDATE_UDP_RDY;

		UDPSrvConnFlag = 1;
//		check_DM9051_link = 1;
	#endif
}

/**
 * atcmd_tcpip_tcpconn(): Dynamic TCP client setup remote server IP:Port.
 * AT command: TCPDCONN 192.168.31.22:9000
 */
void atcmd_tcpip_tcpconn(void)
{
		struct uip_conn *conn;
		uip_ipaddr_t ipaddr;
	
		tcp_init();
		
		#if 1 //[use dnsc's. srvname's]
		if (at_type.dns_mode && at_type.dns_srvname_found) {
			memcpy(at_type.tcp_raddr, at_type.srvname_saddr, sizeof(uip_ipaddr_t ) );
			at_type.tcp_rport = at_type.dns_srvport;
		}
		#endif
				
		uip_ipaddr(&ipaddr, at_type.tcp_raddr[0], (at_type.tcp_raddr[0] >> 8), 
							at_type.tcp_raddr[1], (at_type.tcp_raddr[1] >> 8));
		conn = uip_connect(&ipaddr, HTONS(at_type.tcp_rport));
		
		if(conn != NULL) {
			atcmd_show_dns_srvname(&at_type);
//			check_DM9051_link = 1;

			#ifdef UPDATE_TCP
			atcmd_resp_remote("TCP", at_type.role, at_type.tcp_raddr, at_type.tcp_rport);
			atcmd_resp_configured("TCP", at_type.role, at_type.t_lport);
			#endif
			atcmd_resp_role_start(at_type.role);
		}
}

/**
 * atcmd_tcpip_tcplport(): TCP server setup listen Port.
 * AT command: TCPLPORT 8000
 */
void atcmd_tcpip_tcplport(void)
{
		uip_ipaddr_t ipaddr;
		tcp_init();
		uip_listen(HTONS(at_type.t_lport));
		
		uip_gethostaddr(ipaddr);
		atcmd_comp_listen_port("TCP", ipaddr, at_type.t_lport);
		atcmd_resp_cmd(at_tmpstr);
		printf("%s\r\n", at_tmpstr);

		atcmd_resp_atcmd_ready();
		atcmd_resp_trans_wait(13);
}

void atcmd_resp_mask(void)
{
	sprintf(at_tmpstr, "mask = %d.%d.%d.%d", uip_ipaddr1(at_type.hostmask), uip_ipaddr2(at_type.hostmask),
																							uip_ipaddr3(at_type.hostmask), uip_ipaddr4(at_type.hostmask));
	atcmd_resp_cmd(at_tmpstr);
}
void atcmd_resp_gw(void)
{
	sprintf(at_tmpstr, "gw = %d.%d.%d.%d", uip_ipaddr1(at_type.hostgw), uip_ipaddr2(at_type.hostgw),
																					uip_ipaddr3(at_type.hostgw), uip_ipaddr4(at_type.hostgw));
	atcmd_resp_cmd(at_tmpstr);
}
void atcmd_resp_ip(void)
{
	sprintf(at_tmpstr, "ip = %d.%d.%d.%d", uip_ipaddr1(at_type.hostip), uip_ipaddr2(at_type.hostip), 
			uip_ipaddr3(at_type.hostip), uip_ipaddr4(at_type.hostip));
	atcmd_resp_cmd(at_tmpstr);
}

/**
 * atcmd_tcpip_mask() : Setup fixed network mask.
 * AT command: MASK 255.255.255.0
 */
void atcmd_tcpip_mask(char *tmpstr)
{
	if (tmpstr) {
		at_type.hostmask[0] = atoi(tmpstr);
		tmpstr = strtok(NULL, ".");
		at_type.hostmask[0] |= (atoi(tmpstr) << 8);
		tmpstr = strtok(NULL, ".");
		at_type.hostmask[1] = atoi(tmpstr);
		tmpstr = strtok(NULL, " \r\n");
		at_type.hostmask[1] |= (atoi(tmpstr) << 8);
	}
	
	uip_setnetmask(at_type.hostmask);
	atcmd_resp_mask();
}

/**
 * atcmd_tcpip_gw() : Setup fixed gateway address.
 * AT command: GW 192.168.31.1
 */
void atcmd_tcpip_gw(char *tmpstr)
{
	if (tmpstr) {
		at_type.hostgw[0] = atoi(tmpstr);
		tmpstr = strtok(NULL, ".");
		at_type.hostgw[0] |= (atoi(tmpstr) << 8);
		tmpstr = strtok(NULL, ".");
		at_type.hostgw[1] = atoi(tmpstr);
		tmpstr = strtok(NULL, " \r\n");
		at_type.hostgw[1] |= (atoi(tmpstr) << 8);
	}
	
	uip_setdraddr(at_type.hostgw);
	atcmd_resp_gw();
}

/**
 * atcmd_tcpip_ip() : Setup fixed IP address.
 * AT command: IP 192.168.31.101
 */
void atcmd_tcpip_ip(char *tmpstr)
{
	if (tmpstr) { // && (tmpstr[0] != 0x0a)
		at_type.hostip[0] = atoi(tmpstr);
		tmpstr = strtok(NULL, ".");
		at_type.hostip[0] |= (atoi(tmpstr) << 8);
		tmpstr = strtok(NULL, ".");
		at_type.hostip[1] = atoi(tmpstr);
		tmpstr = strtok(NULL, " \r\n");
		at_type.hostip[1] |= (atoi(tmpstr) << 8);
	}
	
	uip_sethostaddr(at_type.hostip);
	atcmd_resp_ip();
}

/**
 * atcmd_tcpip_mac(): Setup MAC address.
 * AT command: MAC 00:60:6E:90:51:06
 */
void atcmd_tcpip_mac(void)
{
	sprintf(at_tmpstr, "MAC Address = %02X:%02X:%02X:%02X:%02X:%02X", eeprom_type.macaddr[0], eeprom_type.macaddr[1],
						eeprom_type.macaddr[2], eeprom_type.macaddr[3], eeprom_type.macaddr[4], eeprom_type.macaddr[5]);
  atcmd_resp_cmd(at_tmpstr);
}

/**
 * Display Baudrate Setup value
 * NOT AT command: Only subroutine
 */
//void atcmd_baud_msg(void)
//{
//		sprintf(at_tmpstr, "AT UART Baud Rate= %d %d %d %d", at_type.baudrate, at_type.wordlen ,at_type.parity, at_type.stop);
//		atcmd_resp_cmd(at_tmpstr);
//}

/**
 * atcmd_tcpip_dhcpc() : DHCP Client Enable/Disable.
 * AT command: DHCPC ON/OFF
 */
void atcmd_resp_dhcpc_mode(uint8_t sw)
{
		sprintf(at_tmpstr, "DHCP Mode= %d", sw);
		atcmd_resp_cmd(at_tmpstr);
}
//void atcmd_tcpip_dhcpc(uint8_t sw)
//{
//		at_type.dhcpc_mode = sw;
//		atcmd_resp_dhcpc_mode(sw);
//}

/**
 * atcmd_tcpip_role() : Print out role mode.
 * AT command: ROLE 0/1/2/3/4/5
 */
boolean atcmd_tcpip_role(uint8_t role)
{
	if (/*role >= 0 &&*/ role <= 5) {
		return TRUE;
	}
	return FALSE;
//		switch(at_type.role) {
//			// TCP Server & Client Mode
//			case ROLE_TCP_SERVER:
//				atcmd_resp_cmd("Role = 0, TCP_Server"); 
//				break;
//			case ROLE_TCP_DCLIENT:
//				atcmd_resp_cmd("Role = 1, TCP_Dynamic_Client");
//				break;
//			case ROLE_TCP_SCLIENT:
//				atcmd_resp_cmd("Role = 2, TCP_Sataic_Client");
//			break;
//			
//			// UDP Server & Client Mode
//			case ROLE_UDP_SERVER:
//				atcmd_resp_cmd("Role = 3, UDP_Server");
//				break;
//			case ROLE_UDP_DCLIENT:
//				atcmd_resp_cmd("Role = 4, UDP_Dynamic_Client");
//			break;
//			case ROLE_UDP_SCLIENT:
//				atcmd_resp_cmd("Role = 5, UDP_Static_Client");
//			break;
//			default:
//			break;
//		}
}
