#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "at_port.h"
#include "atcommand.h"

#ifdef CUS_TEST
uint8_t showp_read_flag = 0;
#endif //CUS_TEST

#if 0
//char show_role[AT_ROLESTR_LEN];
#endif
#if 0
// AT Show Rank 
//struct at_showrank
//{
//	/* Switch DHCP Client Mode ON/OFF*/
//	uint8_t dhcpc_mode;
//	/* TCP and UDP Applicaiton Mode */
//	uint8_t role;
//	/* Keep alive switch */
//	uint8_t keepalive_mode;
//	/* DNS Server IP */
//	uint8_t dns_mode;
//	/* Setup Transparent lenght */
//	uint16_t trans_len;
//	uint8_t  macaddr[6];
//	/* Host IP, netmask, Getwau */
//	uip_ipaddr_t hostip;
//	uip_ipaddr_t hostmask;
//	uip_ipaddr_t hostgw;
//	/* TCP Server Listen Port */
//	uint16_t t_lport;
//	/* TCP Client Connect Remote Address & Port */
//	uip_ipaddr_t tcp_raddr;
//	uint16_t tcp_rport;
//	/* UDP Listen Port */
//	uint16_t u_lport;
//	/* UDP Client Remote Address & Remote Port */
//	uip_ipaddr_t udp_raddr;
//	uint16_t udp_rport;
//	/* DNS Server IP */
//	uip_ipaddr_t dns_saddr;    //DNS Server IP
//	char dns_srvname[32]; //DNS Server Name
//	uint16_t _dns_srvport;        //manual setup _server port
//	/* Setup Baudrate */
//	uint32_t baudrate;
//	uint8_t wordlen;
//	uint8_t parity;
//	uint8_t stop;
//	uint16_t autoload;
//	uint16_t pincontrol;
//	uint16_t wakeupcontrol2;
//	uint8_t  control3;
//	uint16_t vid;
//	uint16_t pid;
//	// Temp 
//	uip_ipaddr_t tmpraddr;
//	uint16_t tmprport;
//};
//static struct at_showrank at_show;
#else
struct at_tempshow
{
	/* Temp */
	uip_ipaddr_t tmpraddr;
	uint16_t tmprport;
};
static struct at_tempshow temp_show;

struct at_funcation at_show;
struct eeprom_funcation eeprom_show;
#endif

void atcmd_resp_connect_timeout(uip_ipaddr_t *ipaddr, uint16_t rport)
{
	 sprintf(at_tmpstr, "Bridge connection %d.%d.%d.%d port %u timeout...",
		uip_ipaddr1(ipaddr),uip_ipaddr2(ipaddr),uip_ipaddr3(ipaddr),uip_ipaddr4(ipaddr),
		rport);
	 atcmd_resp_cmd(at_tmpstr);

	 printf("%s\r\n", at_tmpstr);
//	 sprintf(at_tmpstr, "Remote conn port %u", rport);
//	 atcmd_resp_cmd(at_tmpstr);
}

void atcmd_resp_srvname(struct at_funcation *at_xxx, uint8_t sp) //(char *dn)
{
	char tmpbuf[8] = { 0 };
	if (sp)
		sprintf(tmpbuf, ":%d", at_xxx->dns_srvport);
	sprintf(at_tmpstr, "SrvName= %s%s", at_xxx->dns_srvname, tmpbuf); //"DNS SrvName" //" : %d", at_type._dns_srvport
	atcmd_resp_cmd(at_tmpstr);
}

/**
 * atcmd_translen(): Show parameter values from flash.
 * AT command: TRANSLEN 1
 */
#if 0 //[temp debug]
void atcmd_translen(void)
{
	sprintf(at_tmpstr, "Transparent Lenght = %d", at_type.trans_len);
	atcmd_resp_cmd(at_tmpstr);
}
#endif

