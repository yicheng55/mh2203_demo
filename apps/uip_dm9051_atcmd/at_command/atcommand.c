/* Includes ------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uip.h"
#include "uip-split.h"

#include "at_port.h"
#include "atcommand.h"
#include "etherbridge.h"

extern uint8_t eth_netif_linkup; /* app_call.c 提供 (取代 AT32 uip_init.c) */

/* DM9051 預設 MAC OUI (原定義於 AT32 SDK 的 DM9051.h，僅供 atcmd_restore() 預設值使用) */
#define emacETHADDR0   0x00
#define emacETHADDR1   0x60
#define emacETHADDR2   0x6E
#define emacETHADDR3   0xB4
#define emacETHADDR4   0x7E
#define emacETHADDR5   0x34

struct at_funcation at_type;
struct eeprom_funcation eeprom_type;
struct udpbc_state mudpcs;

char at_tmpstr[AT_TMPSTR_LEN];
uint8_t atcmd_flag = FALSE;
char g_u8RecData[RecData_Size];
uint8_t tcp_connected = FALSE;
uint8_t udp_connected = UPDATE_UDP_NULL; //.FALSE;
//uint8_t trans_connected = UPDATE_DMP_NULL;;
volatile unsigned int gt_u32comRbytes = 0;
uint8_t gt_comeDataUsart2 = 0;
uint8_t manualset_dns_srvip = 0;

#ifdef ATCMD_UART_RX_DOUB_BUF
char g_u8RecData1[RecData_Size];
volatile unsigned int g_u32comRbytes1 = 0;
#endif //ATCMD_UART_RX_DOUB_BUF

const char *COMMAND_STR[] = {
	"ERR cmd", //0
};

#define ID_ERRCMD	0

/**
 * atcmd_restore(): Setup default value to DataFlash
 * AT command: RESTORE
 */
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
			.dns_srvname[0] = ' ', //(' ', 0,), //NULL,
			.dns_srvname[1] = 0, 
			.dns_srvpath[0] = ' ', //(' ', 0,), //NULL,
			.dns_srvpath[1] = 0, 
			.dns_srvport = 80, //67, //80, //67 recommed //80 good demo explain
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
			.macaddr = {emacETHADDR0, emacETHADDR1, emacETHADDR2, emacETHADDR3,emacETHADDR4, emacETHADDR5},
			.autoload = 0x1401,
			.vid = 0x0A46,
			.pid = 0x9051,
			.pincontrol = 0xFFFF,
			.wakeupcontrol2 = 0x0180,
			.control3 = 0,
			//.ee_lastport = 0,
		};
	
		at_type = _at_type;
		eeprom_type = _eeprom_type;
		Write_AT_DataFlash();	//restore
		
		/* 20240603 add */
		uip_sethostaddr(at_type.hostip);
		uip_setnetmask(at_type.hostmask);	
		uip_setdraddr(at_type.hostgw);
}

/*
 * Sent data move to TX buffer
 */
//void
//senddata(const void *data, int len)
//{
//  if(len > 0)
//  {
//    uip_len = len;
//    memcpy(&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], (data), uip_len);
//  }
//}

#ifdef HTTP_SERVER_SUPPORT
uint8_t eth_rcv_strtok_process(char *buf, char *tmp[])
{
		uint8_t n = 0, i;
#ifdef TL_PAGE_NOT_PARSE_DBG
		uint8_t slen = strlen(buf);
		printf("toklen %u", slen);
		for (i= 0; buf[i] && (i < slen); i++) {
			if (!(i%80))
				printf("\r\n");
			printf("%c", buf[i]);
		}
		printf("\r\n");
#endif
		for(tmp[n] = strtok(buf, "&"); tmp[n] != NULL; tmp[n] = strtok(NULL, "&")) {
			n++;
		}
#ifdef TL_PAGE_NOT_PARSE_DBG
		if (n) {
			//printf("Parsing: end as (%s)\r\n", tmp[n-1]);
			for (i= 0; i < n; i++)
				printf("- tmp[%d]= %s\r\n", i, tmp[i]);
			printf("Hit: (%s)\r\n", tmp[n-1]);
		}
#endif
		return n;
}

//void eth_rcv_strtok_report(uint8_t n)
//{
//	if (n) {
//		printf("0 rcv_atcmd_process() %u param value\r\n", n); //xxx.shtml
//	}
//}

/* 
 * In below:
 *  parse to at_show/_eeprom_show is better,
 *  keep at_type no-touch on write(/except restore)..
 */
extern struct at_funcation at_show;
//extern struct eeprom_funcation eeprom_show;

