#include "mh2030a_board.h"
#include "mh2030a_uip_clock.h"
#include "dm9051_uip_mh2030a_smoke.h"

#include "../../adapters/uip/dm9051_uip.h"
#include "../../adapters/uip/dm9051_uip_stack.h"
#include "app_call.h"

#include <stdio.h>

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

static const uint8_t dm9051_demo_mac[DM9051_MAC_ADDR_LENGTH] = {
    0x00u, 0x60u, 0x6Eu, 0x90u, 0x51u, 0x01u
};

static struct uip_ethernetif *g_eth;
static dm9051_netif_device_t g_netif;

static void dm9051_demo_netif_config(dm9051_netif_device_t *dev)
{
    dev->mac_addr = dm9051_demo_mac;

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
    mh2030a_uip_board_init(115200);
    mh2030a_uip_tick_init();
}

static void network_init(void)
{
    int status;

    status = dm9051_uip_mh2030a_smoke_open(dm9051_demo_mac);
    g_eth = dm9051_uip_mh2030a_smoke_eth();

    printf("[DM9051 uIP] MH2030A staged uIP demo start\r\n");
    printf("[DM9051 uIP] open status=%d found=%d VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n",
           status,
           dm9051_core_device_found(&g_eth->dev),
           dm9051_core_vendor_id(&g_eth->dev),
           dm9051_core_product_id(&g_eth->dev),
           dm9051_core_chip_revision(&g_eth->dev));

    if (status != DM9051_OK) {
        while (1) {
        }
    }

    dm9051_demo_netif_config(&g_netif);
    status = dm9051_uip_stack_init(g_eth, &g_netif);
    printf("[DM9051 uIP] stack status=%d\r\n", status);
    if (status != DM9051_OK) {
        while (1) {
        }
    }

#if WEB_EN
    httpd_init();
    printf("[DM9051 uIP] HTTP server listening on port 80\r\n");
#endif
}

int main(void)
{
    static int prev_link = -1;

    platform_init();
    network_init();

    while (1) {
        int link_up = dm9051_uip_link_poll(g_eth);

        if (link_up != prev_link) {
            printf("[DM9051 uIP] Link %s\r\n",
                   link_up ? "UP" : "DOWN");
            prev_link = link_up;
        }

        dm9051_uip_stack_poll();
    }
}
