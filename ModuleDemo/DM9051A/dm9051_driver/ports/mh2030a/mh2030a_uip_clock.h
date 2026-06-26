#ifndef DM9051_MH2030A_UIP_CLOCK_H
#define DM9051_MH2030A_UIP_CLOCK_H

#include <stdint.h>

#ifndef MH2030A_UIP_TICK_MS
#define MH2030A_UIP_TICK_MS 10u
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mh2030a_uip_tick_init(void);
void mh2030a_uip_update_time(void);
uint32_t mh2030a_uip_millis(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_MH2030A_UIP_CLOCK_H */
