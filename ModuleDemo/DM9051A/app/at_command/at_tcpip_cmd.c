#include "includes.h"
#include "uip.h"
#include "uip_arp.h"
#include "uiplib.h"
#include "tapdev.h"
#include "uip_init.h"
#include "app_call.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "DM9051.h"
#include "at_port.h"

extern uint8_t eth_netif_linkup;

void tcpip_periodic_timer_watch_func(void);
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

boolean on_trans_mode(void)
{
	return (boolean)(tcp_connected || (udp_connected & UPDATE_UDP_CONNECTED));
}

boolean on_trans_out_mode(void)
{
	return (boolean)(tcp_connected || (udp_connected & UPDATE_UDP_SEND));
}

boolean atcmd_on_resp_note(void)
{
	if(on_trans_out_mode())
		return FALSE;
	atcmd_resp_note();
	return TRUE;
}

void atcmd_tcpip_keepalive(uint8_t sw)
{
		at_type.keepalive_mode = sw;
		sprintf(at_tmpstr, "Keepalive Mode= %d", sw);
		atcmd_resp_cmd(at_tmpstr);
}

void atcmd_dns_srvip(void)
{
	sprintf(at_tmpstr, "DNS Server IP= %d.%d.%d.%d",uip_ipaddr1(at_type.dns_saddr), uip_ipaddr2(at_type.dns_saddr),
																				uip_ipaddr3(at_type.dns_saddr), uip_ipaddr4(at_type.dns_saddr));
	atcmd_resp_cmd(at_tmpstr);
}

void atcmd_dnsc_mode(uint8_t sw)
{
		sprintf(at_tmpstr, "DNS Client mode= %d", sw);
		atcmd_resp_cmd(at_tmpstr);
}

uint8_t atcmd_show_dns_srvname(struct at_funcation *at_xxx)
{
		if (at_xxx->dns_mode && at_xxx->dns_srvname_found) {
			sprintf(at_tmpstr, "DNS Found Name %s:%d", dhs_staus_msg, at_xxx->dns_srvport);
			atcmd_resp_cmd(at_tmpstr);
			return 1;
		}
		return 0;
}

void atcmd_resp_tcpip_role(uint8_t full)
{
	sprintf(at_tmpstr, "Role= %u", at_type.role);
	if (full) {
		strcat(at_tmpstr, ", (");
		strcat(at_tmpstr, role_desc_string(at_type.role));
		strcat(at_tmpstr, ")");
	}
	atcmd_resp_cmd(at_tmpstr);
}

void atcmd_tcpip_udpconn(uint8_t on_role)
{
		uip_ipaddr_t ipaddr;
		struct uip_udp_conn *conn;

		if (at_type.dns_mode && at_type.dns_srvname_found) {
			memcpy(at_type.udp_raddr, at_type.srvname_saddr, sizeof(uip_ipaddr_t ) );
			at_type.udp_rport = at_type.dns_srvport;
		}

		uip_ipaddr(ipaddr, at_type.udp_raddr[0], at_type.udp_raddr[0] >> 8, at_type.udp_raddr[1], at_type.udp_raddr[1] >> 8);
		conn = uip_udp_new(&ipaddr, HTONS(at_type.udp_rport));
		if (conn == NULL)
			return;

		uip_udp_bind(conn, HTONS(at_type.u_lport));
		atcmd_show_dns_srvname(&at_type);

		atcmd_resp_remote("UDP", on_role, at_type.udp_raddr, at_type.udp_rport);
		atcmd_resp_configured("UDP", on_role, at_type.u_lport);
		atcmd_resp_role_start(on_role);
		tcpip_periodic_timer_watch = 1;
		if (eth_netif_linkup)
			tcpip_periodic_timer_watch_func();
}

void tcpip_periodic_timer_watch_func(void) {
	if (tcpip_periodic_timer_watch) {
		tcpip_periodic_timer_watch = 0;
		if (eth_netif_linkup) {
			atcmd_resp_trans_ready(3);
			print_trans_ready();
			udp_connected |= UPDATE_UDP_RDY;
		}
	}
}

void atcmd_tcpip_udplport(void)
{
		uip_ipaddr_t ipaddr;
		struct uip_udp_conn *conn;

		uip_ipaddr(ipaddr, 255, 255, 255, 255);
		conn = uip_udp_new(&ipaddr, 0);
		if (conn == NULL)
			return;

		uip_udp_bind(conn, HTONS(at_type.u_lport));
		check_udp_conn("UDP server, Listen start");
		uip_gethostaddr(ipaddr);
		atcmd_comp_listen_port("UDP", ipaddr, at_type.u_lport);
		atcmd_resp_cmd(at_tmpstr);
		printf("%s\r\n", at_tmpstr);

		atcmd_resp_atcmd_ready();
		atcmd_resp_trans_ready(9);

		UDPSrvConnFlag = 1;
}

void atcmd_tcpip_tcpconn(void)
{
		struct uip_conn *conn;
		uip_ipaddr_t ipaddr;

		tcp_init();

		if (at_type.dns_mode && at_type.dns_srvname_found) {
			memcpy(at_type.tcp_raddr, at_type.srvname_saddr, sizeof(uip_ipaddr_t ) );
			at_type.tcp_rport = at_type.dns_srvport;
		}

		uip_ipaddr(&ipaddr, at_type.tcp_raddr[0], (at_type.tcp_raddr[0] >> 8),
							at_type.tcp_raddr[1], (at_type.tcp_raddr[1] >> 8));
		conn = uip_connect(&ipaddr, HTONS(at_type.tcp_rport));

		if(conn != NULL) {
			atcmd_show_dns_srvname(&at_type);
			atcmd_resp_remote("TCP", at_type.role, at_type.tcp_raddr, at_type.tcp_rport);
			atcmd_resp_configured("TCP", at_type.role, at_type.t_lport);
			atcmd_resp_role_start(at_type.role);
		}
}

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

void atcmd_tcpip_ip(char *tmpstr)
{
	if (tmpstr) {
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

void atcmd_tcpip_mac(void)
{
	sprintf(at_tmpstr, "MAC Address = %02X:%02X:%02X:%02X:%02X:%02X", eeprom_type.macaddr[0], eeprom_type.macaddr[1],
						eeprom_type.macaddr[2], eeprom_type.macaddr[3], eeprom_type.macaddr[4], eeprom_type.macaddr[5]);
  atcmd_resp_cmd(at_tmpstr);
}

void atcmd_resp_dhcpc_mode(uint8_t sw)
{
		sprintf(at_tmpstr, "DHCP Mode= %d", sw);
		atcmd_resp_cmd(at_tmpstr);
}

boolean atcmd_tcpip_role(uint8_t role)
{
	if (role <= 5) {
		return TRUE;
	}
	return FALSE;
}
