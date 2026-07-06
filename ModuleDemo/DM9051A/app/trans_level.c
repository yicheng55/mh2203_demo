#include "includes.h"
#include "uip_arp.h"
#include "etherbridge.h"
#include "dataflash.h"
#include <ctype.h>

#define IPBUF ((struct ethip_hdr *)&uip_buf[0])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

struct arp_entry {
  u16_t ipaddr[2];
  struct uip_eth_addr ethaddr;
  u8_t time;
};

static struct arp_entry arp_table[UIP_ARPTAB_SIZE];
static u8_t arptime;

uint8_t arp_count;

const char *ROLEMODE_STR[] = {
	"TCPLPORT", "TCP_Server",
	"TCPDCONN", "TCP_Dynamic_Client",
	"TCPSCONN", "TCP_Sataic_Client",
	"UDPLPORT", "UDP_Server",
	"UDPDCONN", "UDP_Dynamic_Client",
	"UDPSCONN", "UDP_Static_Client",
};

void atcmd_resp_boot(void) {
	atcmd_baseline();
	sprintf(at_tmpstr, "ROLE %u System Boot...", at_type.role);
	atcmd_resp_cmd(at_tmpstr);
}

void print_resp_boot(void) {
	printf("\r\nROLE %u System Boot...\r\n\r\n", at_type.role);
}

void atcmd_resp_dhcpc(void) {
	atcmd_resp_cmd("DHCPC Start...");
}

void atcmd_resp_dnsc(void) {
	atcmd_resp_cmd("DNSC Start...");
}

void print_resp_dnsc(void) {
	printf("DNSC Start...\r\n");
}

void atcmd_resp_establish(void) {
	atcmd_resp_cmd("Bridge connection established...");
	printf("Bridge connection established...\r\n");
}

void atcmd_resp_atcmd_ready(void) {
	atcmd_resp_cmd("AT-CMD ready");
}

void atcmd_resp_trans_ready(uint8_t id) {
	atcmd_resp_cmd("Trans Mode ready");
}

void atcmd_resp_trans_wait(uint8_t id) {
	atcmd_resp_cmd("Trans-in ready");
}

void print_trans_ready(void) {
	printf("Trans Mode ready\r\n");
}

void atcmd_resp_rst(void) {
	atcmd_resp_cmd("Send 'RST' to reset MCU");
}

void atcmd_resp_note(void) {
	atcmd_resp_cmd("[Note]");
}

void atcmd_tcpip_role_err(uint8_t id, uint8_t role)
{
	sprintf(at_tmpstr, "Err, Should in Role = %u", role);
	atcmd_resp_cmd(at_tmpstr);
}

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

void atcmd_resp_listen(char *head, uint16_t rport)
{
	sprintf(at_tmpstr, "%s listen port= %d", head, rport);
	atcmd_resp_cmd(at_tmpstr);
}

void atcmd_comp_listen_port(char *head, uip_ipaddr_t ipaddr, uint16_t rport)
{
	sprintf(at_tmpstr, "%s listen port= %d.%d.%d.%d:%d", head,
		uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
		uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr), rport);
}

void atcmd_resp_remote(char *head, uint8_t role, uip_ipaddr_t ip_raddr, uint16_t rport)
{
	sprintf(at_tmpstr, "%s Mode %s :Remote ip %d.%d.%d.%d:port= %d",
		head, role_config_string(role),
		ip_raddr[0] & 0xff, ip_raddr[0] >> 8,
		ip_raddr[1] & 0xff, ip_raddr[1] >> 8,
		rport);
	atcmd_resp_cmd(at_tmpstr);
}

void atcmd_resp_configured(char *head, uint8_t role, uint16_t lport)
{
	uip_ipaddr_t ipaddr;
	uip_gethostaddr(ipaddr);
	sprintf(at_tmpstr, "%s Mode %s :Local ip %d.%d.%d.%d:port= %d",
		head, role_config_string(role),
		uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
		uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr),
		lport);
	atcmd_resp_cmd(at_tmpstr);
}

void arp_need_new(void) {
	arp_count = ARP_COUNT;
}

void dns_tag_new(void) {
	at_type.dns_srvname_found = 0;
}

uint8_t arp_need_limit(void) {
	if (arp_count) {
		arp_count--;
		return 1;
	}
	return 0;
}

uint8_t arp_need(void)
{
	u16_t ipaddr[2];
	struct arp_entry *tabptr;
	int i;

	do {
		uip_ipaddr_t ipaddr_data;
		uip_ipaddr(ipaddr_data, at_type.udp_raddr[0], at_type.udp_raddr[0] >> 8,
			at_type.udp_raddr[1], at_type.udp_raddr[1] >> 8);
		uip_ipaddr_copy(UDPBUF->destipaddr, ipaddr_data);
	} while(0);

	if(!uip_ipaddr_maskcmp(IPBUF->destipaddr, uip_hostaddr, uip_netmask)) {
		uip_ipaddr_copy(ipaddr, uip_draddr);
	} else {
		uip_ipaddr_copy(ipaddr, IPBUF->destipaddr);
	}

	for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
		tabptr = &arp_table[i];
		if(uip_ipaddr_cmp(ipaddr, tabptr->ipaddr)) {
			break;
		}
	}

	if(i == UIP_ARPTAB_SIZE) {
		return 1;
	}
	return 0;
}

