/*
 * Optional uIP loop for the staged DM9051 adapter.
 *
 * This file is intentionally separate from dm9051_uip.c so the hardware smoke
 * target can keep building without pulling in uIP core objects.
 */

#include "dm9051_uip_stack.h"

#include "../../core/inc/dm9051_core.h"

#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
#include "udp_printf.h"

#include <stdio.h>
#include <string.h>

#define DM9051_UIP_RX_BURST_MAX 8u

#ifndef DM9051_UIP_DIAG
#define DM9051_UIP_DIAG 1
#endif

#if DM9051_UIP_DIAG
/* 診斷輸出走標準 printf()，UART/UDP 切換統一交給 fputc() (見 mh2203_board.c)
 * 依 udp_printf_is_output_mode_enabled() / udp_printf_is_ready() 集中處理，
 * 避免此處與 fputc() 各自維護一套不一致的切換邏輯。 */
#define DM9051_UIP_DIAG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DM9051_UIP_DIAG_PRINTF(...) do { } while (0)
#endif


#ifndef DM9051_UIP_ENABLE_PERIODIC
/*
* Enabling this will cause the stack to call uip_periodic() and uip_udp_periodic()
* at regular intervals, which is necessary for some applications but may cause
* extra CPU load for simple ones. The periodic timers are still driven by the
* application calling dm9051_uip_stack_poll() in either case.
*
* DM9051_UIP_ENABLE_PERIODIC 是控制 dm9051_uip_stack_poll() 是否執行 uIP 週期性處理 的開關。
* 啟用後，dm9051_uip_stack_poll() 會定期呼叫 uip_periodic() 和 uip_udp_periodic()，這對某些應用是必要的，但可能會增加簡單應用的 CPU 負載。
* 無論 DM9051_UIP_ENABLE_PERIODIC 是否啟用，週期性計時器仍然由應用程式呼叫 dm9051_uip_stack_poll() 驅動。
*/
#define DM9051_UIP_ENABLE_PERIODIC 1
#endif

#if DM9051_UIP_ENABLE_PERIODIC
static struct timer dm9051_uip_periodic_timer;
static struct timer dm9051_uip_arp_timer;
static uint8_t dm9051_uip_tcp_periodic_index;
static uint8_t dm9051_uip_udp_periodic_index;
static uint8_t dm9051_uip_tcp_periodic_pending;
static uint8_t dm9051_uip_udp_periodic_pending;
#endif

static void dm9051_uip_stack_send_if_needed(void)
{
    if (uip_len > 0u) {
        uip_arp_out();
        (void)dm9051_uip_output(uip_buf, uip_len);
        /* udp_printf_putchar() 在排隊送出前會鎖定 output_active，
         * 必須在實際幀送出後解鎖，否則第一筆之後全部被吃掉。 */
        udp_printf_output_done();
    }
}

void dm9051_uip_stack_tcp_poke(struct uip_conn *conn)
{
    if (conn == NULL) {
        return;
    }

    uip_poll_conn(conn);
    dm9051_uip_stack_send_if_needed();
}
#if UIP_UDP
void dm9051_uip_stack_udp_poke(struct uip_udp_conn *conn)
{
    if (conn == NULL) {
        return;
    }

    uip_udp_periodic_conn(conn);
    dm9051_uip_stack_send_if_needed();
}
#else
void dm9051_uip_stack_udp_poke(struct uip_udp_conn *conn)
{
    (void)conn;
}
#endif

static int dm9051_uip_stack_drain_rx(void)
{
    uint8_t rx_burst;

    rx_burst = 0u;

    while (rx_burst < DM9051_UIP_RX_BURST_MAX) {
        uip_len = dm9051_uip_input(uip_buf, UIP_BUFSIZE);
        if (uip_len == 0u) {
            break;
        }

        if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_IP)) {
            uip_arp_ipin();
            uip_input();
            dm9051_uip_stack_send_if_needed();
        } else if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_ARP)) {
            uip_arp_arpin();
            if (uip_len > 0u) {
                (void)dm9051_uip_output(uip_buf, uip_len);
            }
        }

        ++rx_burst;
    }

    return rx_burst;
}

static uint8_t dm9051_uip_stack_rx_pending_from_burst(int rx_burst)
{
    return (rx_burst >= DM9051_UIP_RX_BURST_MAX) ? 1u : 0u;
}

static void dm9051_uip_stack_print_rx_burst(const char *reason,
                                            int rx_burst,
                                            uint8_t drain_pending)
{
    static int last_error_status = DM9051_OK;
    int rx_status;

    rx_status = dm9051_uip_last_rx_status();
    if (rx_burst > 0) {
        last_error_status = DM9051_OK;
        DM9051_UIP_DIAG_PRINTF("[DM9051 uIP] rx burst=%d pending=%u status=%d reason=%s\r\n",
                               rx_burst,
                               drain_pending,
                               rx_status,
                               reason);
    } else if ((rx_status != DM9051_OK) &&
               (rx_status != DM9051_ERR_NOT_READY) &&
               (rx_status != last_error_status)) {
        last_error_status = rx_status;
        DM9051_UIP_DIAG_PRINTF("[DM9051 uIP] rx status=%d reason=%s\r\n",
                               rx_status,
                               reason);
    } else if ((rx_status == DM9051_OK) ||
               (rx_status == DM9051_ERR_NOT_READY)) {
        last_error_status = DM9051_OK;
    }
}

