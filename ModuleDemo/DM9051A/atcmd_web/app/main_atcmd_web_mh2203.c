#include "mh2203_board.h"
#include "at_port.h"
#include "atcommand.h"
#include "uip_init.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "etherbridge.h"
#include "httpd.h"
#include "udp_printf.h"

#include <stdio.h>

void atcmd_web_tick_init(void);

static void atcmd_web_platform_init(void)
{
    mh2203_uip_board_init(115200u);
    atcmd_web_tick_init();
}

static void atcmd_web_apply_macaddr(void)
{
    uip_ethaddr.addr[0] = eeprom_type.macaddr[0];
    uip_ethaddr.addr[1] = eeprom_type.macaddr[1];
    uip_ethaddr.addr[2] = eeprom_type.macaddr[2];
    uip_ethaddr.addr[3] = eeprom_type.macaddr[3];
    uip_ethaddr.addr[4] = eeprom_type.macaddr[4];
    uip_ethaddr.addr[5] = eeprom_type.macaddr[5];
}

static void atcmd_web_at_init(void)
{
    uint8_t mflag;

    mflag = Read_AT_DataFlash();
    if (mflag == 2u) {
        atcmd_restore();
    }

    print_resp_boot();
    printf("*** %s ***\r\n", IPSPP_VERSION);

    atp_uart_init(at_type.baudrate,
                  at_type.wordlen,
                  at_type.parity,
                  at_type.stop);

    atcmd_resp_boot();
    atcmd_version();

    /* 開機加入 show 訊息顯示 */
    atcmd_show_sys_msg(0);
    atcmd_show(0);

    arp_need_new();
    dns_tag_new();
    atcmd_web_apply_macaddr();
}

int main(void)
{
    atcmd_web_platform_init();
    atcmd_web_at_init();

    tcpip_init();

#if UIP_UDP
    udp_printf_init();
#endif

#ifdef HTTP_SERVER_SUPPORT
    httpd_init();
#endif

    while (1) {
        bridge_init();
        tcpip_process();

#if UIP_UDP
        udp_printf_set_link(eth_netif_linkup != 0u);
        if (udp_printf_has_pending()) {
            uip_udp_periodic_conn(udp_printf_get_conn());
            if (uip_len > 0u) {
                uip_arp_out();
                tapdev_send();
            }
        }
#endif

        at_cmdProcess();
    }
}