//static boolean param8_strtok_atoi(char *buf, char *param_name, uint8_t *pu8data)
//{
//	if(!strcasecmp(buf, param_name)) {
//		buf = strtok(NULL, "=");
//		*pu8data= atoi(buf);
//		return TRUE;
//	}
//	return FALSE;
//}
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
#ifdef TL_PAGE_NOT_PARSE_DBG
	printf("elm.s %s\r\n", buf);
#endif
	while (*buf== '+')
		buf++;
#ifdef TL_PAGE_NOT_PARSE_DBG
	printf("elm.e %s\r\n", buf);
#endif
//	display_data(PF_MODE_CHAR, buf, 26);
//	display_data(PF_MODE_HEX, buf, 26);
	return buf;
}

void eth_rcv_atcmd_to_at_show_process(uint8_t n, char *tmp[])
{
		uint8_t j;
		char *buf = "void";
		/* 
		 * In below:
		 *  parse to at_show/_eeprom_show is better,
		 *  keep at_type no-touch on write(/except restore)..
		 */
		for(j = 0; j < n; j++)
		{
			uint32_t u32data;
			buf = strtok(tmp[j], "=");
			if (param32_strtok_atoi(buf, "tnmode", &u32data)) {
				at_show.role = u32data; //[.webpage, only 0/2/3/5]
			}
			else if(param32_strtok_atoi(buf, "staticip", &u32data)){ //oppsite value!
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
				at_show.keepalive_mode = u32data; //[0/1]
			}else if(param32_strtok_atoi(buf, "bd", &u32data)){
				at_show.baudrate = u32data;
			}else if(param32_strtok_atoi(buf, "wln", &u32data)){ //wordlen
				at_show.wordlen = u32data;
			}else if(param32_strtok_atoi(buf, "pty", &u32data)){
				at_show.parity = u32data; //[1/2/3]
			}else if(param32_strtok_atoi(buf, "stop", &u32data)){
				at_show.stop = u32data; //[1/2/3]
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
#ifdef TL_PAGE_NOT_PARSE_DBG
display_data(PF_MODE_CHAR, at_show.dns_srvname, 32);
display_data(PF_MODE_HEX, at_show.dns_srvname, 32);
#endif
				} else {
					//at_show.dns_srvname = NULL;
					at_show.dns_srvname[0] = ' ';
					at_show.dns_srvname[1] = 0;
				}
			}

			//[save rom size]
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
					
#if 1
					//printf("[--- TO START, Write_AT_Show_DataFlash() direct to operate 'STATE_OUTPUT' ---]\r\n");
					Write_AT_Show_DataFlash(); //instead, Write_SYSCFG_DataFlash(); //web button
					//printf("[--- DONE, Write_AT_Show_DataFlash() direct to operate 'STATE_OUTPUT' ---]\r\n");
#else
					printf("[- - - HALF SKIP-TEST - - -]\r\n");
					printf("[--- SKIP, Write_AT_Show_DataFlash() and direct go to operate 'STATE_OUTPUT' ---]\r\n");
#endif				
					
					#if 0
					//web button only as 'savep'
					EthernetInit_Done();
					#endif
			}
		}
#ifdef TL_PAGE_NOT_PARSE_DBG
		printf("Parse x%u Items, last is %s\r\n", j, buf);
		printf("........... at_show.baudrate %u\r\n", at_show.baudrate);
		printf("........... at_show.wordlen %u\r\n", at_show.wordlen);
		printf("........... at_show.parity %u\r\n", at_show.parity);
		printf("........... at_show.stop %u\r\n", at_show.stop);
		printf("\r\n");
#endif
}

