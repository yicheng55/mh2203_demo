/****************************************
 * etherbridge.h
 *
 ****************************************/
#ifndef __ETH_BRIDGE_H__
#define __ETH_BRIDGE_H__

#include "atcommand.h"
#include "uip_arp.h"
//#include "connopts.h"

/* AT32 原始 uip.h 曾定義 struct ethip_hdr 供 IPBUF 巨集轉型使用；目的專案的
 * middlewares/3rd_party/uip 沒有這個型別 (uip_arp.c 內的 arp_table 也是 static
 * 檔案內部變數，不對外匯出)，這裡用相容的欄位/型別補一份，不改動共用的
 * uip_arp.h / uip_arp.c。 */
struct ethip_hdr {
  struct uip_eth_hdr ethhdr;
  u8_t vhl, tos, len[2], ipid[2], ipoffset[2], ttl, proto;
  u16_t ipchksum;
  u16_t srcipaddr[2], destipaddr[2];
};

typedef enum {BUF_NO0 = 0, BUF_NO1 = 1}BUF_NO;

extern BUF_NO Free_Buf_Now; 
extern BUF_NO Full_Buf_Now;
extern boolean Buf_Ok; 
extern char reconn_addr[64];

//-----------------------------------------------------------------------
//void udp_bridge_discovery_arp(void);

void bridge_init(void);
void tcp_bridge_appcall(void);
void udp_bridge_appcall(void);
void senddata(const void *data, int len);
void recv_cmd_auto_disconnect(uint8_t state);

uint8_t check_udp_conn(char *head);
void delete_udp_r_conn(uint16_t rport);

#endif		//__ETH_BRIDGE_H__
