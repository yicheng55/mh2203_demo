#include "includes.h"
#include "uip.h"
#include "uip-split.h"
#include "stdio.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "DM9051.h"
#include "at_port.h"

extern uint8_t eth_netif_linkup;

struct at_funcation at_type;
struct eeprom_funcation eeprom_type;
struct udpbc_state mudpcs;

char at_tmpstr[512];
uint8_t atcmd_flag = FALSE;
char g_u8RecData[RecData_Size];
uint8_t tcp_connected = FALSE;
uint8_t udp_connected = UPDATE_UDP_NULL;
volatile unsigned int gt_u32comRbytes = 0;
uint8_t gt_comeDataUsart2 = 0;
uint8_t manualset_dns_srvip = 0;

const char *COMMAND_STR[] = {
	"ERR cmd",
};

#define ID_ERRCMD	0

void atcmd_restore(void)
{
		static struct at_funcation _at_type =
		{
			.dhcpc_mode = FALSE,
			.role = ROLE_TCP_SERVER,
			.hostip = {((DEF_HOSTIP1) | ((DEF_HOSTIP2) << 8)), ((DEF_HOSTIP3) | ((DEF_HOSTIP4) << 8))},
			.hostmask = {((DEF_HOSTNS1) | ((DEF_HOSTNS2) << 8)), ((DEF_HOSTNS3) | ((DEF_HOSTNS4) << 8))},
			.hostgw = {((DEF_HOSTGW1) | ((DEF_HOSTGW2) << 8)), ((DEF_HOSTGW3) | ((DEF_HOSTGW4) << 8))},
			.t_lport = DEF_PORTNUM,
			.tcp_raddr = {((DEF_HOSTIP1) | ((DEF_HOSTIP2) << 8)), ((DEF_HOSTIP3) | ((DEF_HOSTIP4) << 8))},
			.tcp_rport = DEF_PORTNUM,
			.u_lport = DEF_PORTNUM,
			.udp_raddr = {((DEF_HOSTIP1) | ((DEF_HOSTIP2) << 8)), ((DEF_HOSTIP3) | ((DEF_HOSTIP4) << 8))},
			.udp_rport = DEF_PORTNUM,
			.dns_mode = FALSE,
			.dns_saddr = {(0) | (0 << 8), (0) | (0 << 8)},
			.dns_srvname[0] = ' ',
			.dns_srvname[1] = 0,
			.dns_srvpath[0] = ' ',
			.dns_srvpath[1] = 0,
			.dns_srvport = 80,
			.keepalive_mode = FALSE,
			.baudrate = DEF_BAUDRATE,
			.wordlen = DEF_WORDLEN,
			.parity = DEF_PARITY,
			.stop = DEF_STOP,
			.trans_len = 0,
			.rst_count = 0
		};

		static struct eeprom_funcation _eeprom_type =
		{
			.macaddr = {emacETHADDR0, emacETHADDR1, emacETHADDR2, emacETHADDR3, emacETHADDR4, emacETHADDR5},
			.autoload = 0x1401,
			.vid = 0x0A46,
			.pid = 0x9051,
			.pincontrol = 0xFFFF,
			.wakeupcontrol2 = 0x0180,
			.control3 = 0,
		};

		at_type = _at_type;
		eeprom_type = _eeprom_type;
		Write_AT_DataFlash();

		uip_sethostaddr(at_type.hostip);
		uip_setnetmask(at_type.hostmask);
		uip_setdraddr(at_type.hostgw);
}

#ifdef HTTP_SERVER_SUPPORT
uint8_t eth_rcv_strtok_process(char *buf, char *tmp[])
{
		uint8_t n = 0, i;
		for(tmp[n] = strtok(buf, "&"); tmp[n] != NULL; tmp[n] = strtok(NULL, "&")) {
			n++;
		}
		return n;
}

extern struct at_funcation at_show;