#if 0
//[eth_rcv_atcmd_process() .original]
void eth_rcv_atcmd_process(uint8_t n, char *tmp[])
{
		uint8_t j;
		char *buf = "void";

		/* 
		 * In below:
		 *  parse to at_show/_eeprom_show is better,
		 *  keep at_type no-touch on write(/except restore)..
		 */
		#if 0
		//uint8_t i = 0;
		//char *tmp[32];
		for(tmp[i] = strtok(buf, "&"); tmp[i] != NULL; tmp[i] = strtok(NULL, "&"))
		{
			printf("1 tmp[%d]= %s\r\n", i, tmp[i]);
		  //printf("1 tmp[%d]= %s\r\n", i, tmp[i]);
			i++;
			buf = NULL;
		}
		#endif

		for(j = 0; j < n; j++)
		{
//			uint8_t u8data;
//			uint16_t u16data;
			uint32_t u32data;
			buf = strtok(tmp[j], "=");
			//printf("2 tmp[%d] = %s\r\n", j, tmp[j]);
			//printf("2 tmp[%u] is %s\r\n", j, buf);

		#if 0
			if(!strcasecmp(buf, "tnmode"))
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				switch(atoi(buf))
		#endif
			if (param32_strtok_atoi(buf, "tnmode", &u32data)) {
				at_show.role = u32data; //[.webpage, only 0/2/3/5]
			}
		#if 0
			else if(!strcasecmp(buf, "staticip"))
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				if(atoi(buf) == 1)
			} else if(!strcasecmp(buf, "sip1")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				at_type.hostip[0] = atoi(buf);
			}else if(!strcasecmp(buf, "sip2")){
				buf = strtok(NULL, "=");		
				//printf("0 value %u\r\n", atoi(buf));			
				at_type.hostip[0] |= (atoi(buf) << 8);
				//printf("sip2 %d\r\n", atoi(buf));
			}else if(!strcasecmp(buf, "sip3")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				at_type.hostip[1] = atoi(buf);
				//printf("sip3 %d\r\n", atoi(buf));
			}else if(!strcasecmp(buf, "sip4")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				at_type.hostip[1] |= (atoi(buf) << 8);
				//printf("sip4 %d\r\n", atoi(buf));
		#endif
			else if(param32_strtok_atoi(buf, "staticip", &u32data)){ //oppsite value!
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
		#if 0
			else if(!strcasecmp(buf, "tlp")){
				buf = strtok(NULL, "=");			
				if((at_type.role == 0) && (atoi(buf) != 0)){
					at_type.t_lport = atoi(buf);
				}else if(at_type.role == 3){
					at_type.u_lport = atoi(buf);
				}
			}
		#endif
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
		#if 0
			else if(!strcasecmp(buf, "trip1")){
				buf = strtok(NULL, "=");			
				//printf("0 value %u\r\n", atoi(buf));		
				if((at_type.role == 1) || (at_type.role == 2)){
					at_type.tcp_raddr[0] = atoi(buf);
					//printf("tcp_raddr %d\r\n", atoi(buf));
				}else if((at_type.role == 4) || (at_type.role == 5)){
					at_type.udp_raddr[0] = atoi(buf);
					//printf("udp_raddr %d\r\n", atoi(buf));
				}
			}else if(!strcasecmp(buf, "trip2")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));					
				if((at_type.role == 1) || (at_type.role == 2)){
					at_type.tcp_raddr[0] |= (atoi(buf) << 8);
					//printf("tcp_raddr %d\r\n", atoi(buf));
				}else if((at_type.role == 4) || (at_type.role == 5)){
					at_type.udp_raddr[0] |= (atoi(buf) << 8);
					//printf("udp_raddr %d\r\n", atoi(buf));
				}
			}else if(!strcasecmp(buf, "trip3")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));	
				if((at_type.role == 1) || (at_type.role == 2)){
					at_type.tcp_raddr[1] = atoi(buf);
					//printf("tcp_raddr %d\r\n", atoi(buf));
				}else if((at_type.role == 4) || (at_type.role == 5)){
					at_type.udp_raddr[1] = atoi(buf);
					//printf("udp_raddr %d\r\n", atoi(buf));
				}	
			}else if(!strcasecmp(buf, "trip4")){
				buf = strtok(NULL, "=");		
				//printf("0 value %u\r\n", atoi(buf));
				if((at_type.role == 1) || (at_type.role == 2)){
					at_type.tcp_raddr[1] |= (atoi(buf) << 8);
					//printf("tcp_raddr %d\r\n", atoi(buf));
				}else if((at_type.role == 4) || (at_type.role == 5)){
					at_type.udp_raddr[1] |= (atoi(buf) << 8);
					//printf("udp_raddr %d\r\n", atoi(buf));
				}
			}
		#endif
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
		#if 0
			else if(!strcasecmp(buf, "trp")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				if(((at_type.role == 1) || (at_type.role == 2))){
					at_type.tcp_rport = atoi(buf);
					//printf("tcp_rport = %d\r\n",atoi(buf));
				}else if(((at_type.role == 4) || (at_type.role == 5))){
					at_type.udp_rport = atoi(buf);
					//printf("udp_rport = %d\r\n",atoi(buf));
				}
			}else if(!strcasecmp(buf, "ka")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				if(atoi(buf) == 1){
					at_type.keepalive_mode = 1;
				}else{
					at_type.keepalive_mode = 0;
				}
				//printf("keepalive_mode = %d\r\n",atoi(buf));
			}else if(!strcasecmp(buf, "bd")){ //need modify correspond to at-command def.
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				at_type.baudrate = atoi(buf);
				//printf("baud = %d\r\n",atoi(buf));
			}else if(!strcasecmp(buf, "wln")){ //need modify to wordlen 8/wordlen 9
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				at_type.wordlen = atoi(buf);
				//printf("wordlen = %d\r\n",atoi(buf));
			}else if(!strcasecmp(buf, "pty")){ //need modify to parity 1 (odd)/parity 2 (even)
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				if(atoi(buf) == 0){
					at_type.parity = 0;
				}else if(atoi(buf) == 1){
					at_type.parity = 1;
				}else if(atoi(buf) == 2){
					at_type.parity = 2;
				}
				//printf("parity = %d\r\n",atoi(buf));
			}else if(!strcasecmp(buf, "stop")){ //need modify to stop 2 (1.5-bit)/stop 3 (2-bit)
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				if(atoi(buf) == 1){
					at_type.stop = 1;
				}else if(atoi(buf) == 2){
					at_type.stop = 2;
				}else if(atoi(buf) == 3){
					at_type.stop = 3;
				}
				//printf("at_type.stop %x\r\n", at_type.stop);
			}
		#endif
			else if(param32_strtok_atoi(buf, "trp", &u32data)){
				if((at_show.role == 1) || (at_show.role == 2))
					at_show.tcp_rport = u32data;
				else if((at_show.role == 4) || (at_show.role == 5))
					at_show.udp_rport = u32data;
			}else if(param32_strtok_atoi(buf, "ka", &u32data)){
				at_show.keepalive_mode = u32data; //[0/1]
			}else if(param32_strtok_atoi(buf, "bd", &u32data)){
				at_show.baudrate = u32data;
			}else if(param32_strtok_atoi(buf, "wln", &u32data)){ //wordlen
				at_show.wordlen = u32data;
			}else if(param32_strtok_atoi(buf, "pty", &u32data)){
				at_show.parity = u32data; //[1/2/3]
			}else if(param32_strtok_atoi(buf, "stop", &u32data)){
				at_show.stop = u32data; //[1/2/3]
			}
		#if 0
			else if(!strcasecmp(buf, "dnsmode")){
				buf = strtok(NULL, "=");
				//printf("0 value %u\r\n", atoi(buf));
				at_type.dns_mode = atoi(buf);
				//printf("dnsmode = %d\r\n", at_type.dns_mode);
			}
		#endif
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
#if 0
//			}else if(!strcasecmp(buf, "dsp")){
//				buf = strtok(NULL, "=");
//				at_type.dns_srvport = atoi(buf);
//				//printf("dsp = %d\r\n", at_type._dns_srvport); 
//			}
#endif
			else if(!strcasecmp(buf, "srvname")){
				buf = strtok(NULL, "=");
				
				if (srvname_valid_ok(buf)) {
					sprintf(at_show.dns_srvname, "%s", elimite_head(buf));
#ifdef TL_PAGE_NOT_PARSE_DBG
display_data(PF_MODE_CHAR, at_show.dns_srvname, 32);
display_data(PF_MODE_HEX, at_show.dns_srvname, 32);
#endif
				} else {
					//at_show.dns_srvname = NULL;
					at_show.dns_srvname[0] = ' ';
					at_show.dns_srvname[1] = 0;
				}
				//printf("0 str-value %s\r\n", buf);
				//printf("srvname = %s\r\n", at_type.dns_srvname);
			}