/**
 * atcmd_baud(): Set AT UART baudrate & wordlen & stop & parity.
 * AT command: BAUD 19200 8 0 1
 *	STOP:
 *		0 = UART_STOP_BIT_1 (default)
 *		1 = UART_STOP_BIT_1_5
 *		2 = UART_STOP_BIT_2
 *	PARITY:
 *		0 = UART_PARITY_NONE	(default)
 *		1 = UART_PARITY_ODD
 *		2 = UART_PARITY_EVEN
 *		3 = UART_PARITY_MARK
 *		4 = UART_PARITY_SPACE
 *	WORDLEN:
 *		0 = UART_WORD_LEN_5
 *		1 = UART_WORD_LEN_6
 *		2 = UART_WORD_LEN_7
 *		3 = UART_WORD_LEN_8 (default)
 */
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
#if 0
//		at_show.wordlen != 5 &&
//		at_show.wordlen != 6 &&
//		at_show.wordlen != 7 &&
		at_show.wordlen != 7 &&
		//at_show.wordlen != 9 &&
#endif
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
		//&& at_show.stop != 3 
		)
			return 0;
#if 0
	if (at_show.wordlen == 7 && at_show.parity == 0) {
		return 0;
	}
#endif
	return 1;
}

void atcmd_resp_baud_mode(void) {
		sprintf(at_tmpstr, "Baud Rate= %d %d %d ", at_show.baudrate, at_show.wordlen ,at_show.parity); //AT UART 
		switch(at_show.stop){	//at_show.stop
//			case 1:
//				strcat(at_tmpstr, "1bit(1)"); //mstop = UART_STOP_BIT_1;
//			break;
//			case 2:
//				strcat(at_tmpstr, "1.5bit(2)"); //mstop = UART_STOP_BIT_1_5; //1.5 bit
//			break;
//			case 3:
//				strcat(at_tmpstr, "2bit(3)"); //mstop = UART_STOP_BIT_2; //2 bit
//			break;
			case 1:
				strcat(at_tmpstr, "1bit"); //mstop = UART_STOP_BIT_1;
			break;
			case 2:
				strcat(at_tmpstr, "2bit"); //mstop = UART_STOP_BIT_1_5; //1.5 bit
			break;
		}
		atcmd_resp_cmd(at_tmpstr);
}

#if 0
void atcmd_baud(void)
{		
#if 0
//		switch(at_type.wordlen) {
//			case 5:
//				mwordlen = UART_WORD_LEN_5;
//				break;
//			case 6:
//				mwordlen = UART_WORD_LEN_6;
//				break;
//			case 7:
//				mwordlen = UART_WORD_LEN_7;
//				break;
//			case 8:
//				mwordlen = UART_WORD_LEN_8;
//				break;
//		}

		switch(at_type.parity) {
			case 0:
				mparity = UART_PARITY_NONE;
				break;
			case 1:
				mparity = UART_PARITY_ODD;
				break;
			case 2:
				mparity = UART_PARITY_EVEN;
				break;
			case 3:
				mparity = UART_PARITY_MARK;
				break;
			case 4:
				mparity = UART_PARITY_SPACE;
				break;
		}

		switch(at_type.stop){
			case 1:
				mstop = UART_STOP_BIT_1;
			break;
			case 2:
				mstop = UART_STOP_BIT_1_5;
			break;
			case 3:
				mstop = UART_STOP_BIT_2;
			break;
		}
#endif //0
		//Let user explicity manual operate "savep" is better!
		//atcmd_savep();
}
#endif

#if 0
//char *make_role_msg(void)
//{
//	sprintf(show_role, "(%s)", role_desc_string(at_type.role));
//	return show_role;
//}
#endif

void atcmd_role_command(uint8_t role)
{	
	at_type.role = role;
	
	atcmd_resp_tcpip_role(1); 
}

/**
 * atcmd_show_sys_msg(): Display System setup message
 * state:
 *  			0 : Display current system setup value
 *				1 : Display write to DataFlash parameter value
 * AT command: SHOW or SHOWP
 */
uint8_t strcasecmp_dhcpc_ok(char *tmpstr)
{
	if (!tmpstr)
		return 1; // only "baud"
	if(!strcasecmp(tmpstr, "ON")) {
		at_type.dhcpc_mode = 1; //atcmd_tcpip_dhcpc(1);
		return 1;
	}else if(!strcasecmp(tmpstr, "OFF")) {
		at_type.dhcpc_mode = 0; //atcmd_tcpip_dhcpc(0);
		return 1;
	}
	return 0;
}