static boolean param32_strtok_atoi(char *buf, char *param_name, uint32_t *pu32data)
{
	if(!strcasecmp(buf, param_name)) {
		buf = strtok(NULL, "=");
		*pu32data= atoi(buf);
		return TRUE;
	}
	return FALSE;
}

static char *elimite_head(char *buf)
{
	while (*buf== '+')
		buf++;
	return buf;
}

void eth_rcv_atcmd_to_at_show_process(uint8_t n, char *tmp[])
{
		uint8_t j;
		char *buf = "void";
		for(j = 0; j < n; j++)
		{
			uint32_t u32data;
			buf = strtok(tmp[j], "=");
			if (param32_strtok_atoi(buf, "tnmode", &u32data)) {
				at_show.role = u32data;
			}
			else if(param32_strtok_atoi(buf, "staticip", &u32data)){
				at_show.dhcpc_mode = (u32data == 1) ? 1 : 0;
			}else if(param32_strtok_atoi(buf, "sip1", &u32data)){
				at_show.hostip[0] = u32data;
			}else if(param32_strtok_atoi(buf, "sip2", &u32data)){
				at_show.hostip[0] |= (u32data << 8);
			}else if(param32_strtok_atoi(buf, "sip3", &u32data)){
				at_show.hostip[1] = u32data;
			}else if(param32_strtok_atoi(buf, "sip4", &u32data)){
				at_show.hostip[1] |= (u32data << 8);
			}
			else if(param32_strtok_atoi(buf, "mip1", &u32data)){
				at_show.hostmask[0] = u32data;
			}else if(param32_strtok_atoi(buf, "mip2", &u32data)){
				at_show.hostmask[0] |= (u32data << 8);
			}else if(param32_strtok_atoi(buf, "mip3", &u32data)){
				at_show.hostmask[1] = u32data;
			}else if(param32_strtok_atoi(buf, "mip4", &u32data)){
				at_show.hostmask[1] |= (u32data << 8);
			}
			else if(param32_strtok_atoi(buf, "gip1", &u32data)){
				at_show.hostgw[0] = u32data;
			}else if(param32_strtok_atoi(buf, "gip2", &u32data)){
				at_show.hostgw[0] |= (u32data << 8);
			}else if(param32_strtok_atoi(buf, "gip3", &u32data)){
				at_show.hostgw[1] = u32data;
			}else if(param32_strtok_atoi(buf, "gip4", &u32data)){
				at_show.hostgw[1] |= (u32data << 8);
			}else if(param32_strtok_atoi(buf, "tlp", &u32data)){
				if (u32data) {
					if(at_show.role == 0)
						at_show.t_lport = u32data;
					else if(at_show.role == 3)
						at_show.u_lport = u32data;
				}
			}
			else if(param32_strtok_atoi(buf, "trip1", &u32data)){
				if((at_show.role == 1) || (at_show.role == 2))
					at_show.tcp_raddr[0] = u32data;
				else if((at_show.role == 4) || (at_show.role == 5))
					at_show.udp_raddr[0] = u32data;
			}else if(param32_strtok_atoi(buf, "trip2", &u32data)){
				if((at_show.role == 1) || (at_show.role == 2))
					at_show.tcp_raddr[0] |= (u32data << 8);
				else if((at_show.role == 4) || (at_show.role == 5))
					at_show.udp_raddr[0] |= (u32data << 8);
			}else if(param32_strtok_atoi(buf, "trip3", &u32data)){
				if((at_show.role == 1) || (at_show.role == 2))
					at_show.tcp_raddr[1] = u32data;
				else if((at_show.role == 4) || (at_show.role == 5))
					at_show.udp_raddr[1] = u32data;
			}else if(param32_strtok_atoi(buf, "trip4", &u32data)){
				if((at_show.role == 1) || (at_show.role == 2))
					at_show.tcp_raddr[1] |= (u32data << 8);
				else if((at_show.role == 4) || (at_show.role == 5))
					at_show.udp_raddr[1] |= (u32data << 8);
			}
			else if(param32_strtok_atoi(buf, "trp", &u32data)){
				if((at_show.role == 1) || (at_show.role == 2))
					at_show.tcp_rport = u32data;
				else if((at_show.role == 4) || (at_show.role == 5))
					at_show.udp_rport = u32data;
			}else if(param32_strtok_atoi(buf, "ka", &u32data)){
				at_show.keepalive_mode = u32data;
			}else if(param32_strtok_atoi(buf, "bd", &u32data)){
				at_show.baudrate = u32data;
			}else if(param32_strtok_atoi(buf, "wln", &u32data)){
				at_show.wordlen = u32data;
			}else if(param32_strtok_atoi(buf, "pty", &u32data)){
				at_show.parity = u32data;
			}else if(param32_strtok_atoi(buf, "stop", &u32data)){
				at_show.stop = u32data;
			}
			else if(param32_strtok_atoi(buf, "dnsmode", &u32data)){
				at_show.dns_mode = u32data;
			} else if(param32_strtok_atoi(buf, "dip1", &u32data)){
				at_show.dns_saddr[0] = u32data;
			}else if(param32_strtok_atoi(buf, "dip2", &u32data)){
				at_show.dns_saddr[0] |= (u32data << 8);
			}else if(param32_strtok_atoi(buf, "dip3", &u32data)){
				at_show.dns_saddr[1] = u32data;
			}else if(param32_strtok_atoi(buf, "dip4", &u32data)){
				at_show.dns_saddr[1] |= (u32data << 8);
			}
			else if(!strcasecmp(buf, "srvname")){
				buf = strtok(NULL, "=");
				if (srvname_valid_ok(buf)) {
					sprintf(at_show.dns_srvname, "%s", elimite_head(buf));
				} else {
					at_show.dns_srvname[0] = ' ';
					at_show.dns_srvname[1] = 0;
				}
			}
			else if(!strcasecmp(buf, "restore")){
					atcmd_restore();
					if (atcmd_on_resp_note())
						atcmd_resp_ip();
			}else if(!strcasecmp(buf, "rst")){
					atcmd_rst();
			}else if(!strcasecmp(buf, "Disconnect")){
					if(on_trans_mode()){
						sprintf(uip_appdata, "+++");
						recv_cmd_auto_disconnect(UIP_PROTO_TCP);
					}
			}else if(!strcasecmp(buf, "Save")){
					if(on_trans_mode()){
						sprintf(uip_appdata, "+++");
						recv_cmd_auto_disconnect(UIP_PROTO_TCP);
					}
					_Write_AT_Show_DataFlash();
			}
		}
}
#endif

