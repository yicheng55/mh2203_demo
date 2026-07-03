/*
 * Header file: trans_level.h
 * Created 20240514 Joseph
 */
#ifndef __TRANS_LEVEL_H__
#define __TRANS_LEVEL_H__

#include <stddef.h>
#include "uip.h"

#define ARP_COUNT				2

//["at_opts.h"]
#define AT_TMPSTR_LEN			86
#define AT_ROLESTR_LEN			22

//["connopts.h"]
#define UPDATE_TCP
#define UPDATE_UDP

#define UPDATE_UDP_CONN_DBG

//never
//#define UPDATE_UDP_ARP_TO_CONN

//["trans && dmp"]
//[There are "TCP/UDP/DMP"]
//[DMP means suitable for both TCP/UDP]
//#define UPDATE_DMP_NULL			0x00 //00
#define UPDATE_UDP_CONNECTED	0x01 //1 << 0
#define UPDATE_UDP_RDY			0x02 //1 << 1
#define UPDATE_TCP_CONNECTED	0x10 //1 << 4
#define UPDATE_TCP_RDY			0x20 //1 << 5
//#define UPDATE_DMP_ATCMD		0x80 //1 << 7

//["UDP"]
#define UPDATE_UDP_NULL			0x00
//#define UPDATE_UDP_CONNECTED	0x01
//#define UPDATE_UDP_RDY		0x02
#define UPDATE_UDP_SEND			(UPDATE_UDP_CONNECTED | UPDATE_UDP_RDY)
//["TCP"]
#define UPDATE_TCP_NULL			0x00
//#define UPDATE_TCP_CONNECTED	0x10
//#define UPDATE_TCP_RDY		0x20
#define UPDATE_TCP_SEND			(UPDATE_TCP_CONNECTED | UPDATE_TCP_RDY)

//["dbg.h"]
/* About 'TL_FLASH_NOT_WRITE_DBG'
 * Hint Flash (overhead 484 rom bytes, or more)
 * Testing relate to the web page 'save' button,
 *  too late reply TCP. Mass garbage send to the page of the browser.
 */
//#define TL_PAGE_NOT_PARSE_DBG  
#define TL_FLASH_NOT_WRITE_DBG  

extern const char *TRANSLEVEL_STR[];

#ifdef TL_FLASH_NOT_WRITE_DBG
  void display_flash_data(uint32_t addr, uint32_t data);
//  void display_flash_data8SrvName(uint32_t addr_s, uint32_t addr, uint32_t data);
//  void display_flash_data8SrvPath(uint32_t addr_s, uint32_t addr, uint32_t data);
#else //TL_FLASH_NOT_WRITE_DBG
  #define display_flash_data(addr, data)
//  #define display_flash_data8SrvName(addr_s, addr, data)
//  #define display_flash_data8SrvPath(addr_s, addr, data)
#endif //TL_FLASH_NOT_WRITE_DBG

#define TL_ROLE_TX_DBG			//Role TX

void display_role_tx(void);

//["atcmd-msg{]
void atcmd_resp_boot(void); //[sys init call]
void print_resp_boot(void); //[sys init call]
void atcmd_resp_dhcpc(void); //[dhcpc init]
void atcmd_resp_dnsc(void); //[dnsc init]
void print_resp_dnsc(void); //msg
void atcmd_resp_establish(void); //msg
void atcmd_resp_atcmd_ready(void); //bridge_init ready
void atcmd_resp_trans_ready(uint8_t id); //bridge_init ready
void atcmd_resp_trans_wait(uint8_t id); //bridge_init ready
void print_trans_ready(void); //bridge_init ready
void atcmd_ready(void); //msg
void atcmd_resp_rst(void); //msg
void atcmd_resp_note(void); //msg
//void atcmd_tcpip_role_err7(uint8_t role);
//void atcmd_tcpip_role_err8(uint8_t role);
void atcmd_tcpip_role_err(uint8_t id, uint8_t role);
//void atcmd_tcpip_role_to(uint8_t id, uint8_t role);
#define atcmd_tcpip_role_err7(role) 	atcmd_tcpip_role_err(7, role)
#define atcmd_tcpip_role_err8(role) 	atcmd_tcpip_role_err(8, role)
//#define atcmd_tcpip_role_err10(role)	atcmd_tcpip_role_to(10, role)

const char *role_config_string(uint8_t role);
const char *role_desc_string(uint8_t role);
const char *rolemode_string(uint8_t b);

void atcmd_resp_role_start(uint8_t role);

void atcmd_resp_listen(char *head, uint16_t rport);
void atcmd_comp_listen_port(char *head, uip_ipaddr_t ipaddr, uint16_t rport);
void atcmd_resp_remote(char *head, uint8_t role, uip_ipaddr_t ip_raddr, uint16_t rport);
void atcmd_resp_configured(char *head, uint8_t role, uint16_t lport);

//["uip_ipspp_options.h"]
void arp_need_new(void); //[sys init call]
void dns_tag_new(void);
uint8_t arp_need_limit(void); //boolean
uint8_t arp_need(void); //boolean

//uint8_t delete_udp_conn(uint16_t lport, uint16_t rport);
void display_udp_conn(char *head); //DBG

void display_arp(char *head); //DBG

typedef enum { PF_MODE_CHAR = 0, PF_MODE_HEX } pfmode; //TRUE

void display_str(char *buf, size_t tot_len);
void display_data(pfmode pfm, char *buf, size_t len);
void display_str_state(int sta, char *loc, char *head);
void debug_data(char *head, char *buf);
char *trans_strtok(char *tmpstr, char *pattern);
//uint8_t dec_ok(char c);
uint8_t hex_a_f_ok(char c);
uint8_t trans_mac_ok(char *tmpstr);
uint8_t srvname_valid_ok(char *buf);

uint16_t dm_uip_port(char *tmpstr);
void uart_clean_pf(const char *head, char *buf, size_t len);
void uart_data_pf(const char *head, pfmode pfm, char *buf, size_t len);

void atoi_connect_ip(char *tmpstr, uip_ipaddr_t *raddr, uint16_t *rport);

/* MH2203 版不使用 USART2 RX DMA (見 trans_level.c 說明)，
 * USART2_IRQ_Routine1()/RXDMA_IRQ_Routine1() 未移植，RX 改由 at_port.c 的
 * USART2_IRQHandler 以簡單輪詢方式處理。 */

//uint8_t InvalidBaud(void);
//void trans_rst(void);

#endif //__TRANS_LEVEL_H__
