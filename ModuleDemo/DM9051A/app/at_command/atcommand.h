#ifndef __ATCOMMAND_H
#define __ATCOMMAND_H

#include "stdint.h"
#include "uip.h"
#include "uip_mh2203.h"

#define IPSPP_VERSION "MH2203 DM9051 uIP ATCMD V1.0"

#ifdef UIP_USE_BIGDATA_SIZE
#define RecData_Size 2920
#else
#define RecData_Size 1460
#endif

#define TransData_Size 1472

#define ROLE_TCP_SERVER				0
#define ROLE_TCP_DCLIENT			1
#define ROLE_TCP_SCLIENT      2
#define ROLE_UDP_SERVER				3
#define ROLE_UDP_DCLIENT			4
#define ROLE_UDP_SCLIENT			5

#define DEF_HOSTIP1 					192
#define DEF_HOSTIP2 					168
#define DEF_HOSTIP3 					1
#define DEF_HOSTIP4 					100

#define DEF_HOSTNS1 					255
#define DEF_HOSTNS2 					255
#define DEF_HOSTNS3 					255
#define DEF_HOSTNS4 					0

#define DEF_HOSTGW1 					192
#define DEF_HOSTGW2 					168
#define DEF_HOSTGW3 					1
#define DEF_HOSTGW4 					1

#define DEF_PORTNUM						8080

#define DEF_BAUDRATE					115200
#define DEF_WORDLEN						8
#define DEF_PARITY						0
#define DEF_STOP							1

typedef enum {FALSE = 0, TRUE} boolean;

struct udpbc_state {
  struct pt pt;
  uint8_t state;
  struct uip_udp_conn *conn;
  struct timer timer;
  uint16_t ticks;
  uint8_t serverid[4];
};

struct at_funcation
{
	uint8_t dhcpc_mode;
	uint8_t role;
	uip_ipaddr_t hostip;
	uip_ipaddr_t hostmask;
	uip_ipaddr_t hostgw;
	uint16_t t_lport;
	uip_ipaddr_t tcp_raddr;
	uint16_t tcp_rport;
	uint16_t u_lport;
	uip_ipaddr_t udp_raddr;
	uint16_t udp_rport;
	uint8_t dns_mode;
	uip_ipaddr_t dns_saddr;
	char dns_srvname[32];
	char dns_srvpath[32];
	uint16_t dns_srvport;
	uint8_t keepalive_mode;
	uint8_t keepalive_send_period;
	uint8_t keepalive_no_data_period;
	uint8_t keepalive_send_count;
	uint32_t baudrate;
	uint8_t wordlen;
	uint8_t parity;
	uint8_t stop;
	uint16_t trans_len;
	uint16_t rst_count;
	char devicename[16];
	uint8_t  dns_srvname_found;
	uip_ipaddr_t srvname_saddr;
};

struct eeprom_funcation{
	uint8_t  macaddr[6];
	uint16_t autoload;
	uint16_t vid;
	uint16_t pid;
	uint16_t pincontrol;
	uint16_t wakeupcontrol2;
	uint8_t  control3;
};

extern struct at_funcation at_type;
extern struct eeprom_funcation eeprom_type;
extern uint8_t atcmd_flag;
extern char g_u8RecData[RecData_Size];
extern volatile unsigned int gt_u32comRbytes;
extern uint8_t gt_comeDataUsart2;
extern uint8_t tcp_connected;
extern uint8_t udp_connected;
extern uint8_t manualset_dns_srvip;
extern uint8_t showp_read_flag;
extern uint8_t UDPSrvConnFlag;

#ifdef ATCMD_UART_RX_DOUB_BUF
extern char g_u8RecData1[RecData_Size];
extern volatile unsigned int g_u32comRbytes1;
#endif

extern char at_tmpstr[];

void u_writeuart1(char uartchar);
boolean on_trans_mode(void);
void atcmd_baseline(void);
void atcmd_resp_cmd(const char *uartTXstr);
boolean atcmd_on_resp_note(void);
void at_cmdProcess(void);
void at_defaultset(void);
int AT_Pass_Custom_Mode(void);
void Write_AT_Show_DataFlash(void);
void Write_AT_DataFlash(void);
uint8_t Read_AT_Show_DataFlash(uint8_t inc);
uint8_t Read_AT_DataFlash(void);
void Copy_AT_Type_to_Show(void);
uint32_t Read_AT_FlashDWord(uint32_t u32WordIndex);
uint8_t eth_rcv_strtok_process(char *buf, char *tmp[]);
void eth_rcv_strtok_report(uint8_t n);
void eth_rcv_atcmd_process(uint8_t n, char *tmp[]);

void atcmd_help(void);
void atcmd_version(void);
void atcmd_restore(void);
void atcmd_rst(void);
void atcmd_savep(void);
void atcmd_resp_connect_timeout(uip_ipaddr_t *ipaddr, uint16_t rport);
void atcmd_resp_srvname(struct at_funcation *at_xxx, uint8_t sp);
uint8_t strcasecmp_dhcpc_ok(char *tmpstr);
uint8_t atoi_to_baud(char *tmpstr);
uint8_t strtol_mac_ok(char *tmpstr);
void atcmd_show_sys_msg(uint8_t p);
void atcmd_set_baud_msg(void);
void atcmd_baud_msg(void);
void atcmd_baud(void);
void atcmd_translen(void);
boolean atcmd_tcpip_role(uint8_t role);
char *make_role_msg(void);
void atcmd_role_command(uint8_t role);
void atcmd_resp_tcpip_role(uint8_t full);
void atsmd_resp_command_err(void);
void atcmd_resp_baud_mode(void);
void atcmd_show(uint8_t state);

void atcmd_tcpip_dhcpc(uint8_t sw);
void atcmd_tcpip_mac(void);
void atcmd_resp_ip(void);
void atcmd_resp_mask(void);
void atcmd_resp_gw(void);
void atcmd_tcpip_ip(char *tmpstr);
void atcmd_tcpip_mask(char *tmpstr);
void atcmd_tcpip_gw(char *tmpstr);
void atcmd_tcpip_tcplport(void);
void atcmd_tcpip_tcpconn(void);
void atcmd_tcpip_dnstcpconn(void);
void atcmd_tcpip_udplport(void);
void atcmd_tcpip_udpconn(uint8_t on_role);
void atcmd_tcpip_keepalive(uint8_t sw);
void atcmd_tcpip_keepalive_config(void);
void atcmd_resp_dhcpc_mode(uint8_t sw);
void dnsc_mode(uint8_t sw);
uint8_t atcmd_show_dns_srvname(struct at_funcation *at_xxx);
void atcmd_dnsc_mode(uint8_t sw);
void atcmd_dns_srvip(void);
void atcmd_dns_srvpath(void);
void eth_rcv_atcmd_to_at_show_process(uint8_t n, char *tmp[]);
boolean on_trans_out_mode(void);

#endif