char *gt_tmpstr, *gt_tmpstrX;

char *gt_strtok(char *pattern) {
	char *newstr = strtok(NULL, pattern);
	if (!newstr)
		return NULL;
	return newstr;
}

void at_cmdProcess(void)
{
			char *tmpstr;
			if(on_trans_out_mode() && eth_netif_linkup)
			{
			}else{
				if (atcmd_flag)
				{
					display_str_state(1, g_u8RecData, "Input command");

					tmpstr = strtok(g_u8RecData, " \r\n");

					if(!strcasecmp(tmpstr,"RDY")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (!tmpstr)
								atcmd_ready();
							else
								goto CMD_ERR;
					}else if(!strcasecmp(tmpstr,"VERSION")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (!tmpstr) {
								atcmd_baseline();
								atcmd_version();
							} else
								goto CMD_ERR;
					}else if(!strcasecmp(tmpstr, "RESTORE")) {
							atcmd_restore();
							atcmd_ready();
					}else if(!strcasecmp(tmpstr, "RST")) {
							atcmd_rst();
					}else if(!strcasecmp(tmpstr, "SAVEP")) {
							atcmd_savep();
					}else if(!strcasecmp(tmpstr, "SHOW")) {
							atcmd_show_sys_msg(0);
							atcmd_show(0);
					}else if(!strcasecmp(tmpstr, "SHOWP")) {
							atcmd_show_sys_msg(1);
							atcmd_show(1);
					}else if(!strcasecmp(tmpstr, "BAUD")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (atoi_to_baud(tmpstr)) {
								atcmd_ready();
							} else
								goto CMD_ERR;
					}

					else if(!strcasecmp(tmpstr, "ROLE")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (!tmpstr)
								atcmd_resp_tcpip_role(0);
							else if (atcmd_tcpip_role(tmpstr[0] - 0x30) && tmpstr[1] == 0)
								atcmd_role_command(tmpstr[0] - 0x30);
							else
								goto CMD_ERR;
					}else if(!strcasecmp(tmpstr, "DHCPC")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (strcasecmp_dhcpc_ok(tmpstr))
								atcmd_resp_dhcpc_mode(at_type.dhcpc_mode);
							else
								goto CMD_ERR;
					}else if(!strcasecmp(tmpstr, "MAC")) {
							tmpstr = trans_strtok(tmpstr, ":");
							if (strtol_mac_ok(tmpstr))
								atcmd_tcpip_mac();
							else
								goto CMD_ERR;
					}else if(!strcasecmp(tmpstr, "IP")) {
							tmpstr = trans_strtok(tmpstr, ".");
							atcmd_tcpip_ip(tmpstr);
					}else if(!strcasecmp(tmpstr, "GW")) {
							tmpstr = trans_strtok(tmpstr, ".");
							atcmd_tcpip_gw(tmpstr);
					}	else if(!strcasecmp(tmpstr, "MASK")) {
							tmpstr = trans_strtok(tmpstr, ".");
							atcmd_tcpip_mask(tmpstr);
					}else if(!strcasecmp(tmpstr, "TCPLPORT")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (tmpstr) {
								at_type.t_lport = dm_uip_port(tmpstr);
								if(at_type.role != ROLE_TCP_SERVER){
									atcmd_tcpip_role_err7(ROLE_TCP_SERVER);
								}else{
									atcmd_tcpip_tcplport();
								}
							} else {
								atcmd_resp_listen("tcp", at_type.t_lport);
							}
					}else if(!strcasecmp(tmpstr, "TCPDCONN")) {
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) {
								atoi_connect_ip(tmpstr, &at_type.tcp_raddr, &at_type.tcp_rport);
								if(at_type.role != ROLE_TCP_DCLIENT){
									atcmd_tcpip_role_err7(ROLE_TCP_DCLIENT);
								}else{
									atcmd_tcpip_tcpconn();
								}
							} else {
								sprintf(at_tmpstr, "TCP Dynamic CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.tcp_raddr), uip_ipaddr2(at_type.tcp_raddr),
													uip_ipaddr3(at_type.tcp_raddr), uip_ipaddr4(at_type.tcp_raddr), at_type.tcp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "TCPSCONN")) {
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) {
								atoi_connect_ip(tmpstr, &at_type.tcp_raddr, &at_type.tcp_rport);
								if(at_type.role != ROLE_TCP_SCLIENT){
									atcmd_tcpip_role_err7(ROLE_TCP_SCLIENT);
								}else{
									atcmd_savep();
								}
							} else {
								sprintf(at_tmpstr, "TCP Static CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.tcp_raddr), uip_ipaddr2(at_type.tcp_raddr),
													uip_ipaddr3(at_type.tcp_raddr), uip_ipaddr4(at_type.tcp_raddr), at_type.tcp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "UDPLPORT")) {
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (tmpstr) {
								at_type.u_lport = dm_uip_port(tmpstr);
								atcmd_tcpip_udplport();
							} else {
								atcmd_resp_listen("udp", at_type.u_lport);
							}
					}else if(!strcasecmp(tmpstr, "UDPDCONN")) {
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) {
								atoi_connect_ip(tmpstr, &at_type.udp_raddr, &at_type.udp_rport);
								atcmd_tcpip_udpconn(ROLE_UDP_DCLIENT);
							} else {
								sprintf(at_tmpstr, "UDP Dynamic CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.udp_raddr), uip_ipaddr2(at_type.udp_raddr),
													uip_ipaddr3(at_type.udp_raddr), uip_ipaddr4(at_type.udp_raddr), at_type.udp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "UDPSCONN")) {
						if(at_type.role == ROLE_UDP_SCLIENT){
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr)
								atoi_connect_ip(tmpstr, &at_type.udp_raddr, &at_type.udp_rport);
							sprintf(at_tmpstr, "UDP Static CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.udp_raddr), uip_ipaddr2(at_type.udp_raddr),
												uip_ipaddr3(at_type.udp_raddr), uip_ipaddr4(at_type.udp_raddr), at_type.udp_rport);
							atcmd_resp_cmd(at_tmpstr);
							if (tmpstr)
								atcmd_savep();
						} else
							atcmd_tcpip_role_err7(ROLE_UDP_SCLIENT);
					}
					else if(!strcasecmp(tmpstr, "DNSC")){
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (tmpstr) {
								if(!strcasecmp(tmpstr, "ON")) {
									at_type.dns_mode = 1;
									atcmd_dnsc_mode(1);
								}else if(!strcasecmp(tmpstr, "OFF")) {
									at_type.dns_mode = 0;
									atcmd_dnsc_mode(0);
									manualset_dns_srvip = 0;
									at_type.dns_saddr[0] = 0 + (0 << 8);
									at_type.dns_saddr[1] = 0 + (0 << 8);
									at_type.dns_srvport = 0;
								}
							} else {
								atcmd_dnsc_mode(at_type.dns_mode);
							}
					}
					else if(!strcasecmp(tmpstr, "SRVIP")){
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) {
								at_type.dns_saddr[0] = atoi(tmpstr);
								tmpstr = strtok(NULL, ".");
								at_type.dns_saddr[0] |= (atoi(tmpstr) << 8);
								tmpstr = strtok(NULL, ".");
								at_type.dns_saddr[1] = atoi(tmpstr);
								tmpstr = strtok(NULL, " \r\n");
								at_type.dns_saddr[1] |= (atoi(tmpstr) << 8);
								manualset_dns_srvip = 1;
								atcmd_dns_srvip();
							} else {
								sprintf(at_tmpstr, "DNS Server IP= %d.%d.%d.%d", uip_ipaddr1(at_type.dns_saddr), uip_ipaddr2(at_type.dns_saddr),
													uip_ipaddr3(at_type.dns_saddr), uip_ipaddr4(at_type.dns_saddr));
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "SRVNAME")){
							gt_tmpstr = trans_strtok(NULL, ": \r\n");
							if (gt_tmpstr) {
								memset(&at_type.dns_srvname[0], 0, 32);
								strcpy(&at_type.dns_srvname[0], gt_tmpstr);
								gt_tmpstrX = gt_strtok(" \r\n");
								if (gt_tmpstrX) {
									at_type.dns_srvport = atoi(gt_tmpstrX);
								}
							}
							atcmd_resp_srvname(&at_type, 1);
					}
					else if(!strcasecmp(tmpstr, "KEEPALIVE")){
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (tmpstr) {
								if(!strcasecmp(tmpstr, "ON")) {
									if((at_type.role == ROLE_TCP_SERVER) || (at_type.role == ROLE_TCP_DCLIENT)
										|| (at_type.role == ROLE_TCP_SCLIENT)){
										atcmd_tcpip_keepalive(1);
									}else{
										atcmd_resp_cmd("Keepalive only support TCP...");
										atcmd_tcpip_keepalive(0);
									}
								}else if(!strcasecmp(tmpstr, "OFF")) {
									atcmd_tcpip_keepalive(0);
								}
							} else {
								atcmd_tcpip_keepalive(at_type.keepalive_mode);
							}
					}
					else{
					CMD_ERR:
						atsmd_resp_command_err();
					}

					memset(g_u8RecData, 0, sizeof(g_u8RecData));
					gt_u32comRbytes = 0;
					atcmd_flag = FALSE;
					return;
				}
		}
}
