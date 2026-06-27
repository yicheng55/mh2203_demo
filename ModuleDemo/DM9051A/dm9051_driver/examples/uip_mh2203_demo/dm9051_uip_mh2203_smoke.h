#ifndef DM9051_UIP_MH2203_SMOKE_H
#define DM9051_UIP_MH2203_SMOKE_H

#include <stdint.h>
#include "../../core/inc/dm9051_types.h"
#include "../../adapters/uip/dm9051_uip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 純硬體探測 (讀 VID/PID/CHIPR)，不接 uIP stack。 */
int dm9051_uip_mh2203_smoke_init(void);

/* uIP 整合用的開啟流程 (HAL bind + core_open)，比照 mh2030a smoke。
 * 開啟後以 dm9051_uip_mh2203_smoke_eth() 取得 netif 私有資料供 stack 使用。 */
int dm9051_uip_mh2203_smoke_open(const uint8_t *mac_addr);
struct uip_ethernetif *dm9051_uip_mh2203_smoke_eth(void);
const dm9051_device_t *dm9051_uip_mh2203_smoke_device(void);
dm9051_device_t *dm9051_uip_mh2203_smoke_mutable_device(void);
int dm9051_uip_mh2203_smoke_last_status(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_UIP_MH2203_SMOKE_H */