uint8_t atoi_to_baud(char *tmpstr)
{
	if (!tmpstr)
		return 1; // only "baud"
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
					/* Setup Baudrate */
//					at_show.baudrate= at_type.baudrate;
//					at_show.wordlen= at_type.wordlen;
//					at_show.parity= at_type.parity;
//					at_show.stop= at_type.stop;
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
		return 1; // only "mac"
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
//	*raddr[0] = atoi(tmpstr);
	tmpstr = strtok(NULL, ".");
	temp_show.tmpraddr[0] |= (atoi(tmpstr) << 8);
//	*raddr[0] |= atoi(tmpstr);
	tmpstr = strtok(NULL, ".");
	temp_show.tmpraddr[1] = atoi(tmpstr);
//	*raddr[1] = atoi(tmpstr);
	tmpstr = strtok(NULL, ":");
	temp_show.tmpraddr[1] |= (atoi(tmpstr) << 8);
//	*raddr[1] |= atoi(tmpstr);
	uip_ipaddr_copy(raddr, temp_show.tmpraddr);

	tmpstr = strtok(NULL, " \r\n");
//	temp_show.tmprport = at_show.tmprport;
//	*rport = temp_show.tmprport;
	*rport = dm_uip_port(tmpstr);
}

void atsmd_resp_command_err(void)
{
	atcmd_resp_cmd("AT command error...!!!");
}

void atcmd_show_sys_msg(uint8_t state)
{
/* Was state == 1, loadp
 */
 
/* Is STATE == 1, showp
 */
		//uip_ipaddr_t ipaddr;
		atcmd_baseline(); //atcmd_resp_cmd(" ");

		/* STATE == 0, show
		 */
		Copy_AT_Type_to_Show(); //prepare //at_show = at_type; //eeprom_show = eeprom_type;
		
		/* STATE == 1, show
		 */
		if(state == 1){
#ifdef CUS_TEST
			showp_read_flag = 1;
#endif //CUS_TEST
			atcmd_resp_cmd("--- Display flash parameters ---+");		
			Read_AT_Show_DataFlash(0); //Read_AT_DataFlash_to_Show();
		}
}

void atcmd_set_baud_msg(void) {
		at_show.baudrate= at_type.baudrate;
		at_show.wordlen= at_type.wordlen;
		at_show.parity= at_type.parity;
		at_show.stop= at_type.stop;
}

