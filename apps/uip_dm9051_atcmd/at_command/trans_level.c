/*******************************************
 * trans_level.c
 * Created 20240513 Joseph
 *
 *******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "uip.h"
#include "uip_arp.h"
#include "uipopt.h"
#include "at_port.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "dataflash.h"

extern struct eeprom_funcation eeprom_show;

#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#define TCPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

const char *ROLEMODE_STR[] = {
	"TCPLPORT", "TCP_Server",
	"TCPDCONN", "TCP_Dynamic_Client",
	"TCPSCONN", "TCP_Sataic_Client",
	"UDPLPORT", "UDP_Server",
	"UDPDCONN", "UDP_Dynamic_Client",
	"UDPSCONN", "UDP_Static_Client",
};

const char *TRANSLEVEL_STR[] = {
	"System Boot...", //0
	"DHCPC Start...", //1
	"AT-CMD ready", //2
	"Trans Mode ready", //3
	"OK", //4
	"Send 'RST' to reset MCU", //5
	"[Note]", //6
	"Should", //7
	"Should not", //8
	"Trans-in ready", //9
	"To be", //10
	"DNSC Start...", //11
	"Bridge connection established...", //12
	"Wait a client...", //13 ("Trans-in listen")("Listen Trans-in")
	"listen port=", //14
//	"UDP Listen Port=", //14
//	"TCP Listen Port=", //15
};

uint8_t arp_count;

//[sys init call]

void atcmd_resp_boot(void) {
	atcmd_baseline();
	sprintf(at_tmpstr, "ROLE %u %s", at_type.role, TRANSLEVEL_STR[0]);
	atcmd_resp_cmd(at_tmpstr);
}

void print_resp_boot(void) {
	_printf("\x0c\0");_printf("\x0c\0"); _printf("\r\n");
	_printf("ROLE %u %s\r\n", at_type.role, TRANSLEVEL_STR[0]); //_printf("System Boot...\r\n\r\n");
#if JOSRANDOM
	_printf("[ee_lastport]= %u\r\n", eeprom_show.ee_lastport);
#endif
	_printf("\r\n");
}

void atcmd_resp_dhcpc(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[1]);
}

void atcmd_resp_dnsc(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[11]);
}

void print_resp_dnsc(void) {
	printf("%s\r\n", TRANSLEVEL_STR[11]);
}

void atcmd_resp_establish(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[12]);
	printf("%s\r\n", TRANSLEVEL_STR[12]);
}

void atcmd_resp_atcmd_ready(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[2]);
}

void atcmd_resp_trans_ready(uint8_t id) {
	atcmd_resp_cmd(TRANSLEVEL_STR[id]); //3 or 9
}

void atcmd_resp_trans_wait(uint8_t id) {
	atcmd_resp_cmd(TRANSLEVEL_STR[id]); //3 or 13
}

void print_trans_ready(void) {
	printf("%s\r\n", TRANSLEVEL_STR[3]);
}

void atcmd_ready(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[4]);
}
void atcmd_resp_rst(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[5]);
}
void atcmd_resp_note(void) {
	atcmd_resp_cmd(TRANSLEVEL_STR[6]);
}

//void atcmd_tcpip_role_err7(uint8_t role)
//{
//		sprintf(at_tmpstr, "Error, %s in Role = %u", TRANSLEVEL_STR[7], role);
//		atcmd_resp_cmd(at_tmpstr);
//}

//void atcmd_tcpip_role_err8(uint8_t role)
//{
//		sprintf(at_tmpstr, "Error, %s in Role = %u", TRANSLEVEL_STR[8], role);
//		atcmd_resp_cmd(at_tmpstr);
//}

void atcmd_tcpip_role_err(uint8_t id, uint8_t role)
{
		sprintf(at_tmpstr, "Err, %s in Role = %u", TRANSLEVEL_STR[id], role);
		atcmd_resp_cmd(at_tmpstr);
}

//void atcmd_tcpip_role_to(uint8_t id, uint8_t role)
//{
//		sprintf(at_tmpstr, "Note, %s Role = %u", TRANSLEVEL_STR[id], role);
//		atcmd_resp_cmd(at_tmpstr);
//}

const char *role_config_string(uint8_t role) {
	return ROLEMODE_STR[(role << 1) | 0];
}

const char *role_desc_string(uint8_t role) {
	return ROLEMODE_STR[(role << 1) | 1];
}

void atcmd_resp_role_start(uint8_t role) {
	sprintf(at_tmpstr, "%s Start...", role_config_string(role));
	atcmd_resp_cmd(at_tmpstr);
	printf("%s\r\n", at_tmpstr);
}

static const char *padstr[] = {
	" ",   //if 3 digi
	"  ",  //if 2 digi
	"   ", //if 1 digi
};

void atcmd_resp_listen(char *head, uint16_t rport)
{
	sprintf(at_tmpstr, "%s %s %d", head, TRANSLEVEL_STR[14], rport);
	atcmd_resp_cmd(at_tmpstr);
}

void atcmd_comp_listen_port(char *head, uip_ipaddr_t ipaddr, uint16_t rport)
{
	sprintf(at_tmpstr, "%s %s %d.%d.%d.%d:%d", head, TRANSLEVEL_STR[14],
						uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), 
						uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr),rport);
}

void atcmd_resp_remote(char *head, uint8_t role, uip_ipaddr_t ip_raddr, uint16_t rport)
{
	int len_ipaddr4 = sprintf(at_tmpstr, "%d", ip_raddr[1] >> 8);
	sprintf(at_tmpstr, "%s Mode %s :Remote ip %d.%d.%d.%d%s:port= %d", head, role_config_string(role),
						ip_raddr[0] & 0xff, ip_raddr[0] >> 8, 
						ip_raddr[1] & 0xff, ip_raddr[1] >> 8, padstr[3 - len_ipaddr4],
						rport);
	atcmd_resp_cmd(at_tmpstr);
}

void atcmd_resp_configured(char *head, uint8_t role, uint16_t lport)
{
	uip_ipaddr_t ipaddr;
	int len_ipaddr4;

	uip_gethostaddr(ipaddr);
	len_ipaddr4 = sprintf(at_tmpstr, "%d", ipaddr[1] >> 8);
	sprintf(at_tmpstr, "%s Mode %s :Local ip  %d.%d.%d.%d%s:port= %d", head, role_config_string(role),
			uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
			uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr), padstr[3 - len_ipaddr4],
			lport);
	atcmd_resp_cmd(at_tmpstr);
#if 0
	uip_getnetmask(ipaddr);
	sprintf(at_tmpstr, "%s Mode %s :net mask  %d.%d.%d.%d", head, role_config_string(role),
			uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
			uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	atcmd_resp_cmd(at_tmpstr);
	
	uip_getdraddr(ipaddr);
	sprintf(at_tmpstr, "%s Mode %s :gateway   %d.%d.%d.%d", head, role_config_string(role),
			uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
			uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	atcmd_resp_cmd(at_tmpstr);
#endif
}

/* Note : This function also require always called by "uip_arp_update"
 */