#if 0
//			else if(!strcasecmp(buf, "srvpath")){
//				buf = strtok(NULL, "=");
//				sprintf(at_type.dns_srvpath, "%s", buf);
//				//printf("0 str-value %s\r\n", buf);
//				//printf("srvpath = %s\r\n", at_type.dns_srvpath);
//			}
#endif
//[save rom size]
			else if(!strcasecmp(buf, "restore")){
			
//				buf = strtok(NULL, "=");
//				if(atoi(buf) == 1){
//				}
					atcmd_restore();
					if (atcmd_on_resp_note())
						atcmd_resp_ip();
			}else if(!strcasecmp(buf, "rst")){
			
//				buf = strtok(NULL, "=");
//				if(atoi(buf) == 1){
//				}
					atcmd_rst();
			}else if(!strcasecmp(buf, "Disconnect")){
			
//				buf = strtok(NULL, "=");
//				if(atoi(buf) == 1){
//				}
					if(on_trans_mode()){
						sprintf(uip_appdata, "+++");
						recv_cmd_auto_disconnect(UIP_PROTO_TCP);
					}
			}else if(!strcasecmp(buf, "Save")){
			
//				buf = strtok(NULL, "=");
//				if(atoi(buf) == 1){
//				}

					if(on_trans_mode()){
						sprintf(uip_appdata, "+++");
						recv_cmd_auto_disconnect(UIP_PROTO_TCP);
					}
					_Write_AT_Show_DataFlash(); //instead, Write_SYSCFG_DataFlash(); //web button
#if 0
					//web button only as 'savep'
					EthernetInit_Done();
#endif
			}
		}
