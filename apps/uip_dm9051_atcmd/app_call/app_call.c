/**
 * @file    app_call.c
 * @brief   uIP UIP_APPCALL / UIP_UDP_APPCALL 分派層 — MH2203 AT_Command 版
 *
 * 邏輯搬自 AT32 SDK/User/app_call.c (tcp_appcall/udp_appcall/uip_log/
 * dns_client_init/EthernetInit_Done/resolv_found)，核心不改。
 *
 * 兩個平台缺口說明 (詳見移植計畫第 6a 節)：
 *   - eth_netif_linkup：AT32 原本由 uip_init.c 依 DM9051 暫存器狀態設定；
 *     這裡改由主迴圈依既有 dm9051_uip_link_poll() 的回傳值同步 (見
 *     main_uip_mh2203_atcmd.c)，不搬 uip_init.c 整份 (會跟 dm9051_uip_stack_poll()
 *     的職責重複)。
 *   - tcp_init()/udp_init()：AT32 自己的 uip.c 把標準 uIP 的 uip_init() 拆成
 *     這兩個函式；目的專案的 uip.c 只有合併版 uip_init()，這裡補兩個小
 *     wrapper (欄位語意與目的專案 uip.c 的 uip_init() 內部一致)，不修改
 *     uip.c 本身。
 */
#include <stdio.h>
#include <string.h>

#include "uip.h"
#include "app_call.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "at_port.h"

#if defined(HTTP_SERVER_SUPPORT)
#include "httpd.h"
#endif

uint8_t EthernetInitDoneFlag = 0;
char    dhs_staus_msg[64];
uint8_t eth_netif_linkup = 0;

/* --- tcp_init()/udp_init() 相容 wrapper (見檔頭說明) --- */
void tcp_init(void)
{
    int i;
    for (i = 0; i < UIP_CONNS; ++i) {
        uip_conns[i].lport = 0;
        uip_conns[i].tcpstateflags = UIP_CLOSED;
    }
}

void udp_init(void)
{
    int i;
    for (i = 0; i < UIP_UDP_CONNS; ++i) {
        uip_udp_conns[i].lport = 0;
    }
}

void tcp_appcall(void)
{
    if ((uip_conn->lport == HTONS(at_type.t_lport)) || (uip_conn->rport == HTONS(at_type.tcp_rport))) {
        tcp_bridge_appcall();
    }

    /* Local Port */
    switch (uip_conn->lport) {
#ifdef HTTP_SERVER_SUPPORT
    case HTONS(80):
        httpd_appcall();
        break;
#endif /* HTTP_SERVER_SUPPORT */
    default:
        break;
    }
}

void udp_appcall(void)
{
    if ((uip_udp_conn->lport == HTONS(at_type.u_lport)) || (uip_udp_conn->rport == HTONS(at_type.udp_rport))) {
        udp_bridge_appcall();
    }

    /* UDP Remote Port */
    switch (uip_udp_conn->rport) {
    case HTONS(53):
        resolv_appcall();
        break;
    default:
        break;
    }
    /* UDP Local Port */
    switch (uip_udp_conn->lport) {
#if DHCPC_EN
    case HTONS(67):
        dhcpc_appcall();
        break;
    case HTONS(68):
        dhcpc_appcall();
        break;
#endif /* DHCPC_EN */
    default:
        break;
    }
}

void uip_log(char *m)
{
    printf("uIP log: %s\n", m);
}

void dns_client_init(void)
{
    uip_ipaddr_t ipaddr;

    resolv_init();

    uip_ipaddr(ipaddr, at_type.dns_saddr[0], (at_type.dns_saddr[0] >> 8), at_type.dns_saddr[1], (at_type.dns_saddr[1] >> 8));

    resolv_conf(ipaddr);
    resolv_query(at_type.dns_srvname);
}

void EthernetInit_Done(void)
{
    tcp_init();
    udp_init();

    if ((at_type.dns_mode == 1) && ((at_type.role == ROLE_TCP_DCLIENT) ||
                                     (at_type.role == ROLE_TCP_SCLIENT) ||
                                     (at_type.role == ROLE_UDP_DCLIENT) ||
                                     (at_type.role == ROLE_UDP_SCLIENT))) {
        atcmd_resp_dnsc();
        print_resp_dnsc();
        dns_client_init();
    } else {
        EthernetInitDoneFlag = 1;
    }
}

void resolv_found(char *name, uint16_t *ipaddr)
{
    uint8_t n;

    if (ipaddr == NULL) {
        sprintf(dhs_staus_msg, "Host '%s' not found.", name);
        _printf("%s\r\n", dhs_staus_msg);
        atcmd_resp_cmd(dhs_staus_msg);
        /* 放棄啟動 bridge_init()，維持在 AT 指令模式 */
        EthernetInitDoneFlag = 1;
    } else {
        sprintf(dhs_staus_msg, "'%s' = %d.%d.%d.%d", name,
                htons(ipaddr[0]) >> 8,
                htons(ipaddr[0]) & 0xff,
                htons(ipaddr[1]) >> 8,
                htons(ipaddr[1]) & 0xff);

        at_type.dns_srvname_found = 1;
        at_type.srvname_saddr[0] = ipaddr[0];
        at_type.srvname_saddr[1] = ipaddr[1];

        printf("Found name %s\r\n", dhs_staus_msg);

        EthernetInitDoneFlag = 1;
    }

    n = check_udp_conn("dns.end");
    delete_udp_r_conn(53);
    printf("dns.finish, now %d udp.conn\r\n", check_udp_conn("dns.x"));
    printf("\r\n");
}
