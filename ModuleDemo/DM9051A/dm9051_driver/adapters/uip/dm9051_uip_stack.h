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
void dm9051_uip_stack_poll_udp_conn(struct uip_udp_conn *conn);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_UIP_STACK_H */
