#include "stdio.h"
#include "includes.h"
#include "atcommand.h"
#include "DM9051.h"
#include "at_port.h"

#define FMC_IAP_PROGARM_ADDR  0x08000000
typedef void (*iapfun)(void);
typedef void (FUNC_PTR)(void);
iapfun jump_defapp;

#ifdef CUS_TEST
uint8_t showp_read_flag = 0;
#endif

struct at_tempshow
{
	uip_ipaddr_t tmpraddr;
	uint16_t tmprport;
};
static struct at_tempshow temp_show;

struct at_funcation at_show;
struct eeprom_funcation eeprom_show;

void atcmd_resp_connect_timeout(uip_ipaddr_t *ipaddr, uint16_t rport)
{
	 sprintf(at_tmpstr, "Bridge connection %d.%d.%d.%d port %u timeout...",
		uip_ipaddr1(ipaddr),uip_ipaddr2(ipaddr),uip_ipaddr3(ipaddr),uip_ipaddr4(ipaddr),
		rport);
	 atcmd_resp_cmd(at_tmpstr);
	 printf("%s\r\n", at_tmpstr);
}

void atcmd_resp_srvname(struct at_funcation *at_xxx, uint8_t sp)
{
	char tmpbuf[8] = { 0 };
	if (sp)
		sprintf(tmpbuf, ":%d", at_xxx->dns_srvport);
	sprintf(at_tmpstr, "SrvName= %s%s", at_xxx->dns_srvname, tmpbuf);
	atcmd_resp_cmd(at_tmpstr);
}

uint8_t param_baud_ok(void) {
	if (at_show.baudrate != 4800 &&
		at_show.baudrate != 9600 &&
		at_show.baudrate != 14400 &&
		at_show.baudrate != 19200 &&
		at_show.baudrate != 38400 &&
		at_show.baudrate != 57600 &&
		at_show.baudrate != 115200 &&
		at_show.baudrate != 230400 &&
		at_show.baudrate != 460800 &&
		at_show.baudrate != 921600
		)
			return 0;
	if (
		at_show.wordlen != 8
		)
			return 0;
	if (at_show.parity != 0 &&
		at_show.parity != 1 &&
		at_show.parity != 2
		)
			return 0;
	if (at_show.stop != 1 &&
		at_show.stop != 2
		)
			return 0;
	return 1;
}

void atcmd_resp_baud_mode(void) {
		sprintf(at_tmpstr, "Baud Rate= %d %d %d ", at_show.baudrate, at_show.wordlen ,at_show.parity);
		switch(at_show.stop){
		case 1:
			strcat(at_tmpstr, "1bit");
		break;
		case 2:
			strcat(at_tmpstr, "2bit");
		break;
		}
		atcmd_resp_cmd(at_tmpstr);
}

void atcmd_role_command(uint8_t role)
{
	at_type.role = role;
	atcmd_resp_tcpip_role(1);
}

uint8_t strcasecmp_dhcpc_ok(char *tmpstr)
{
	if (!tmpstr)
		return 1;
	if(!strcasecmp(tmpstr, "ON")) {
		at_type.dhcpc_mode = 1;
		return 1;
	}else if(!strcasecmp(tmpstr, "OFF")) {
		at_type.dhcpc_mode = 0;
		return 1;
	}
	return 0;
}