int dm9051_uip_stack_init(struct uip_ethernetif *eth,
                           const dm9051_netif_device_t *netif)
{
    struct uip_eth_addr ethaddr;
    uip_ipaddr_t ipaddr;
    int status;

    if ((eth == NULL) || !dm9051_netif_device_is_valid(netif) ||
        (netif->mac_addr == 0)) {
        return DM9051_ERR_PARAM;
    }

    status = dm9051_uip_attach(&eth->dev);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_uip_init(netif);
    if (status != DM9051_OK) {
        return status;
    }

    uip_init();
    uip_arp_init();

    (void)memcpy(ethaddr.addr, netif->mac_addr, sizeof(ethaddr.addr));
    uip_setethaddr(ethaddr);

    uip_ipaddr(ipaddr,
               netif->static_ip[0], netif->static_ip[1],
               netif->static_ip[2], netif->static_ip[3]);
    uip_sethostaddr(ipaddr);

    uip_ipaddr(ipaddr,
               netif->gateway_ip[0], netif->gateway_ip[1],
               netif->gateway_ip[2], netif->gateway_ip[3]);
    uip_setdraddr(ipaddr);

    uip_ipaddr(ipaddr,
               netif->netmask_ip[0], netif->netmask_ip[1],
               netif->netmask_ip[2], netif->netmask_ip[3]);
    uip_setnetmask(ipaddr);

#if DM9051_UIP_ENABLE_PERIODIC
    timer_set(&dm9051_uip_periodic_timer, CLOCK_SECOND / 2);
    timer_set(&dm9051_uip_arp_timer, CLOCK_SECOND * 10);
    dm9051_uip_tcp_periodic_index = 0u;
    dm9051_uip_udp_periodic_index = 0u;
    dm9051_uip_tcp_periodic_pending = 0u;
    dm9051_uip_udp_periodic_pending = 0u;
#endif
    printf("[DM9051 uIP] stack init mode=%s\r\n", dm9051_uip_target_mode());
    printf("[DM9051 uIP] MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           ethaddr.addr[0], ethaddr.addr[1], ethaddr.addr[2],
           ethaddr.addr[3], ethaddr.addr[4], ethaddr.addr[5]);
    printf("[DM9051 uIP] IP  %u.%u.%u.%u\r\n",
           netif->static_ip[0], netif->static_ip[1],
           netif->static_ip[2], netif->static_ip[3]);

    return DM9051_OK;
}

void dm9051_uip_stack_poll(void)
{
    static uint8_t dm9051_uip_rx_drain_pending;
    int rx_burst;
    int irq_mode;

    irq_mode = dm9051_uip_interrupt_mode();
    if ((irq_mode == DM9051_INPUT_MODE_POLL) ||
        (dm9051_uip_rx_drain_pending != 0u) ||
        (dm9051_uip_interrupt_take() != 0)) {
        rx_burst = dm9051_uip_stack_drain_rx();
        dm9051_uip_rx_drain_pending =
            dm9051_uip_stack_rx_pending_from_burst(rx_burst);
        dm9051_uip_stack_print_rx_burst("rx",
                                        rx_burst,
                                        dm9051_uip_rx_drain_pending);

        if ((irq_mode != DM9051_INPUT_MODE_POLL) &&
            (dm9051_uip_rx_drain_pending == 0u)) {
            dm9051_uip_interrupt_reset();
        }
    }

#if DM9051_UIP_ENABLE_PERIODIC
    if (timer_expired(&dm9051_uip_periodic_timer)) {
        timer_reset(&dm9051_uip_periodic_timer);
        dm9051_uip_tcp_periodic_pending = 1u;
        dm9051_uip_udp_periodic_pending = 1u;
        dm9051_uip_tcp_periodic_index = 0u;
        dm9051_uip_udp_periodic_index = 0u;
    }

    if (dm9051_uip_tcp_periodic_pending != 0u) {
        if (dm9051_uip_tcp_periodic_index < UIP_CONNS) {
            uip_periodic(dm9051_uip_tcp_periodic_index);
            dm9051_uip_stack_send_if_needed();
            dm9051_uip_tcp_periodic_index++;
        }
        if (dm9051_uip_tcp_periodic_index >= UIP_CONNS) {
            dm9051_uip_tcp_periodic_index = 0u;
            dm9051_uip_tcp_periodic_pending = 0u;
        }
    }

#if UIP_UDP
    if (dm9051_uip_udp_periodic_pending != 0u) {
        if (dm9051_uip_udp_periodic_index < UIP_UDP_CONNS) {
            uip_udp_periodic(dm9051_uip_udp_periodic_index);
            dm9051_uip_stack_send_if_needed();
            dm9051_uip_udp_periodic_index++;
        }
        if (dm9051_uip_udp_periodic_index >= UIP_UDP_CONNS) {
            dm9051_uip_udp_periodic_index = 0u;
            dm9051_uip_udp_periodic_pending = 0u;
        }
    }
#else
    dm9051_uip_udp_periodic_pending = 0u;
#endif

    if ((dm9051_uip_tcp_periodic_pending != 0u) ||
        (dm9051_uip_udp_periodic_pending != 0u)) {
        if ((irq_mode == DM9051_INPUT_MODE_POLL) ||
            (dm9051_uip_rx_drain_pending != 0u) ||
            (dm9051_uip_interrupt_take() != 0)) {
            rx_burst = dm9051_uip_stack_drain_rx();
            dm9051_uip_rx_drain_pending =
                dm9051_uip_stack_rx_pending_from_burst(rx_burst);
            dm9051_uip_stack_print_rx_burst("periodic",
                                            rx_burst,
                                            dm9051_uip_rx_drain_pending);

            if ((irq_mode != DM9051_INPUT_MODE_POLL) &&
                (dm9051_uip_rx_drain_pending == 0u)) {
                dm9051_uip_interrupt_reset();
            }
        }
    }

    if (timer_expired(&dm9051_uip_arp_timer)) {
        timer_reset(&dm9051_uip_arp_timer);
        uip_arp_timer();
    }
#endif
}