//		printf("Item %u. is %s\r\n", j, buf);
//		printf("Item %u. is %s\r\n", j, buf);
//		printf("Item %u. is %s\r\n", j, buf);
//		printf("Item %u. is %s\r\n", j, buf);
		printf("x%u Items, last is %s\r\n", j, buf);
		//return 1;
} //atoi
#endif
#endif //HTTP_SERVER_SUPPORT

char *gt_tmpstr, *gt_tmpstrX;

char *gt_strtok(char *pattern) {
	/* NULL instead tmpstr,
	   Need check if tmpstr */
	char *newstr = strtok(NULL, pattern);
	if (!newstr)
		return NULL;
//	if (newstr[0] == 0x0d || newstr[0] == 0x0a || newstr[0] == 0x20)
//		return NULL;
	return newstr;
}

/**
 * @brief  Distinguish commad and to execution.
 * @param  pAtRcvData: point to received (command) 
 * @retval None
 */
void at_cmdProcess(void)
{
			char *tmpstr;
			if(on_trans_out_mode() && eth_netif_linkup) //UPDATE_UDP_SEND/UPDATE_UDP_CONNECTED
			{
				/*** UART command input ***/
				//printf(".trans_out_mode\r\n");
			}else{
				/* Setting AT Command */

//				if (atcmd_flag == FALSE) {
//					if (gt_u32comRbytes) {
//						display_str_state(1, g_u8RecData, "Un-used command");
//						memset(g_u8RecData, 0, sizeof(g_u8RecData));
//						gt_u32comRbytes = 0;
//						atcmd_flag = FALSE;
//					}
//				} else

				//printf(".at_cmd\r\n");
				if (atcmd_flag)
				{
					
					display_str_state(1, g_u8RecData, "Input command");

					tmpstr = strtok(g_u8RecData, " \r\n");
					
					//Base AT CMD
					if(!strcasecmp(tmpstr,"RDY")) {   //Check AT mode
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (!tmpstr)
								atcmd_ready();
							else
								goto CMD_ERR; //atsmd_resp_command_err();
					}/*else if(!strcasecmp(tmpstr,"?") || !strcasecmp(tmpstr,"HELP") || !strcasecmp(tmpstr,"-h")){ // Display AT Command Help Message
							atcmd_help();
					}*/else if(!strcasecmp(tmpstr,"VERSION")) { //also, || !strcasecmp(tmpstr,"-v")
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (!tmpstr) {
								atcmd_baseline();
								atcmd_version();
							} else
								goto CMD_ERR; //atsmd_resp_command_err();
					}else if(!strcasecmp(tmpstr, "RESTORE")) {	// Restore parameters to Data Flash
							atcmd_restore();
							atcmd_ready();
					}else if(!strcasecmp(tmpstr, "RST")) {		// Reset MCU System
							atcmd_rst();
					}else if(!strcasecmp(tmpstr, "SAVEP")) {  // Save parameters to Data Flash
							atcmd_savep();
					}else if(!strcasecmp(tmpstr, "SHOW")) {
							atcmd_show_sys_msg(0);
							atcmd_show(0); 
					}else if(!strcasecmp(tmpstr, "SHOWP")) {  // Display parameters from Data Flash
							atcmd_show_sys_msg(1);
							atcmd_show(1); 
					}else if(!strcasecmp(tmpstr, "BAUD")) {   //UART baudrate setting
//							tmpstr = strtok(NULL, " \r\n");
//							display_str_state(0, tmpstr, "BAUD");
							tmpstr = trans_strtok(tmpstr, " \r\n");
//							display_str_state(0, tmpstr, "trans");
							
							if (atoi_to_baud(tmpstr)) {
//								atcmd_set_baud_msg();
//								atcmd_resp_baud_mode();
								atcmd_ready();
							} else
								goto CMD_ERR; //atsmd_resp_command_err();
					}
#if 0
					else if(!strcasecmp(tmpstr, "TRANSLEN")){
						tmpstr = strtok(NULL, " \r\n");
						at_type.trans_len = (uint16_t)atoi(tmpstr);
						atcmd_translen();
					}
#endif

					// TCP/IP AT CMD		
					else if(!strcasecmp(tmpstr, "ROLE")) { //Setup Transparent TCP / UDP Mode
//							tmpstr = strtok(NULL, " \r\n");
							tmpstr = trans_strtok(tmpstr, " \r\n");
							
							if (!tmpstr)
								atcmd_resp_tcpip_role(0); //atcmd_role_command(at_type.role);
							else if (atcmd_tcpip_role(tmpstr[0] - 0x30) && tmpstr[1] == 0)
								atcmd_role_command(tmpstr[0] - 0x30);
							else
								goto CMD_ERR; //atsmd_resp_command_err();
					}else if(!strcasecmp(tmpstr, "DHCPC")) { //Setup DHCPC Mode
//							tmpstr = strtok(NULL, " \r\n");
							tmpstr = trans_strtok(tmpstr, " \r\n");

							if (strcasecmp_dhcpc_ok(tmpstr))
								atcmd_resp_dhcpc_mode(at_type.dhcpc_mode); //atcmd_tcpip_dhcpc(at_type.dhcpc_mode);
							else
								goto CMD_ERR; //atsmd_resp_command_err();
					}else if(!strcasecmp(tmpstr, "MAC")) {
//							tmpstr = strtok(NULL, ":");
//							display_str_state(0, tmpstr, "MAC");
							tmpstr = trans_strtok(tmpstr, ":");
//							display_str_state(0, tmpstr, "trans");
							
							if (strtol_mac_ok(tmpstr))
								atcmd_tcpip_mac();
							else 
								goto CMD_ERR; //atsmd_resp_command_err();
					}else if(!strcasecmp(tmpstr, "IP")) {
//							debug_data("aft-ip", tmpstr);
//							debug_data("aft-'.'", tmpstr);
//							tmpstr = strtok(NULL, ".");

//							display_str_state(0, tmpstr, "IP");
							tmpstr = trans_strtok(tmpstr, ".");
//							display_str_state(0, tmpstr, "trans");
							atcmd_tcpip_ip(tmpstr);
					}else if(!strcasecmp(tmpstr, "GW")) {
					
//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");
							atcmd_tcpip_gw(tmpstr);
					}	else if(!strcasecmp(tmpstr, "MASK")) {
					
//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");
							atcmd_tcpip_mask(tmpstr);
					}else if(!strcasecmp(tmpstr, "TCPLPORT")) {		// TCP listen port
//							tmpstr = strtok(NULL, " \r\n");
							tmpstr = trans_strtok(tmpstr, " \r\n");
							
							if (tmpstr) {
								at_type.t_lport = dm_uip_port(tmpstr);
								
								if(at_type.role != ROLE_TCP_SERVER){
									atcmd_tcpip_role_err7(ROLE_TCP_SERVER); //"Should"
								}else{
									atcmd_tcpip_tcplport();
								}
							} else {
//								sprintf(at_tmpstr, "TCP Listen Port= %d", at_type.t_lport);
//								atcmd_resp_cmd(at_tmpstr);
								atcmd_resp_listen("tcp", at_type.t_lport);
							}
					}else if(!strcasecmp(tmpstr, "TCPDCONN")) {		//Dynamic Connect TCP server
//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) {
							
								/*	Data: temp storage
									uip_ipaddr_t s_srv_saddr;    //Server IP
									uint16_t s_srv_port;        //Server port
								 */
								/*	Get : function
									atoi_connect_ip(tmpstr, &at_type.tcp_raddr, &at_type.tcp_rport)
								 */
								atoi_connect_ip(tmpstr, &at_type.tcp_raddr, &at_type.tcp_rport);
//								at_type.tcp_raddr[0] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ".");
//								at_type.tcp_raddr[0] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, ".");
//								at_type.tcp_raddr[1] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ":");
//								at_type.tcp_raddr[1] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, " \r\n");
//								at_type.tcp_rport = dm_uip_port(tmpstr);

								if(at_type.role != ROLE_TCP_DCLIENT){
									atcmd_tcpip_role_err7(ROLE_TCP_DCLIENT); //"Should"
								}else{
									atcmd_tcpip_tcpconn();
								}
							} else {
								sprintf(at_tmpstr, "TCP Dynamic CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.tcp_raddr), uip_ipaddr2(at_type.tcp_raddr),
													uip_ipaddr3(at_type.tcp_raddr), uip_ipaddr4(at_type.tcp_raddr), at_type.tcp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "TCPSCONN")) {	 // Static TCP Client CONNECT SERVER
//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) { 
								atoi_connect_ip(tmpstr, &at_type.tcp_raddr, &at_type.tcp_rport);
//								at_type.tcp_raddr[0] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ".");
//								at_type.tcp_raddr[0] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, ".");
//								at_type.tcp_raddr[1] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ":");
//								at_type.tcp_raddr[1] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, " \r\n");

//								at_type.tcp_rport = dm_uip_port(tmpstr);
							
								if(at_type.role != ROLE_TCP_SCLIENT){
									atcmd_tcpip_role_err7(ROLE_TCP_SCLIENT); //"Should"
								}else{
									atcmd_savep();
								}
							} else {
								sprintf(at_tmpstr, "TCP Static CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.tcp_raddr), uip_ipaddr2(at_type.tcp_raddr),
													uip_ipaddr3(at_type.tcp_raddr), uip_ipaddr4(at_type.tcp_raddr), at_type.tcp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "UDPLPORT")) {	// UDP Server Listen port
//							tmpstr = strtok(NULL, " \r\n");
							tmpstr = trans_strtok(tmpstr, " \r\n");

							if (tmpstr) {
								at_type.u_lport = dm_uip_port(tmpstr);
							
								//if(at_type.role != ROLE_UDP_SERVER){
									//atcmd_tcpip_role_err();
								//}else{
									atcmd_tcpip_udplport();
								//}
							} else {
//								sprintf(at_tmpstr, "UDP Listen Port= %d", at_type.u_lport);
//								atcmd_resp_cmd(at_tmpstr);
								atcmd_resp_listen("udp", at_type.u_lport);
							}
					}else if(!strcasecmp(tmpstr, "UDPDCONN")) {//UDP Dynamic Client

//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");

							if (tmpstr) { 
								atoi_connect_ip(tmpstr, &at_type.udp_raddr, &at_type.udp_rport);
//								at_type.udp_raddr[0] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ".");
//								at_type.udp_raddr[0] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, ".");
//								at_type.udp_raddr[1] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ":");
//								at_type.udp_raddr[1] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, " \r\n");
//								
//								at_type.udp_rport = dm_uip_port(tmpstr);
							
								//if(at_type.role != ROLE_UDP_DCLIENT){
									//atcmd_tcpip_role_err();
								//}else{
									atcmd_tcpip_udpconn(ROLE_UDP_DCLIENT);
								//}
							} else {
								sprintf(at_tmpstr, "UDP Dynamic CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.udp_raddr), uip_ipaddr2(at_type.udp_raddr),
													uip_ipaddr3(at_type.udp_raddr), uip_ipaddr4(at_type.udp_raddr), at_type.udp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
					}else if(!strcasecmp(tmpstr, "UDPSCONN")) {//UDP Static Client
					
						if(at_type.role == ROLE_UDP_SCLIENT){
//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");
							
							if (tmpstr)
								atoi_connect_ip(tmpstr, &at_type.udp_raddr, &at_type.udp_rport);
								
							sprintf(at_tmpstr, "UDP Static CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.udp_raddr), uip_ipaddr2(at_type.udp_raddr),
												uip_ipaddr3(at_type.udp_raddr), uip_ipaddr4(at_type.udp_raddr), at_type.udp_rport);
							atcmd_resp_cmd(at_tmpstr);
								
							if (tmpstr)
								atcmd_savep();
									
#if 0
							if (tmpstr) { 
								atoi_connect_ip(tmpstr, &at_type.udp_raddr, &at_type.udp_rport);
//								at_type.udp_raddr[0] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ".");
//								at_type.udp_raddr[0] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, ".");
//								at_type.udp_raddr[1] = atoi(tmpstr);
//								tmpstr = strtok(NULL, ":");
//								at_type.udp_raddr[1] |= (atoi(tmpstr) << 8);
//								tmpstr = strtok(NULL, " \r\n");
//								
//								at_type.udp_rport = dm_uip_port(tmpstr);
								
								//if(at_type.role != ROLE_UDP_SCLIENT){
									//atcmd_tcpip_role_err();
								//}else{
									atcmd_savep();
								//}
							} else {
								// Jerry add Display AT Flash UDP Static Client Remote Address and Port
								sprintf(at_tmpstr, "UDP Static CONN= %d.%d.%d.%d:%d", uip_ipaddr1(at_type.udp_raddr), uip_ipaddr2(at_type.udp_raddr),
													uip_ipaddr3(at_type.udp_raddr), uip_ipaddr4(at_type.udp_raddr), at_type.udp_rport);
								atcmd_resp_cmd(at_tmpstr);
							}
#endif
						} else
							atcmd_tcpip_role_err7(ROLE_UDP_SCLIENT); //"Should"
					}
					// DNS switch & DNS server name & path
					else if(!strcasecmp(tmpstr, "DNSC")){ // DNS mode
							/* Oh, NULL, or tmpstr */
//							tmpstr = strtok(NULL, " \r\n");
							tmpstr = trans_strtok(tmpstr, " \r\n");
							if (tmpstr) {
								if(!strcasecmp(tmpstr, "ON")) {
//									dnsc_mode(1);
									at_type.dns_mode = 1;
									atcmd_dnsc_mode(1);
								}else if(!strcasecmp(tmpstr, "OFF")) {
//									dnsc_mode(0);
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
					else if(!strcasecmp(tmpstr, "SRVIP")){ //DNS Server IP
							/* Check, NULL to get next */
//							tmpstr = strtok(NULL, ".");
							tmpstr = trans_strtok(tmpstr, ".");
							if (tmpstr) {
								//atoi_connect_ip(tmpstr, &at_type.dns_saddr, &xxx);
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
					}else if(!strcasecmp(tmpstr, "SRVNAME")){ //DNS Server Name
#if 1
							// need debug more~
							//	tmpstr = strtok(NULL, ":");
							gt_tmpstr = trans_strtok(NULL/*tmpstr*/, ": \r\n");
							if (gt_tmpstr) {
								memset(&at_type.dns_srvname[0], 0, 32); //do asure.
								strcpy(&at_type.dns_srvname[0], gt_tmpstr);
								gt_tmpstrX = gt_strtok(" \r\n"); //strtok
								if (gt_tmpstrX) {
									#if 1
									//[NOUSED.] //gt_tmpstr = gt_strtok(" \r\n"); //strtok
									at_type.dns_srvport = atoi(gt_tmpstrX); // dns_srvport= 67 recommanded.
									#endif
								}
								//else goto CMD_ERR;
								//atcmd_resp_srvname(&at_type, 1); //._atcmd_dns_srvname();
							}
							//else
							//	atcmd_resp_srvname(&at_type, 1);
							atcmd_resp_srvname(&at_type, 1);
#endif
					}
//					else if(!strcasecmp(tmpstr, "SRVPATH")){ //DNS Server Path
//							tmpstr = strtok(NULL, " \r\n");
//							sprintf(at_type.dns_srvpath, "%s", tmpstr);
//							atcmd_dns_srvpath();
//					}
					// Keepalive mode & set timer/interval/probes
					else if(!strcasecmp(tmpstr, "KEEPALIVE")){ 
//							tmpstr = strtok(NULL, " \r\n");
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
//								sprintf(at_tmpstr, "Keepalive Mode= %d", at_type.keepalive_mode);
//								atcmd_resp_cmd(at_tmpstr);
								atcmd_tcpip_keepalive(at_type.keepalive_mode);
							}
					}
#if 0
					else if(!strcasecmp(tmpstr, "KACONFIG")){
						tmpstr = strtok(NULL, " \r\n");
						at_type.keepalive_no_data_period = atoi(tmpstr);
						tmpstr = strtok(NULL, " \r\n");
						at_type.keepalive_send_period = atoi(tmpstr);
						tmpstr = strtok(NULL, " \r\n");
						at_type.keepalive_send_count = atoi(tmpstr);
						
						atcmd_tcpip_keepalive_config();
					}
#endif
					else{
					CMD_ERR:
						atsmd_resp_command_err(); //atcmd_resp_cmd("AT command error...!!!");
						#if 0 //because EXTRA-GUSES-SOLUTION
						/* EXTRA-GUSES-SOLUTION */
						uart_data_pf(COMMAND_STR[ID_ERRCMD], PF_MODE_CHAR, g_u8RecData, gt_u32comRbytes);
						#endif
//						uart_clean_pf(COMMAND_STR[ID_ERRCMD], g_u8RecData, sizeof(g_u8RecData));  //memset(g_u8RecData, 0, sizeof(g_u8RecData));
//						uart_data_pf(COMMAND_STR[ID_ERRCMD], PF_MODE_HEX, g_u8RecData, gt_u32comRbytes);
//						gt_u32comRbytes = 0;
//						atcmd_flag = FALSE; //end atcmd mode
//						return;
					}

			#if 1 //EXTRA-GUESS-SOLUTION
					#if 0
					/* EXTRA-GUESS-DISP- */
					uart_data_pf("END-CLEAN", PF_MODE_HEX, g_u8RecData, gt_u32comRbytes);
					uart_clean_pf("SOL-CLEAN", g_u8RecData, sizeof(g_u8RecData));  //memset(g_u8RecData, 0, sizeof(g_u8RecData));
					#endif
					/* EXTRA-GUESS-SOLUTION */
					memset(g_u8RecData, 0, sizeof(g_u8RecData));
			#endif
					gt_u32comRbytes = 0;
					atcmd_flag = FALSE; //end atcmd mode

					return;
				}
		}
}
