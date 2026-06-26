/**
 * @file  dm9051_uip_adapter.h
 * @brief DM9051 uIP network stack adapter — public header.
 *
 * This header is the only DM9051-related include that uIP application code
 * (e.g. main_uip_mh2030a.c, netconf_mh2030a.c) should reference.
 *
 * It re-exports dm9051_netif.h (the generic device interface) so callers do
 * not need to include both.
 */

#ifndef __DM9051_UIP_ADAPTER_H
#define __DM9051_UIP_ADAPTER_H

/* Re-export the generic netif interface so callers include only this file. */
#include "../../../drivers/dm9051_edriver_v1.6.1a_beta/include/dm9051_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise the DM9051 uIP adapter.
 *
 *         Internally calls dm9051_netif_open() and configures uIP with the
 *         supplied network parameters.  Should be called after
 *         mh2030a_uip_tick_init() and before the uIP main loop.
 *
 * @param  dev  Pointer to network configuration (IP, GW, mask, MAC).
 *              The struct must remain valid for the lifetime of the session.
 * @return 0 on success, negative on error.
 */
int dm9051_uip_adapter_init(const dm9051_netif_device_t *dev);

/**
 * @brief  Run one iteration of the uIP receive / periodic loop.
 *
 *         Must be called from the application main loop as fast as possible.
 *         Internally handles RX burst drain, uip_input(), uip_periodic(),
 *         ARP timer expiry, and ethernetif_output().
 */
void dm9051_uip_adapter_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* __DM9051_UIP_ADAPTER_H */
