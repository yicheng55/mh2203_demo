#include "netconf_mh2030a.h"
#include "dm9051_uip_adapter.h"
#include "uip.h"
#include <stdio.h>

volatile uint32_t uip_elapsed_ms = 0;
uint32_t g_RunTime = 0;
volatile uint32_t all_local_time = 0;

const char *mh2030a_uip_target_mode(void)
{
    return dm9051_netif_target_mode();
}

void uip_log(char *msg)
{
    printf("[uIP] %s\r\n", msg);
}

void resolv_found(char *name, u16_t *ipaddr)
{
    if (ipaddr == NULL) {
        printf("[uIP DNS] %s not found\r\n", name);
        return;
    }

    printf("[uIP DNS] %s resolved to %u.%u.%u.%u\r\n",
           name,
           (unsigned int)(ntohs(ipaddr[0]) >> 8),
           (unsigned int)(ntohs(ipaddr[0]) & 0xffu),
           (unsigned int)(ntohs(ipaddr[1]) >> 8),
           (unsigned int)(ntohs(ipaddr[1]) & 0xffu));
}

// Moved to mh2030a_uip_clock.c to avoid duplicate definition
 void mh2030a_uip_update_time(void)
 {
     uip_elapsed_ms += MH2030A_UIP_TICK_MS;
 //    g_RunTime += MH2030A_UIP_TICK_MS;
 //    all_local_time += MH2030A_UIP_TICK_MS;
 }

void mh2030a_uip_net_init(void)
{
    dm9051_netif_device_t dev;
    int rc;

    printf("[MH2030A uIP] network init\r\n");
    dev.mac_addr = 0;
    dev.static_ip[0] = MH2030A_UIP_STATIC_IP0;
    dev.static_ip[1] = MH2030A_UIP_STATIC_IP1;
    dev.static_ip[2] = MH2030A_UIP_STATIC_IP2;
    dev.static_ip[3] = MH2030A_UIP_STATIC_IP3;
    dev.gateway_ip[0] = MH2030A_UIP_GW_IP0;
    dev.gateway_ip[1] = MH2030A_UIP_GW_IP1;
    dev.gateway_ip[2] = MH2030A_UIP_GW_IP2;
    dev.gateway_ip[3] = MH2030A_UIP_GW_IP3;
    dev.netmask_ip[0] = MH2030A_UIP_MASK_IP0;
    dev.netmask_ip[1] = MH2030A_UIP_MASK_IP1;
    dev.netmask_ip[2] = MH2030A_UIP_MASK_IP2;
    dev.netmask_ip[3] = MH2030A_UIP_MASK_IP3;

    rc = dm9051_uip_adapter_init(&dev);
    if (rc != 0) {
        printf("[MH2030A uIP] adapter init failed: %d\r\n", rc);
        while (1) {
        }
    }
}

void mh2030a_uip_net_loop(void)
{
    dm9051_uip_adapter_poll();
}
