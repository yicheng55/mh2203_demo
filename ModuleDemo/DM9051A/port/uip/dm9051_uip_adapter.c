/**
 * @file  dm9051_uip_adapter.c
 * @brief DM9051 uIP network stack adapter — implementation.
 *
 * This file implements:
 *   - dm9051_netif_open()       : device init + uIP IP/MAC configuration
 *   - dm9051_netif_input()      : thin wrapper over dm9051_rx()
 *   - dm9051_netif_output()     : thin wrapper over dm9051_tx()
 *   - dm9051_netif_target_mode(): compile-time target mode string
 *   - dm9051_uip_adapter_init() : convenience init for uIP projects
 *   - dm9051_uip_adapter_poll() : one-iteration uIP main-loop helper
 *
 * DESIGN RULE: this file must NOT include hal_mh2030a.h or any platform SPI
 * header.  It interacts with the DM9051 only through dm9051.h (core driver).
 */

#include "dm9051_uip_adapter.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"

/* The DM9051 core driver API — the only DM9051 header this file needs. */
#include "core/dm9051.h"

#include <string.h>
#include <stdio.h>

/* ---- compile-time mode ------------------------------------------------- */

const char *dm9051_netif_target_mode(void)
{
#if defined(DMPLUG_INT)
    return "interrupt";
#elif defined(MH2030A_DM9051_SPI_DMA)
    return "spi dma";
#else
    return "polling";
#endif
}

/* ---- adapter-private state --------------------------------------------- */

static struct timer s_periodic_timer;
static struct timer s_arp_timer;

#define RX_BURST_MAX 8u

/* ---- dm9051_netif interface -------------------------------------------- */

int dm9051_netif_open(const dm9051_netif_device_t *dev)
{
    const uint8_t *mac;
    struct uip_eth_addr ethaddr;
    uip_ipaddr_t ipaddr;

    if (dev == 0) {
        return -1;
    }

    uip_init();
    uip_arp_init();

    /* Initialise DM9051 hardware (SPI, reset, PHY power-on). */
    mac = dm9051_init(dev->mac_addr);
    if (mac == 0) {
        printf("[dm9051_uip] init failed: no MAC\r\n");
        return -1;
    }

    memcpy(ethaddr.addr, mac, sizeof(ethaddr.addr));
    uip_setethaddr(ethaddr);

    uip_ipaddr(ipaddr,
               dev->static_ip[0], dev->static_ip[1],
               dev->static_ip[2], dev->static_ip[3]);
    uip_sethostaddr(ipaddr);

    uip_ipaddr(ipaddr,
               dev->gateway_ip[0], dev->gateway_ip[1],
               dev->gateway_ip[2], dev->gateway_ip[3]);
    uip_setdraddr(ipaddr);

    uip_ipaddr(ipaddr,
               dev->netmask_ip[0], dev->netmask_ip[1],
               dev->netmask_ip[2], dev->netmask_ip[3]);
    uip_setnetmask(ipaddr);

    timer_set(&s_periodic_timer, CLOCK_SECOND / 2);
    timer_set(&s_arp_timer, CLOCK_SECOND * 10);

    printf("[dm9051_uip] target mode : %s\r\n", dm9051_netif_target_mode());
    printf("[dm9051_uip] MAC  %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           uip_ethaddr.addr[0], uip_ethaddr.addr[1], uip_ethaddr.addr[2],
           uip_ethaddr.addr[3], uip_ethaddr.addr[4], uip_ethaddr.addr[5]);
    printf("[dm9051_uip] IP   %u.%u.%u.%u\r\n",
           dev->static_ip[0], dev->static_ip[1],
           dev->static_ip[2], dev->static_ip[3]);

    return 0;
}

uint16_t dm9051_netif_input(uint8_t *buf, uint16_t buf_len)
{
    return dm9051_rx(buf, buf_len);
}

int dm9051_netif_output(const uint8_t *buf, uint16_t len)
{
    /* uip_buf is shared for TX too; the cast removes const as expected by
     * dm9051_tx which does not modify the buffer. */
    dm9051_tx((uint8_t *)(uintptr_t)buf, len);
    return 0;
}

/* ---- convenience helpers for uIP projects ------------------------------ */

int dm9051_uip_adapter_init(const dm9051_netif_device_t *dev)
{
    int rc = dm9051_conf();   /* configure SPI + optional IRQ */
    (void)rc;
    return dm9051_netif_open(dev);
}

void dm9051_uip_adapter_poll(void)
{
    int i;
    uint8_t rx_burst;
    static uint8_t rx_drain_pending = 0u;

    /* ---- RX phase -------------------------------------------------------- */
    if (rx_drain_pending || dm9051_interrupt_get()) {
        rx_burst = 0u;
        rx_drain_pending = 0u;

        while (rx_burst < RX_BURST_MAX) {
            uip_len = dm9051_netif_input(uip_buf, UIP_BUFSIZE);
            if (uip_len == 0u) {
                break;
            }

            if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_IP)) {
                uip_arp_ipin();
                uip_input();
                if (uip_len > 0) {
                    uip_arp_out();
                    dm9051_netif_output(uip_buf, uip_len);
                }
            } else if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_ARP)) {
                uip_arp_arpin();
                if (uip_len > 0) {
                    dm9051_netif_output(uip_buf, uip_len);
                }
            }
            ++rx_burst;
        }

        if (rx_burst == RX_BURST_MAX) {
            rx_drain_pending = 1u;   /* still more frames pending */
        } else {
            dm9051_interrupt_reset();
        }
    }

    /* ---- Periodic timers ------------------------------------------------- */
    if (timer_expired(&s_periodic_timer)) {
        timer_reset(&s_periodic_timer);
        for (i = 0; i < UIP_CONNS; ++i) {
            uip_periodic(i);
            if (uip_len > 0) {
                uip_arp_out();
                dm9051_netif_output(uip_buf, uip_len);
            }
        }
#if UIP_UDP
        for (i = 0; i < UIP_UDP_CONNS; ++i) {
            uip_udp_periodic(i);
            if (uip_len > 0) {
                uip_arp_out();
                dm9051_netif_output(uip_buf, uip_len);
            }
        }
#endif
    }

    if (timer_expired(&s_arp_timer)) {
        timer_reset(&s_arp_timer);
        uip_arp_timer();
    }
}