uint8_t atoi_to_baud(char *tmpstr)
{
	if (!tmpstr)
		return 1;
	if (isdigit(*tmpstr)) {
		at_show.baudrate = atoi(tmpstr);;
		tmpstr = strtok(NULL, " \r\n");
		if (isdigit(*tmpstr)) {
			at_show.wordlen = atoi(tmpstr);
			tmpstr = strtok(NULL, " \r\n");
			if (isdigit(*tmpstr)) {
				at_show.parity = atoi(tmpstr);
				tmpstr = strtok(NULL, " \r\n");
				if (isdigit(*tmpstr)) {
					at_show.stop = atoi(tmpstr);
					if (param_baud_ok()) {
						at_type.baudrate= at_show.baudrate;
						at_type.wordlen= at_show.wordlen;
						at_type.parity= at_show.parity;
						at_type.stop= at_show.stop;
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

uint8_t strtol_mac_ok(char *tmpstr)
{
	if (!tmpstr)
		return 1;
	if (trans_mac_ok(tmpstr)) {
		eeprom_show.macaddr[0] = strtol(tmpstr, NULL, 16);
		tmpstr = strtok(NULL, ":");
		if (trans_mac_ok(tmpstr)) {
			eeprom_show.macaddr[1] = strtol(tmpstr, NULL, 16);
			tmpstr = strtok(NULL, ":");
			eeprom_show.macaddr[2] = strtol(tmpstr, NULL, 16);
			tmpstr = strtok(NULL, ":");
			eeprom_show.macaddr[3] = strtol(tmpstr, NULL, 16);
			tmpstr = strtok(NULL, ":");
			eeprom_show.macaddr[4] = strtol(tmpstr, NULL, 16);
			tmpstr = strtok(NULL, " \r\n");
			eeprom_show.macaddr[5] = strtol(tmpstr, NULL, 16);
			memcpy(eeprom_type.macaddr, eeprom_show.macaddr, sizeof(eeprom_show.macaddr));
			return 1;
		}
	}
	return 0;
}

void atoi_connect_ip(char *tmpstr, uip_ipaddr_t *raddr, uint16_t *rport)
{
	temp_show.tmpraddr[0] = atoi(tmpstr);
	tmpstr = strtok(NULL, ".");
	temp_show.tmpraddr[0] |= (atoi(tmpstr) << 8);
	tmpstr = strtok(NULL, ".");
	temp_show.tmpraddr[1] = atoi(tmpstr);
	tmpstr = strtok(NULL, ":");
	temp_show.tmpraddr[1] |= (atoi(tmpstr) << 8);
	uip_ipaddr_copy(raddr, temp_show.tmpraddr);
	tmpstr = strtok(NULL, " \r\n");
	*rport = dm_uip_port(tmpstr);
}

void atsmd_resp_command_err(void)
{
	atcmd_resp_cmd("AT command error...!!!");
}

void atcmd_show_sys_msg(uint8_t state)
{
		atcmd_baseline();
		Copy_AT_Type_to_Show();
		if(state == 1){
#ifdef CUS_TEST
			showp_read_flag = 1;
#endif
			atcmd_resp_cmd("--- Display flash parameters ---+");
			Read_AT_Show_DataFlash(0);
		}
}

void atcmd_set_baud_msg(void) {
		at_show.baudrate= at_type.baudrate;
		at_show.wordlen= at_type.wordlen;
		at_show.parity= at_type.parity;
		at_show.stop= at_type.stop;
}

void atcmd_show(uint8_t state) {
			sprintf(at_tmpstr, "ROLE %d", at_show.role);
			atcmd_resp_cmd(at_tmpstr);
			atcmd_resp_dhcpc_mode(at_show.dhcpc_mode);
			sprintf(at_tmpstr, "Keepalive Mode= %d", at_show.keepalive_mode);
			atcmd_resp_cmd(at_tmpstr);
			atcmd_dnsc_mode(at_show.dns_mode);
			sprintf(at_tmpstr, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", eeprom_show.macaddr[0], eeprom_show.macaddr[1],
								eeprom_show.macaddr[2], eeprom_show.macaddr[3], eeprom_show.macaddr[4], eeprom_show.macaddr[5]);
			atcmd_resp_cmd(at_tmpstr);
			sprintf(at_tmpstr, "ip= %d.%d.%d.%d", uip_ipaddr1(at_show.hostip), uip_ipaddr2(at_show.hostip), uip_ipaddr3(at_show.hostip), uip_ipaddr4(at_show.hostip));
			atcmd_resp_cmd(at_tmpstr);
			sprintf(at_tmpstr, "mask= %d.%d.%d.%d", uip_ipaddr1(at_show.hostmask), uip_ipaddr2(at_show.hostmask), uip_ipaddr3(at_show.hostmask), uip_ipaddr4(at_show.hostmask));
			atcmd_resp_cmd(at_tmpstr);
			sprintf(at_tmpstr, "gw= %d.%d.%d.%d", uip_ipaddr1(at_show.hostgw), uip_ipaddr2(at_show.hostgw), uip_ipaddr3(at_show.hostgw), uip_ipaddr4(at_show.hostgw));
			atcmd_resp_cmd(at_tmpstr);
			atcmd_resp_listen("tcp", at_show.t_lport);
			atcmd_resp_listen("udp", at_type.u_lport);
			sprintf(at_tmpstr, "TCPSCONN= %d.%d.%d.%d:%d",
								uip_ipaddr1(at_show.tcp_raddr), uip_ipaddr2(at_show.tcp_raddr),
								uip_ipaddr3(at_show.tcp_raddr), uip_ipaddr4(at_show.tcp_raddr), at_show.tcp_rport);
			atcmd_resp_cmd(at_tmpstr);
			sprintf(at_tmpstr, "UDPSCONN= %d.%d.%d.%d:%d",
								uip_ipaddr1(at_show.udp_raddr), uip_ipaddr2(at_show.udp_raddr),
								uip_ipaddr3(at_show.udp_raddr), uip_ipaddr4(at_show.udp_raddr), at_show.udp_rport);
			atcmd_resp_cmd(at_tmpstr);
			sprintf(at_tmpstr, "DNS Server IP= %d.%d.%d.%d", uip_ipaddr1(at_show.dns_saddr), uip_ipaddr2(at_show.dns_saddr),
								uip_ipaddr3(at_show.dns_saddr), uip_ipaddr4(at_show.dns_saddr));
			atcmd_resp_cmd(at_tmpstr);
			if (state == 0) {
				if (!atcmd_show_dns_srvname(&at_show))
					atcmd_resp_srvname(&at_show, 1);
			} else {
				atcmd_resp_srvname(&at_show, 1);
			}
			atcmd_resp_baud_mode();
}

void atcmd_savep(void)
{
	if ((at_type.role == ROLE_TCP_DCLIENT) || (at_type.role == ROLE_UDP_DCLIENT)) {
		atcmd_tcpip_role_err8(at_type.role);
		return;
	}
		Write_AT_DataFlash();
		if((at_type.role == ROLE_TCP_SCLIENT) || (at_type.role == ROLE_UDP_SCLIENT)){
			atcmd_resp_rst();
		}else{
			atcmd_ready();
		}
}

void atcmd_rst(void)
{
	atp_system_reset();
}

void atcmd_version(void)
{
		atcmd_resp_cmd(IPSPP_VERSION);
}

void atcmd_ready(void)
{
	atcmd_resp_cmd("OK");
}
