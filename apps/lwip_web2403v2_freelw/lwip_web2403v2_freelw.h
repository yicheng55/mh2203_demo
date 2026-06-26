#ifndef LWIP_WEB2403V2_FREELW_H
#define LWIP_WEB2403V2_FREELW_H

/**
 * @file lwip_web2403v2_freelw.h
 * @brief lwip_web2403v2_freelw 應用層的簡單啟動/輪詢介面。
 */

#include "lwip/err.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 啟動 Web server，並綁定到指定 netif 的 IPv4 位址。
 */
err_t lwip_web2403v2_freelw_init(struct netif *netif);

/**
 * @brief 應用層輪詢 hook。
 *
 * 目前此 HTTPD 使用 lwIP raw API callback，不需要額外輪詢；
 * 保留此函式可讓 main loop 架構固定，未來若加入 LED、CGI 狀態機或
 * 背景資料更新，可直接放在這裡。
 */
void lwip_web2403v2_freelw_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_WEB2403V2_FREELW_H */