void arp_need_new(void) {
	arp_count = ARP_COUNT;
}

void dns_tag_new(void) {
	at_type.dns_srvname_found = 0;
}

uint8_t arp_need_limit(void) { //boolean
	if (arp_count) {
		arp_count--;
		return 1;
	}
	return 0; //FALSE
}

//[sys operating call]

//void display_tcp_payload(uint8_t n) {
//#ifdef TL_ROLE_TX_DBG
//	 uint16_t u;
//	uint16_t ppos = (((uint16_t)(TCPBUF->len[0]) << 8) + TCPBUF->len[1]) + UIP_LLH_LEN;
//	for (u = 0; (u < n) && ((u + ppos) < uip_len); u++)
//		printf(" %02x", uip_buf[u + ppos]);
//	printf(" : ");
//	for (u = 0; (u < n) && ((u + ppos) < uip_len); u++)
//		printf(" %c", uip_buf[u + ppos]);
//	printf(" (ppos %u uip_len %u)\r\n", ppos, uip_len);
//#endif
//}

void display_udp_payload(uint8_t n) {
#ifdef TL_ROLE_TX_DBG
#if 0
	//display_tcp_payload(n);
#else
	 uint16_t u;
	uint16_t ppos = UIP_LLH_LEN + sizeof(struct uip_udpip_hdr); //(((uint16_t)(UDPBUF->len[0]) << 8) + UDPBUF->len[1]) + UIP_LLH_LEN;
//	printf(" ");
	for (u = 0; (u < n) && ((u + ppos) < uip_len); u++)
		printf(" %02x", uip_buf[u + ppos]);
	printf(" : ");
	for (u = 0; (u < n) && ((u + ppos) < uip_len); u++)
		printf("%c", uip_buf[u + ppos]);
	printf("\r\n");
//	printf(" (ppos %u uip_len %u)\r\n", ppos, uip_len);
#endif
#endif
}

