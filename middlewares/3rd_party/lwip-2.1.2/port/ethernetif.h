#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

/**
 * @file ethernetif.h
 * @brief DM9051 SPI Ethernet lwIP netif 移植層介面。
 *
 * 與 ethernetif.c 成對，提供標準的 lwIP netif init / input 函式。
 */

#include <stdint.h>

#include "lwip/err.h"
#include "lwip/netif.h"

/*
 * DM9051 types + HAL 的相對路徑。
 * 如果 Keil 的 Include Paths 已涵蓋則自動忽略；這裡確保 header 可獨立使用。
 */
#include "../../../../ModuleDemo/DM9051A/dm9051_driver/core/inc/dm9051_types.h"
#include "../../../../ModuleDemo/DM9051A/dm9051_driver/hal/inc/dm9051_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * 每個 netif 的私有資料 — 儲存在 netif->state 中
 * ------------------------------------------------------------------------ */
struct ethernetif {
    dm9051_device_t dev;
    dm9051_hal_t    hal;
    uint8_t         rx_buf[1514];
    uint8_t         tx_buf[1514];
};

/* ---------------------------------------------------------------------------
 * 標準 netif 介面 (適用於進階用法: 自行管理 netif 實例)
 *
 * 用法:
 *   struct netif my_netif;
 *   netif_add(&my_netif, &ip, &mask, &gw, NULL,
 *             ethernetif_init, ethernet_input);
 *   netif_set_link_callback(my_netif, ethernetif_update_config);
 *   netif_set_default(&my_netif);
 *   netif_set_up(&my_netif);
 *
 *   之後在主迴圈定期呼叫 ethernetif_input(&my_netif) 收封包。
 * ------------------------------------------------------------------------ */

/** @brief netif_add 的 init callback: 初始化硬體並設定 netif 屬性。 */
err_t ethernetif_init(struct netif *netif);

/** @brief 從 DM9051 輪詢一幀並餵入 lwIP。若收到封包回傳 ERR_OK。 */
err_t ethernetif_input(struct netif *netif);

/** @brief Link 狀態變更回呼。透過 netif_set_link_callback 註冊。 */
void ethernetif_update_config(struct netif *netif);

/** @brief 輪詢 PHY link 狀態並同步 lwIP 旗標 (建議 100-500ms 週期呼叫)。 */
void ethernetif_link_poll(struct netif *netif);

/* ---------------------------------------------------------------------------
 * PTP 硬體時間戳 (預留 / 視晶片支援啟用)
 * ------------------------------------------------------------------------ */
#if LWIP_PTP
struct ptptime_t {
    s32_t tv_sec;
    s32_t tv_nsec;
};

void emac_ptptime_settime(struct ptptime_t *timestamp);
void emac_ptptime_gettime(struct ptptime_t *timestamp);
void emac_ptptime_updateoffset(struct ptptime_t *timeoffset);
void emac_ptptime_adjfreq(int32_t Adj);
u32_t emac_ptpsubsecond2nanosecond(u32_t subsecondvalue);
#endif /* LWIP_PTP */

#ifdef __cplusplus
}
#endif

#endif /* __ETHERNETIF_H__ */

