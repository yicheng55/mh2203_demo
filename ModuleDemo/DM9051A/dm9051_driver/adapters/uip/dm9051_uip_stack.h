#ifndef DM9051_UIP_STACK_H
#define DM9051_UIP_STACK_H

#include "dm9051_uip.h"

struct uip_udp_conn;

#ifdef __cplusplus
extern "C" {
#endif

int dm9051_uip_stack_init(struct uip_ethernetif *eth,
                           const dm9051_netif_device_t *netif);
void dm9051_uip_stack_poll(void);

/* 立即輪詢指定 UDP 連線並在有資料待傳時送出 (供應用層在資料佇列後盡快清空，
 * 例如 udp_printf 佇列，避免等到下一輪 periodic timer 才送出)。 */
void dm9051_uip_stack_udp_poke(struct uip_udp_conn *conn);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_UIP_STACK_H */