#ifdef TL_FLASH_NOT_WRITE_DBG
void display_flash_data(uint32_t idx, uint32_t data)
{
  /* 原版以絕對 flash 位址回推 word index；MH2203 走 at_port 的相對 word index，直接印。 */
  printf(".flash idx=%2u, %8x\r\n", idx, data);
}
//void display_flash_data8SrvName(uint32_t addr_s, uint32_t addr, uint32_t data)
//{
//  printf(".HINT flash %x.SrvName8, %2u, %8x\r\n", addr, (addr_s - StartAddress) >> 2, data);
//}
//void display_flash_data8SrvPath(uint32_t addr_s, uint32_t addr, uint32_t data)
//{
//  printf(".HINT flash %x.SrvPath8, %2u, %8x\r\n", addr, (addr_s - StartAddress) >> 2, data);
//}
#endif //TL_FLASH_NOT_WRITE_DBG

void display_role_tx(void)
{
#ifdef TL_ROLE_TX_DBG
  static uint8_t txc = 0;
  if ((at_type.role == ROLE_TCP_SERVER) || (at_type.role == ROLE_UDP_SCLIENT)) {
	 uint8_t u;
	 if(BUF->type == HTONS(UIP_ETHTYPE_ARP)) {
		printf("\r\n%s arp tx\r\n", role_config_string(at_type.role));
		 for (u = 0; u < 16; u++) {
			if (u == 8)
				printf(" ");
			printf(" %02x", uip_buf[u]); 
		 }
		 printf("\r\n");
//		printf("\r\n%s arp tx", rolemode_string(0));
//		 for (u = 0; u < uip_len; u++) {
//			if (!(u % 16))
//				printf("\r\n");
//			if (!(u % 8))
//				printf(" ");
//			printf(" %02x", uip_buf[u]); 
//		 }
//		 printf("\r\n");
	 } else {
		if (txc < 8) {
			txc++;

//			printf("%s to %d.%d.%d.%d tx\r\n", role_config_string(at_type.role),
//				uip_ipaddr1(IPBUF->destipaddr), uip_ipaddr2(IPBUF->destipaddr), 
//				uip_ipaddr3(IPBUF->destipaddr), uip_ipaddr4(IPBUF->destipaddr));  //IPBUF

			switch(at_type.role) {
//				case 0:
//				case 1:
//				case 2:
//					display_tcp_payload(8);
//					break;
				case 3:
				case 4:
				case 5:
					display_udp_payload(8);
					break;
			}
		}
	 } //if.else
  }
#endif
}

/*
 * To: delete_udp_conn(HTONS(DHCPC_CLIENT_PORT), HTONS(DHCPC_SERVER_PORT));
  *    delete_udp_conn(HTONS(at_type.u_lport), HTONS(at_type.udp_rport));
 */
void display_udp_conn(char *head)
{
#ifdef UPDATE_UDP_CONN_DBG
  //such as got newdata from UDP server (or client.)
	uint8_t c, n = 0;
	for(c = 0; c < UIP_UDP_CONNS; ++c) {
		if (uip_udp_conns[c].lport) {
			n++;
		}
			printf("%s.x.%u [udp conn lport %4u rport %4u : %d.%d.%d.%d]\r\n", head, UIP_UDP_CONNS,
				HTONS(uip_udp_conns[c].lport), HTONS(uip_udp_conns[c].rport),
				uip_ipaddr1(uip_udp_conns[c].ripaddr), uip_ipaddr2(uip_udp_conns[c].ripaddr),
				uip_ipaddr3(uip_udp_conns[c].ripaddr), uip_ipaddr4(uip_udp_conns[c].ripaddr));
	}
	printf("%s.x.%u [has %u udp conn]\r\n", head, UIP_UDP_CONNS, n);
#endif
}

