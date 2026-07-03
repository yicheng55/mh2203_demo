/*
 * MH2203 DM9051 uIP + AT_Command 整合進入點。
 *
 * 流程比照黃金樣板 dm9051_driver/examples/uip_mh2203_demo/main_uip_mh2203_demo.c：
 *   platform_init  -> board (clock/delay/uart) + SysTick tick
 *   network_init   -> demo_open (HAL bind + core_open) -> uIP stack_init -> httpd_init
 *   main loop      -> link_poll + stack_poll
 *
 * 在此之上比照 AT32 原韌體 SDK/User/main.c 的主迴圈順序，加入 AT_Command 三件事：
 *   開機讀 Data Flash 設定 -> atp_uart_init 套用 -> 每圈 bridge_init() + at_cmdProcess()。
 */

#include "mh2203_board.h"
#include "mh2203_uip_clock.h"
#include "dm9051_uip_mh2203_demo.h"

#include "dm9051_core.h"
#include "dm9051_uip.h"
#include "dm9051_uip_stack.h"
#include "uip.h"

#include "app_call.h"
#include "at_port.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "udp_printf.h"

#include <stdio.h>

/* TODO: 待使用者提供正式靜態 IP/MAC；暫沿用 uip_mh2203_demo 佔位值，
 * 與既有 DM9051A_mh2203_uip.uvprojx 的預設值一致，方便先驗證連線。 */
#define DM9051_UIP_IP0    192u
#define DM9051_UIP_IP1    168u
#define DM9051_UIP_IP2    249u
#define DM9051_UIP_IP3    37u

#define DM9051_UIP_GW0    192u
#define DM9051_UIP_GW1    168u
#define DM9051_UIP_GW2    249u
#define DM9051_UIP_GW3    1u

#define DM9051_UIP_MASK0  255u
#define DM9051_UIP_MASK1  255u
#define DM9051_UIP_MASK2  255u
#define DM9051_UIP_MASK3  0u

static const uint8_t dm9051_atcmd_mac[DM9051_MAC_ADDR_LENGTH] = {
    0x00u, 0x60u, 0x6Eu, 0x90u, 0x51u, 0x01u
};

static struct uip_ethernetif *g_eth;
static dm9051_netif_device_t g_netif;

static void dm9051_atcmd_netif_config(dm9051_netif_device_t *dev)
{
    dev->mac_addr = dm9051_atcmd_mac;

    dev->static_ip[0] = DM9051_UIP_IP0;
    dev->static_ip[1] = DM9051_UIP_IP1;
    dev->static_ip[2] = DM9051_UIP_IP2;
    dev->static_ip[3] = DM9051_UIP_IP3;

    dev->gateway_ip[0] = DM9051_UIP_GW0;
    dev->gateway_ip[1] = DM9051_UIP_GW1;
    dev->gateway_ip[2] = DM9051_UIP_GW2;
    dev->gateway_ip[3] = DM9051_UIP_GW3;

    dev->netmask_ip[0] = DM9051_UIP_MASK0;
    dev->netmask_ip[1] = DM9051_UIP_MASK1;
    dev->netmask_ip[2] = DM9051_UIP_MASK2;
    dev->netmask_ip[3] = DM9051_UIP_MASK3;
}

static void platform_init(void)
{
    mh2203_uip_board_init(115200u);
    mh2203_uip_tick_init();
}

static void at_command_settings_init(void)
{
    uint8_t mflag;

    mflag = Read_AT_DataFlash();
    if (mflag == 2u) { /* Data Flash 是空的，寫入預設值 */
        atcmd_restore();
    }

    atp_uart_init(at_type.baudrate, at_type.wordlen, at_type.parity, at_type.stop);

    printf("[DM9051 uIP+ATCMD] AT settings loaded: role=%u baud=%lu\r\n",
           at_type.role, (unsigned long)at_type.baudrate);
    printf("  ------------------------------------------------------\r\n");
    /* 開機時把目前設定值透過 AT UART 印出，內容與手動下 "SHOW" 指令一致，
     * 讓使用者一開機接上終端機就能看到目前設定，不用再手動下指令。 */
    atcmd_show_sys_msg(0);
    atcmd_show(0);
    printf("  ------------------------------------------------------\r\n");

    /* AT_Command 初始化完成，透過 AT UART (非 debug log) 送出 "ready" 確認訊息，
     * 讓使用者知道 AT 指令介面已經可以收指令了。 */
    atcmd_resp_atcmd_ready();
}

static void network_init(void)
{
    int status;

    status = dm9051_uip_mh2203_demo_open(dm9051_atcmd_mac);
    g_eth = dm9051_uip_mh2203_demo_eth();

    printf("[DM9051 uIP+ATCMD] MH2203 uIP + AT_Command start\r\n");
    printf("[DM9051 uIP+ATCMD] open status=%d found=%d VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n",
           status,
           dm9051_core_device_found(&g_eth->dev),
           dm9051_core_vendor_id(&g_eth->dev),
           dm9051_core_product_id(&g_eth->dev),
           dm9051_core_chip_revision(&g_eth->dev));

    if (status != DM9051_OK) {
        while (1) {
        }
    }

    dm9051_atcmd_netif_config(&g_netif);
    status = dm9051_uip_stack_init(g_eth, &g_netif);
    printf("[DM9051 uIP+ATCMD] stack status=%d\r\n", status);
    if (status != DM9051_OK) {
        while (1) {
        }
    }

#if UIP_UDP
    udp_printf_init();
#endif

#ifdef HTTP_SERVER_SUPPORT
    httpd_init();
    printf("[DM9051 uIP+ATCMD] HTTP server listening on port 80\r\n");
#endif
}

int main(void)
{
    static int prev_link = -1;

    platform_init();
    at_command_settings_init();
    network_init();

    while (1) {
        int link_up = dm9051_uip_link_poll(g_eth);

        eth_netif_linkup = (uint8_t)link_up;

#if UIP_UDP
        udp_printf_set_link(link_up);
#endif

        if (link_up != prev_link) {
            printf("[DM9051 uIP+ATCMD] Link %s\r\n", link_up ? "UP" : "DOWN");
#if UIP_UDP
            if (link_up) {
                dm9051_uip_set_diag_output_udp(1);
            }
#endif
            prev_link = link_up;
        }

        dm9051_uip_stack_poll();

        bridge_init();
        at_cmdProcess();

#if UIP_UDP
        if (udp_printf_has_pending()) {
            dm9051_uip_stack_udp_poke(udp_printf_get_conn());
        }
#endif
    }
}
