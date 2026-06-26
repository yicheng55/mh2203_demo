#ifndef DM9051_UIP_H
#define DM9051_UIP_H

#include "../../core/inc/dm9051_types.h"
#include "../../hal/inc/dm9051_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Staging uIP adapter API.
 *
 * Current source of truth:
 *   ModuleDemo/DM9051A/port/uip/dm9051_uip_adapter.h
 *
 * This header is intentionally stack-facing, not platform-facing. It must not
 * include MH2030A SPI/GPIO/IRQ headers.
 */

/* ---------------------------------------------------------------------------
 * struct uip_ethernetif — 每個 netif 的私有資料 (比照 lwIP ethernetif)
 *
 * 用法 (示意):
 *   struct uip_ethernetif eth;
 *   dm9051_uip_low_level_init(&eth, mac_addr);  // HAL bind + core_open
 *   dm9051_uip_attach(&eth.dev);
 *   dm9051_uip_stack_init(&eth, &netif_config);
 *   // 主迴圈:
 *   dm9051_uip_link_poll(&eth);
 *   dm9051_uip_stack_poll();
 * ------------------------------------------------------------------------ */
struct uip_ethernetif {
    dm9051_device_t dev;
    dm9051_hal_t    hal;
    uint8_t         rx_buf[DM9051_ETH_FRAME_MAX];
    uint8_t         tx_buf[DM9051_ETH_FRAME_MAX];
};

int dm9051_uip_init(const dm9051_netif_device_t *dev);
int dm9051_uip_attach(dm9051_device_t *dev);
uint16_t dm9051_uip_input(uint8_t *buf, uint16_t buf_len);
int dm9051_uip_last_rx_status(void);
int dm9051_uip_output(const uint8_t *buf, uint16_t len);
int dm9051_uip_interrupt_mode(void);
int dm9051_uip_interrupt_take(void);
void dm9051_uip_interrupt_reset(void);
void dm9051_uip_poll(void);
const char *dm9051_uip_target_mode(void);

/* --- 新增: 連結輪詢 (比照 ethernetif_link_poll) --- */
int dm9051_uip_link_poll(struct uip_ethernetif *eth);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_UIP_H */
