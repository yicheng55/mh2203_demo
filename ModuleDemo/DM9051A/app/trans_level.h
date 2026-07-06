#ifndef __TRANS_LEVEL_H__
#define __TRANS_LEVEL_H__

#include "uip.h"

#define ARP_COUNT				2

#define AT_TMPSTR_LEN			86
#define AT_ROLESTR_LEN			22

#define UPDATE_TCP
#define UPDATE_UDP

#define UPDATE_UDP_CONN_DBG

#define UPDATE_UDP_CONNECTED	0x01
#define UPDATE_UDP_RDY			0x02
#define UPDATE_TCP_CONNECTED	0x10
#define UPDATE_TCP_RDY			0x20

#define UPDATE_UDP_NULL			0x00
#define UPDATE_UDP_SEND			(UPDATE_UDP_CONNECTED | UPDATE_UDP_RDY)
#define UPDATE_TCP_NULL			0x00
#define UPDATE_TCP_SEND			(UPDATE_TCP_CONNECTED | UPDATE_TCP_RDY)

#define TL_FLASH_NOT_WRITE_DBG

extern const char *TRANSLEVEL_STR[];

#ifdef TL_FLASH_NOT_WRITE_DBG
  void display_flash_data(uint32_t addr, uint32_t data);
#else
  #define display_flash_data(addr, data)
#endif

#define TL_ROLE_TX_DBG

void display_role_tx(void);

void atcmd_resp_boot(void);
void print_resp_boot(void);
void atcmd_resp_dhcpc(void);
void atcmd_resp_dnsc(void);
void print_resp_dnsc(void);
void atcmd_resp_establish(void);
void atcmd_resp_atcmd_ready(void);
void atcmd_resp_trans_ready(uint8_t id);
void atcmd_resp_trans_wait(uint8_t id);
void print_trans_ready(void);
void atcmd_ready(void);
void atcmd_resp_rst(void);
void atcmd_resp_note(void);
void atcmd_tcpip_role_err(uint8_t id, uint8_t role);
#define atcmd_tcpip_role_err7(role) atcmd_tcpip_role_err(7, role)
#define atcmd_tcpip_role_err8(role) atcmd_tcpip_role_err(8, role)

const char *role_config_string(uint8_t role);
const char *role_desc_string(uint8_t role);
const char *rolemode_string(uint8_t b);

void atcmd_resp_role_start(uint8_t role);

void atcmd_resp_listen(char *head, uint16_t rport);
void atcmd_comp_listen_port(char *head, uip_ipaddr_t ipaddr, uint16_t rport);
void atcmd_resp_remote(char *head, uint8_t role, uip_ipaddr_t ip_raddr, uint16_t rport);
void atcmd_resp_configured(char *head, uint8_t role, uint16_t lport);

void arp_need_new(void);
void dns_tag_new(void);
uint8_t arp_need_limit(void);
uint8_t arp_need(void);

void display_udp_conn(char *head);
void display_arp(char *head);

typedef enum { PF_MODE_CHAR = 0, PF_MODE_HEX } pfmode;

void display_str(char *buf, size_t tot_len);
void display_data(pfmode pfm, char *buf, size_t len);
void display_str_state(int sta, char *loc, char *head);
void debug_data(char *head, char *buf);
char *trans_strtok(char *tmpstr, char *pattern);
uint8_t hex_a_f_ok(char c);
uint8_t trans_mac_ok(char *tmpstr);
uint8_t srvname_valid_ok(char *buf);

uint16_t dm_uip_port(char *tmpstr);
void uart_clean_pf(const char *head, char *buf, size_t len);
void uart_data_pf(const char *head, pfmode pfm, char *buf, size_t len);

void atoi_connect_ip(char *tmpstr, uip_ipaddr_t *raddr, uint16_t *rport);

#include "uip_arp.h"
struct ethip_hdr {
  struct uip_eth_hdr ethhdr;
  u8_t vhl, tos, len[2], ipid[2], ipoffset[2], ttl, proto;
  u16_t ipchksum;
  u16_t srcipaddr[2], destipaddr[2];
};

#define UPDATE_USART2_RX_DMA
void USART2_IRQ_Routine1(void);
void RXDMA_IRQ_Routine1(void);

#endif