void display_arp(char *head)
{
    printf("%s:\r\n", head);
#if 0
	int k, i;
    for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
	  struct arp_entry *tabptr;
      tabptr = &arp_table[k];
	  printf("N.%d tab->ethaddr.addr ", UIP_ARPTAB_SIZE);
	  for (i=0; i<6; i++)
		 printf("%02x ", tabptr->ethaddr.addr[i]);
	  printf("tab->ipaddr ");
	  printf("%d.%d.%d.%d\r\n", uip_ipaddr1(tabptr->ipaddr), uip_ipaddr2(tabptr->ipaddr),
				uip_ipaddr3(tabptr->ipaddr), uip_ipaddr4(tabptr->ipaddr));
    }
#endif
}

uint8_t arp_need(void) //boolean
{
	/* 原版直接走訪 uip_arp.c 內部的 arp_table[] 判斷目的 MAC 是否已知，
	 * 若未知就搶先送 ARP request 再重試，避免白白浪費一次 UDP 直送。
	 * 目的專案的 uip_arp.c 把 arp_table 宣告成檔案內 static，不對外匯出
	 * (見 middlewares/3rd_party/uip/src/uip_arp.c)，不能沿用同樣的直接查表
	 * 方式，也不修改共用的 uip_arp.c 去匯出它。
	 * 簡化為一律回傳 0 (視為已知)：uip_arp_out() 本身遇到未知位址時，
	 * 仍會自動把封包轉成 ARP request 送出 (uIP 標準行為)，只是少了這裡
	 * 原本的搶先重試優化，功能不受影響。 */
	return 0;
}
							
uint16_t dm_uip_port(char *tmpstr) {
	if(atoi(tmpstr) > 65535) {
		atcmd_resp_cmd("Port over 65536, set to 65535");
		return 65535;
	}
	return atoi(tmpstr);
}

/* memset(,0,) with printf the head-string
 */
//void uart_clean_pf(const char *head, char *buf, size_t len) {
//	printf("%s, clean\r\n", head);
//	memset(buf, 0, len);
//}

//void display_str(char *buf, size_t tot_len) {
//	int len = strlen(buf);
//	printf("=");
//	while (tot_len-- && len-- && (buf[0] != 0x0d && buf[0] != 0x0a)) {
//		printf("%c", buf[0]);
//		buf++;
//	}
//	printf(" ");
//}
void display_data(pfmode pfm, char *buf, size_t len) {
		int i;
		if (!len)
			return;
//		if (pfm == PF_MODE_CHAR)
		printf("=");
		for (i=0; i< len; i++) { //g_u32comRbytes
			if (i && !(i%8))
				printf(" ");
			if (pfm == PF_MODE_CHAR)
				printf("%c", buf[i]); //g_u8RecData[i]
			else
				printf("%02x ", buf[i]); //g_u8RecData[i]
		}
		printf("\r\n");
} //display_data(PF_MODE_CHAR, buf, len);

void display_str_state(int sta, char *loc, char *head) {
	static size_t endloc;
	if (!loc)
		return;
	if (sta)
		endloc = (size_t) loc + (size_t) gt_u32comRbytes;
	_printf("%08x %s[%u] ", loc, head, endloc - (size_t) loc); //gt_u32comRbytes
#if 0
	display_str(loc, endloc - (size_t) loc);
#endif
	display_data(PF_MODE_HEX, loc, endloc - (size_t) loc); //gt_u32comRbytes
}

//void debug_data(char *head, char *buf) {
//	printf("%s: ptr %x : ", head, buf);
//	if (buf)
//		_display_data(PF_MODE_HEX, buf, 3);
//	else
//		printf("\r\n");
//}

