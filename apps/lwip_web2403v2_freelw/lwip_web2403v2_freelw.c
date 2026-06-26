/**
 * @file lwip_web2403v2_freelw.c
 * @brief lwip_web2403v2_freelw 與專案 main loop 之間的薄型整合層。
 */

#include "lwip_web2403v2_freelw.h"

/*
 * httpd.c 內已實作此函式，但原本沒有專用 app header。
 * 這裡用 extern 將既有 HTTP server 啟動點包成應用層 init API。
 */
extern err_t httpd_init_with_netif(struct netif *preferred_netif);

err_t lwip_web2403v2_freelw_init(struct netif *netif)
{
    if (netif == NULL) {
        return ERR_ARG;
    }

    return httpd_init_with_netif(netif);
}

void lwip_web2403v2_freelw_poll(void)
{
    /*
     * lwIP raw API 的 HTTPD 由 TCP callback 與 lwIP timeout 推進。
     * Bare-metal main loop 中仍保留此 hook，方便未來放應用層週期工作。
     */
}

void lwip_platform_assert(const char *msg)
{
    (void)msg;

    while (1) {
    }
}