void atcmd_show(uint8_t state) { //(struct at_showrank *show)
//			sprintf(at_tmpstr, "Role= %d", at_show.role);
			sprintf(at_tmpstr, "ROLE %d", at_show.role);
			/* HERE we can
			 *  strcat() to "ROLE" with "tcp Listen/tcp dconn/tcp sconn, udp Listen/udp dconn/udp sconn"
			 */
			atcmd_resp_cmd(at_tmpstr);
			
			atcmd_resp_dhcpc_mode(at_show.dhcpc_mode);
			
			sprintf(at_tmpstr, "Keepalive Mode= %d", at_show.keepalive_mode);
			atcmd_resp_cmd(at_tmpstr);
			
			atcmd_dnsc_mode(at_show.dns_mode); //"DNS Client Mode= 
			
//			sprintf(at_tmpstr, "Trans Lenght = %d", at_show.trans_len);
//			atcmd_resp_cmd(at_tmpstr);
			
		// Display MAC Address 
			sprintf(at_tmpstr, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", eeprom_show.macaddr[0], eeprom_show.macaddr[1],
								eeprom_show.macaddr[2], eeprom_show.macaddr[3], eeprom_show.macaddr[4], eeprom_show.macaddr[5]);
			atcmd_resp_cmd(at_tmpstr);
		// Display HOST IP Address		
			sprintf(at_tmpstr, "ip= %d.%d.%d.%d", uip_ipaddr1(at_show.hostip), uip_ipaddr2(at_show.hostip), uip_ipaddr3(at_show.hostip), uip_ipaddr4(at_show.hostip));
			atcmd_resp_cmd(at_tmpstr);
		// Display Network mask Address 
			sprintf(at_tmpstr, "mask= %d.%d.%d.%d", uip_ipaddr1(at_show.hostmask), uip_ipaddr2(at_show.hostmask), uip_ipaddr3(at_show.hostmask), uip_ipaddr4(at_show.hostmask));
			atcmd_resp_cmd(at_tmpstr);
		//Display Getway Address
			sprintf(at_tmpstr, "gw= %d.%d.%d.%d", uip_ipaddr1(at_show.hostgw), uip_ipaddr2(at_show.hostgw), uip_ipaddr3(at_show.hostgw), uip_ipaddr4(at_show.hostgw));
			atcmd_resp_cmd(at_tmpstr);
			
		// Display TCP Listen Port
			atcmd_resp_listen("tcp", at_show.t_lport);
			
		// Display UDP Listen Port
			atcmd_resp_listen("udp", at_type.u_lport);
			
		// Jerry add Display AT Flash TCP Static Client Remote Address and Port
			sprintf(at_tmpstr, "TCPSCONN= %d.%d.%d.%d:%d", //(Static)(Remote)(TCP Static CONN)
								uip_ipaddr1(at_show.tcp_raddr), uip_ipaddr2(at_show.tcp_raddr),
								uip_ipaddr3(at_show.tcp_raddr), uip_ipaddr4(at_show.tcp_raddr), at_show.tcp_rport);
			atcmd_resp_cmd(at_tmpstr);
			
		// Jerry add Display AT Flash UDP Static Client Remote Address and Port
			sprintf(at_tmpstr, "UDPSCONN= %d.%d.%d.%d:%d", //(Static)(Remote)(UDP Static CONN)
								uip_ipaddr1(at_show.udp_raddr), uip_ipaddr2(at_show.udp_raddr),
								uip_ipaddr3(at_show.udp_raddr), uip_ipaddr4(at_show.udp_raddr), at_show.udp_rport);
			atcmd_resp_cmd(at_tmpstr);
			
//		if (at_show.dns_mode) {
			sprintf(at_tmpstr, "DNS Server IP= %d.%d.%d.%d", uip_ipaddr1(at_show.dns_saddr), uip_ipaddr2(at_show.dns_saddr),
								uip_ipaddr3(at_show.dns_saddr), uip_ipaddr4(at_show.dns_saddr));
			atcmd_resp_cmd(at_tmpstr);
			
			if (state == 0) { 
				//&& at_type.dns_mode && at_type.dns_srvname_found)
				if (!atcmd_show_dns_srvname(&at_show))
					atcmd_resp_srvname(&at_show, 1);
			} else {
//				sprintf(at_tmpstr, "DNS SrvName= %s", at_show.dns_srvname); //" : %d", at_show._dns_srvport
//				atcmd_resp_cmd(at_tmpstr);
				atcmd_resp_srvname(&at_show, 1);
			}
//		}
			
			/*sprintf(at_tmpstr, "DNS Server Path= %s", at_type.dns_srvpath);
			atcmd_resp_cmd(at_tmpstr);*/
			
			// Display Baudrate Setup value
			//atcmd_baud_msg();
			atcmd_resp_baud_mode();

#if 0
			sprintf(at_tmpstr, "(%s)", role_desc_string(at_show.role)); //_show_role
			atcmd_resp_cmd(at_tmpstr);
#endif		
			/*if(state == 1){
				at_show.autoload =
				at_show.pincontrol=
				at_show.wakeupcontrol2=
				at_show.control3 = 
				at_show.vid= 
				at_show.pid= 
				atcmd_resp_cmd(at_tmpstr);
			}*/
		#if 0 //[temp debug]
			atcmd_translen();
		#endif
}

/**
 * atcmd_savep(): Save setting values to Data Flash.
 * AT command: SAVEP
 */
