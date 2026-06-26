/**
 * @file main_dm9051_lwip_example.c
 * @brief MH2030 + DM9051A + lwIP + lwip_web2403v2_freelw 的 bare-metal 範例架構。
 *
 * 此檔是整合範例，不一定要直接取代現有 USER/main.c。
 * 若要放進 Keil target，請確認 include path 已包含：
 *   - middlewares/3rd_party/lwip-2.1.2/src/include
 *   - apps/lwip_web2403v2_freelw
 *   - ModuleDemo/DM9051A/dm9051_driver/core/inc
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mh20xx.h"
#include "mh2030a_board.h"

#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"

#include "ethernetif.h"
#include "lwip_web2403v2_freelw.h"

static struct netif g_dm9051_netif;

#define DM9051_LINK_POLL_INTERVAL_MS 500U

static void lwip_link_poll_wrapper(struct netif *netif)
{
    static u32_t last_link_poll_ms;
    u32_t now_ms;

    if (netif == NULL) {
        return;
    }

    now_ms = sys_now();
    if ((u32_t)(now_ms - last_link_poll_ms) < DM9051_LINK_POLL_INTERVAL_MS) {
        return;
    }
    last_link_poll_ms = now_ms;

    /* 使用新的適配層 link poll 函式，會自動觸發 link callback */
    ethernetif_link_poll(netif);
}

static void platform_init(void)
{
    /*
     * 放置 MH2030 board-level 初始化，例如：
     *   - system clock
     *   - GPIO
     *   - UART debug console
     *   - SPI pinmux/clock
     *   - SysTick 或 lwIP sys_now() 所需的 millisecond timer
     *
     * DM9051A 晶片本身的初始化由 ethernetif_init() 透過 staged core/HAL 完成。
     */

    mh2030a_uip_board_init(115200U);

    /*
     * lwIP 的 sys_now() 由 middlewares/3rd_party/lwip-2.1.2/port/sys_arch.c
     * 中的 lwip_sys_now 提供；SysTick_Handler() 會在 MH2030A_LWIP_PORT
     * target 下每 1ms 遞增此計數。
     */
    if (SysTick_Config(SystemCoreClock / 1000U) != 0U) {
        while (1) {
            /* SysTick 啟動失敗：可在此閃燈或輸出 debug 訊息。 */
        }
    }
}

static void network_init(void)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
    struct netif *added_netif;
    err_t err;

    /* 依實際產品調整 MAC。請避免多台設備使用相同 MAC。 */
    static const uint8_t mac_addr[6] = {
        0x00U, 0x60U, 0x6EU, 0x11U, 0x22U, 0x33U
    };

    IP4_ADDR(&ipaddr, 192, 168, 249, 37);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 249, 1);

    /*
     * NO_SYS=1 bare-metal 模式使用 lwip_init()。
     * 若改成 RTOS/NO_SYS=0，初始化入口通常會改用 tcpip_init()。
     */
    lwip_init();

    memset(&g_dm9051_netif, 0, sizeof(g_dm9051_netif));
    memcpy(g_dm9051_netif.hwaddr, mac_addr, sizeof(mac_addr));

    /*
     * 這裡 input callback 使用 ethernet_input，因為 ethernetif_input()
     * 交給 lwIP 的是完整 Ethernet frame。
     */
    added_netif = netif_add(&g_dm9051_netif,
                            &ipaddr,
                            &netmask,
                            &gateway,
                            NULL,
                            ethernetif_init,
                            ethernet_input);
    if (added_netif == NULL) {
        while (1) {
            /* netif 建立失敗：可在此閃燈或輸出 debug 訊息。 */
        }
    }

    netif_set_default(&g_dm9051_netif);
    lwip_link_poll_wrapper(&g_dm9051_netif);
    netif_set_up(&g_dm9051_netif);

    printf("[DM9051 lwIP] netif up IP=%u.%u.%u.%u mask=%u.%u.%u.%u gw=%u.%u.%u.%u\r\n",
           ip4_addr1(&ipaddr),
           ip4_addr2(&ipaddr),
           ip4_addr3(&ipaddr),
           ip4_addr4(&ipaddr),
           ip4_addr1(&netmask),
           ip4_addr2(&netmask),
           ip4_addr3(&netmask),
           ip4_addr4(&netmask),
           ip4_addr1(&gateway),
           ip4_addr2(&gateway),
           ip4_addr3(&gateway),
           ip4_addr4(&gateway));

    /*
     * 啟動 Web 應用。此 wrapper 會呼叫 httpd_init_with_netif()，
     * 將 HTTP server 綁定在 DM9051A netif 的 IPv4 位址上。
     */
    err = lwip_web2403v2_freelw_init(&g_dm9051_netif);
    if (err != ERR_OK) {
        while (1) {
            /* Web server 啟動失敗：可在此閃燈或輸出 debug 訊息。 */
        }
    }
}

int main(void)
{
    platform_init();
    network_init();

    while (1) {
        lwip_link_poll_wrapper(&g_dm9051_netif);

        /*
         * 1. 輪詢 DM9051A RX，收到封包後交給 lwIP。
         *    若使用外部中斷，也建議在中斷中只設 flag，
         *    再於主迴圈或工作任務中呼叫此函式。
         */
        ethernetif_input(&g_dm9051_netif);

        /*
         * 2. 推進 lwIP 內部 timer：
         *    TCP retransmission、ARP timer、DHCP timer 等都靠它處理。
         */
#if LWIP_TIMERS
        sys_check_timeouts();
#endif

        /*
         * 3. 應用層輪詢 hook。
         *    目前 HTTPD raw API 不需要額外 poll，但保留此呼叫可讓架構固定。
         */
        lwip_web2403v2_freelw_poll();
    }
}
