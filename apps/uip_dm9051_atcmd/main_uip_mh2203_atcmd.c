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

static struct uip_ethernetif *g_eth;
static dm9051_netif_device_t g_netif;

/* MAC/IP/mask/gw 一律取自 AT_Command 設定 (at_type / eeprom_type)，
 * 由 at_command_settings_init() -> Read_AT_DataFlash() 在呼叫本函式前載入完成，
 * 這樣 "+ROLE"/"+ip="/"+MAC Address" 顯示的設定才會與實際上線的網卡設定一致。 */
static void dm9051_atcmd_netif_config(dm9051_netif_device_t *dev)
{
    dev->mac_addr = eeprom_type.macaddr;

    dev->static_ip[0] = uip_ipaddr1(at_type.hostip);
    dev->static_ip[1] = uip_ipaddr2(at_type.hostip);
    dev->static_ip[2] = uip_ipaddr3(at_type.hostip);
    dev->static_ip[3] = uip_ipaddr4(at_type.hostip);

    dev->gateway_ip[0] = uip_ipaddr1(at_type.hostgw);
    dev->gateway_ip[1] = uip_ipaddr2(at_type.hostgw);
    dev->gateway_ip[2] = uip_ipaddr3(at_type.hostgw);
    dev->gateway_ip[3] = uip_ipaddr4(at_type.hostgw);

    dev->netmask_ip[0] = uip_ipaddr1(at_type.hostmask);
    dev->netmask_ip[1] = uip_ipaddr2(at_type.hostmask);
    dev->netmask_ip[2] = uip_ipaddr3(at_type.hostmask);
    dev->netmask_ip[3] = uip_ipaddr4(at_type.hostmask);
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

    status = dm9051_uip_mh2203_demo_open(eeprom_type.macaddr);
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

    /* 搬自 AT32 SDK 的 EthernetInit_Done()：原本兩處呼叫點都被包在 #if 0，
     * 導致 EthernetInitDoneFlag 永遠是 0，bridge_init() 的角色分派
     * (ROLE 3 -> atcmd_tcpip_udplport() 建立 UDP Server 監聽) 永遠不會執行。
     * 必須放在 udp_printf_init() 之前，因為它內部的 udp_init() 會清空
     * 整個 uip_udp_conns[]，晚呼叫會把除錯用 UDP 連線也清掉。 */
    EthernetInit_Done();

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
                udp_printf_set_output_mode(1);
            }
#endif
            prev_link = link_up;
        }

        dm9051_uip_stack_poll();

        bridge_init();
        at_cmdProcess();

#if UIP_UDP
        struct uip_udp_conn *bridge_udp_conn;

        bridge_udp_conn = udp_bridge_get_pending_tx_conn();
        if (bridge_udp_conn != NULL) {
            dm9051_uip_stack_udp_poke(bridge_udp_conn);
        }

        if (udp_printf_has_pending()) {
            dm9051_uip_stack_udp_poke(udp_printf_get_conn());
        }
#endif
    }
}