void atcmd_savep(void)
{
#if 1
	if ((at_type.role == ROLE_TCP_DCLIENT) || (at_type.role == ROLE_UDP_DCLIENT)) {
		atcmd_tcpip_role_err8(at_type.role); //"Should not"
		return;
	}
#else
		if (at_type.role == ROLE_TCP_DCLIENT) {
			atcmd_tcpip_role_err8(ROLE_TCP_DCLIENT); //"Should not"
//			atcmd_tcpip_role_err10(ROLE_TCP_SCLIENT); 
//			at_type.role= ROLE_TCP_SCLIENT;
			return;
		}
		if (at_type.role == ROLE_UDP_DCLIENT) {
			atcmd_tcpip_role_err8(ROLE_UDP_DCLIENT); //"Should not"
//			atcmd_tcpip_role_err10(ROLE_UDP_SCLIENT); 
//			at_type.role= ROLE_UDP_SCLIENT;
			return;
		}
#endif
	
		Write_AT_DataFlash();	//savep
	
		if((at_type.role == ROLE_TCP_SCLIENT) || (at_type.role == ROLE_UDP_SCLIENT)){
			atcmd_resp_rst(); //atcmd_resp_cmd("Send 'RST' to reset MCU");
		}else{
			atcmd_ready();
		}
}

/**
 * atcmd_rst(): Reset MCU.
 * AT command: RST
 *
 * 原 AT32 版本用 IAP __asm 手動跳轉；MH2203 沒有這個需求，直接走 at_port 的
 * 軟體系統重置 (NVIC_SystemReset)。
 */
void atcmd_rst(void)
{
	atp_system_reset();
}

/**
 * atcmd_version(): print version information.
 * AT command: VERSION
 */
void atcmd_version(void)
{
		atcmd_resp_cmd(IPSPP_VERSION);	
}

#if 0
/*
 * atcmd_help(): Display Help Mesage.
 * AT command: HELP | -h | ?
 */
void atcmd_help(void)
{		
		sprintf(at_tmpstr, "RDY:           check entry AT Command mode");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "HELP | -h | ?  Show al AT Command Description");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "VERSION:       Display IPSPP version");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "RESTORE:       Restore default setup and wrrte to DataFlash");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "RST:           Reset MCU Module");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "SHOW:          Display current system information");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "SHOWP:         Display DataFlash information");
		atcmd_resp_cmd(at_tmpstr);	
		sprintf(at_tmpstr, "SAVEP:         Save parameters to Data Flash");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "BAUD:          setup USART1 baudrate, ex:baud 115200 8 0 1");
		atcmd_resp_cmd(at_tmpstr);
	
		sprintf(at_tmpstr, "ROLE <mode>    User option TCP/UDP mode");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "<mode> :0 : TCP server\r\n           1 : TCP dynamic Client");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "        2 : TCP static Client\r\n    3 : UDP server");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "        4 : UDP dynamic Client\r\n   5 : UDP static Client");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "usage : role 0");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "DHCPC ON/OFF:  DHCP client mode switch, ex:dhcpc on");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "MAC:           Host MAC address, ex:mac 00:60:6E:90:51");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "IP:            Host IP address, ex:ip 192.168.0.100");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "GW:            Host Getway address, ex:gw 192.168.0.1");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "MASK:          Host netmask address, ex:mask 255.255.255.0");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "TCPLPORT <Port num>                  TCP server port to listen on/connect");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "TCPDCONN <Remote IP>:<Remote Port>   TCP client dynamic connect to server");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "TCPSCONN <Remote IP>:<Remote Port>   TCP client static connect to server");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "UDPLPORT <Port num>                  UDP server port to listen on/connect");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "UDPDCONN <Remote IP>:<Remote Port>   UDP client dynamic connect to server");
		atcmd_resp_cmd(at_tmpstr);
		sprintf(at_tmpstr, "UDPSCONN <Remote IP>:<Remote Port>   UDP client static connect to server");
		atcmd_resp_cmd(at_tmpstr);
}
#endif //0

/**
 * atcmd_ready() : Response AT Command OK.
 * AT command: RDY
 */
//void atcmd_ready(void)
//{
//		char *okstr = "OK";
//		
//		atcmd_resp_cmd(okstr);
//}
