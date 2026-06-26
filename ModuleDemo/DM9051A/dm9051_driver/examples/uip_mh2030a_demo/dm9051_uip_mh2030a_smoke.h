#ifndef DM9051_UIP_MH2030A_SMOKE_H
#define DM9051_UIP_MH2030A_SMOKE_H

#include "../../adapters/uip/dm9051_uip.h"

#ifdef __cplusplus
extern "C" {
#endif

int dm9051_uip_mh2030a_smoke_open(const uint8_t *mac_addr);
struct uip_ethernetif *dm9051_uip_mh2030a_smoke_eth(void);
const dm9051_device_t *dm9051_uip_mh2030a_smoke_device(void);
dm9051_device_t *dm9051_uip_mh2030a_smoke_mutable_device(void);
int dm9051_uip_mh2030a_smoke_last_status(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_UIP_MH2030A_SMOKE_H */