char *trans_strtok(char *tmpstr, char *pattern) {
	/* NULL instead tmpstr,
	   Need check if tmpstr */
	char *newstr = strtok(NULL, pattern);
	if (newstr[0] == 0x0d || newstr[0] == 0x0a || newstr[0] == 0x20)
		return NULL;
	return newstr;
}

//uint8_t dec_ok(char c) {
//	if (isdigit(c))
//		return 1;
//	return 0;
//}

//uint8_t hex_a_f_ok(char c) {
//	if (c >= 'a' && c <= 'f')
//		return 1;
//	if (c >= 'A' && c <= 'F')
//		return 1;
//	return 0;
//}

uint8_t madd_ok(char c) {
	if (isdigit(c)) //(c >= '0' && c <= '9')
		return 1;
//	if (c >= 'a' && c <= 'f')
//		return 1;
//	if (c >= 'A' && c <= 'F')
//		return 1;
	if (isalpha(c))
		return 1;
	return 0;
}

uint8_t trans_mac_ok(char *tmpstr) {
	if (!madd_ok(*tmpstr++))
		return 0;
	if (!madd_ok(*tmpstr++))
		return 0;
	if (*tmpstr != 0)
		return 0;
	return 1;
}

uint8_t srvname_char(char c) {
	if (isdigit(c)) //(c >= '0' && c <= '9')
		return 1;
//	if (c >= 'a' && c <= 'z')
//		return 1;
//	if (c >= 'A' && c <= 'Z')
//		return 1;
	if (isalpha(c))
		return 1;
	if (c == '/' || c == '.' || c == '+' ||
		c == ':' || c == '#' || c == '?' || c == '@' || c == '-')
		return 1;
	return 0;
}

uint8_t srvname_valid_ok(char *buf) {
	int n = strlen(buf);
	do {
		if (!srvname_char(*buf++))
			return 0;
	} while (*buf && --n);
	return 1;
}

#if 1 //DBG
//void uart_data_pf(const char *head, pfmode pfm, char *buf, size_t len)
//{
//	do {
//		printf("%s[%u] ", head, len); //%s, RecData
//		display_data(pfm, buf, len);
//	} while(0);
//}
#endif

/* USART2 RX DMA ISR (USART2_IRQ_Routine1 / RXDMA_IRQ_Routine1)：
 * 原版靠 AT32 USART/DMA 暫存器與 at32_board_uart.c 的 ISR 呼叫；MH2203 版
 * 的 RX 走簡單輪詢式中斷 (at_port.c 的 USART2_IRQHandler 逐字元填
 * g_u8RecData/gt_u32comRbytes/atcmd_flag)，不需要、也不編譯這兩個 DMA 專屬函式。
 * ATCMD_USART_TX_DMA / ATCMD_UART_RX_DOUB_BUF / UIP_USE_BIGDATA_SIZE 三個巨集
 * 在本專案的 .uvprojx 都不定義，維持這個簡單路徑。 */

//uint8_t InvalidBaud(void)
//{
//	if ((at_type.baudrate == DEF_BAUDRATE || at_type.baudrate == DEF_BAUDRATE) &&
//		(at_type.wordlen == DEF_WORDLEN || at_type.wordlen == DEF_WORDLEN) &&
//		(at_type.parity == DEF_PARITY || at_type.parity == DEF_PARITY) &&
//		(at_type.stop == DEF_STOP || at_type.stop == DEF_STOP))
//		return 0; //invalid
//	return 1;
//}

//void trans_rst(void)
//{
//	sprintf(at_tmpstr, "Invalid Baud Rate= %d %d %d %d", at_type.baudrate, at_type.wordlen ,at_type.parity, at_type.stop);
//	//atcmd_resp_cmd(at_tmpstr);
//	printf("%s\r\n", at_tmpstr);
//	
//	at_type.baudrate = DEF_BAUDRATE;
//	at_type.wordlen = DEF_WORDLEN;
//	at_type.parity =DEF_PARITY;
//	at_type.stop = DEF_STOP;
//	Write_AT_DataFlash();	//only recover baud
//	
//	printf("Restore to %u\r\n", DEF_BAUDRATE);
//	printf("Do 'RST' to reset MCU\r\n");
//	atcmd_rst();
//}