void display_data(pfmode pfm, char *buf, size_t len) {
	int i;
	if (!len) return;
	printf("=");
	for (i = 0; i < len; i++) {
		if (i && !(i % 8)) printf(" ");
		if (pfm == PF_MODE_CHAR)
			printf("%c", buf[i]);
		else
			printf("%02x ", buf[i]);
	}
	printf("\r\n");
}

void display_str_state(int sta, char *loc, char *head) {
	static size_t endloc;
	if (!loc) return;
	if (sta)
		endloc = (size_t)loc + (size_t)gt_u32comRbytes;
	printf("%08x %s[%u] ", loc, head, endloc - (size_t)loc);
	display_data(PF_MODE_HEX, loc, endloc - (size_t)loc);
}

char *trans_strtok(char *tmpstr, char *pattern) {
	char *newstr = strtok(NULL, pattern);
	if (newstr[0] == 0x0d || newstr[0] == 0x0a || newstr[0] == 0x20)
		return NULL;
	return newstr;
}

static uint8_t madd_ok(char c) {
	if (isdigit(c)) return 1;
	if (isalpha(c)) return 1;
	return 0;
}

uint8_t trans_mac_ok(char *tmpstr) {
	if (!madd_ok(*tmpstr++)) return 0;
	if (!madd_ok(*tmpstr++)) return 0;
	if (*tmpstr != 0) return 0;
	return 1;
}

uint16_t dm_uip_port(char *tmpstr) {
	if(atoi(tmpstr) > 65535) {
		atcmd_resp_cmd("Port over 65536, set to 65535");
		return 65535;
	}
	return atoi(tmpstr);
}

#ifdef TL_FLASH_NOT_WRITE_DBG
void display_flash_data(uint32_t addr, uint32_t data)
{
	printf(".flash %x, %2u, %8x\r\n", addr, (addr - 0x0801F800) >> 2, data);
}
#endif

void display_role_tx(void)
{
#ifdef TL_ROLE_TX_DBG
	static uint8_t txc = 0;
	if ((at_type.role == ROLE_TCP_SERVER) || (at_type.role == ROLE_UDP_SCLIENT)) {
		if (txc < 8) {
			txc++;
			switch(at_type.role) {
				case 3:
				case 4:
				case 5:
					break;
			}
		}
	}
#endif
}

void display_udp_conn(char *head)
{
#ifdef UPDATE_UDP_CONN_DBG
	uint8_t c, n = 0;
	for(c = 0; c < UIP_UDP_CONNS; ++c) {
		if (uip_udp_conns[c].lport) {
			n++;
		}
		printf("%s.x.%u [udp conn lport %4u rport %4u]\r\n", head, UIP_UDP_CONNS,
			HTONS(uip_udp_conns[c].lport), HTONS(uip_udp_conns[c].rport));
	}
	printf("%s.x.%u [has %u udp conn]\r\n", head, UIP_UDP_CONNS, n);
#endif
}

void display_arp(char *head)
{
	printf("%s:\r\n", head);
}

uint8_t srvname_valid_ok(char *buf) {
	int n = strlen(buf);
	do {
		if (!(isdigit(*buf) || isalpha(*buf) ||
			*buf == '/' || *buf == '.' || *buf == '+' ||
			*buf == ':' || *buf == '#' || *buf == '?' || *buf == '@' || *buf == '-'))
			return 0;
		buf++;
	} while (*buf && --n);
	return 1;
}

void tcp_init(void)
{
	uint8_t c;
	for(c = 0; c < UIP_CONNS; ++c) {
		uip_conns[c].lport = 0;
		uip_conns[c].tcpstateflags = UIP_CLOSED;
	}
}

void uip_arp_update(u16_t *ipaddr, struct uip_eth_addr *ethaddr)
{
	register struct arp_entry *tabptr;
	int k = 0;
	uint16_t tmpage = 0;
	int c = 0;

	for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
		tabptr = &arp_table[k];
		if(tabptr->ipaddr[0] != 0 && tabptr->ipaddr[1] != 0) {
			if(ipaddr[0] == tabptr->ipaddr[0] && ipaddr[1] == tabptr->ipaddr[1]) {
				memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
				tabptr->time = arptime;
				return;
			}
		}
	}

	for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
		tabptr = &arp_table[k];
		if(tabptr->ipaddr[0] == 0 && tabptr->ipaddr[1] == 0)
			break;
	}

	if(k == UIP_ARPTAB_SIZE) {
		tmpage = 0;
		c = 0;
		for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
			tabptr = &arp_table[k];
			if(arptime - tabptr->time > tmpage) {
				tmpage = arptime - tabptr->time;
				c = k;
			}
		}
		k = c;
		tabptr = &arp_table[k];
	}

	memcpy(tabptr->ipaddr, ipaddr, 4);
	memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
	tabptr->time = arptime;
	arp_need_new();
}

uint8_t eth_netif_linkup = 0;
