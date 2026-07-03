/**
 * @file    app_call.h
 * @brief   uIP UIP_APPCALL / UIP_UDP_APPCALL 分派層 — MH2203 AT_Command 版
 *
 * 仿 AT32 SDK/User/app_call.h 的寫法：tcp_appcall()/udp_appcall() 依 port
 * 分派到 httpd_appcall() (webserver) / etherbridge 的 tcp_bridge_appcall()、
 * udp_bridge_appcall() / resolv_appcall() / dhcpc_appcall()。
 */
#ifndef APP_CALL_H
#define APP_CALL_H

#include <stdint.h>

#define CFG_TCP_APP_ENABLE  /* 需要 webserver + etherbridge TCP 透傳 */
#define CFG_UDP_APP_ENABLE  /* 需要 DNS resolver + etherbridge UDP 透傳 */

#ifdef CFG_TCP_APP_ENABLE
#define HTTP_SERVER_SUPPORT
#endif

/*--------------------- TCP APP ---------------------------------------*/
#ifdef CFG_TCP_APP_ENABLE

#ifdef HTTP_SERVER_SUPPORT
#include "httpd.h"
#endif /* HTTP_SERVER_SUPPORT */

typedef union {
#ifdef HTTP_SERVER_SUPPORT
    struct httpd_state httpd_app;
#endif /* HTTP_SERVER_SUPPORT */
} Str_TCP_App_State;

typedef Str_TCP_App_State uip_tcp_appstate_t;
#else
typedef int uip_tcp_appstate_t;
#endif /* CFG_TCP_APP_ENABLE */

void tcp_appcall(void);
#ifndef UIP_APPCALL
#define UIP_APPCALL tcp_appcall
#endif

/*--------------------- UDP APP ---------------------------------------*/
#ifdef CFG_UDP_APP_ENABLE

/* DHCPC_EN 預設關閉 (比照 AT32 原版)；型別仍需要 dhcpc.h 供 union 使用。 */
#include "dhcpc.h"
#include "resolv.h"

typedef union {
    struct dhcpc_state dhcpc_app;
} Str_UDP_App_State;

typedef Str_UDP_App_State uip_udp_appstate_t;
#else
typedef int uip_udp_appstate_t;
#endif /* CFG_UDP_APP_ENABLE */

void udp_appcall(void);
#ifndef UIP_UDP_APPCALL
#define UIP_UDP_APPCALL udp_appcall
#endif

/* tcp_init()/udp_init() 相容 wrapper (見 app_call.c 檔頭說明)：目的專案的
 * uip.c 只有合併版 uip_init()，這裡補上 AT32 原本拆開的兩個函式。 */
void tcp_init(void);
void udp_init(void);

/* 取代 AT32 uip_init.c 提供的連線狀態旗標 (見 03 節)：
 * eth_netif_linkup 由主迴圈依 dm9051_uip_link_poll() 的結果同步。 */
extern uint8_t eth_netif_linkup;
extern uint8_t EthernetInitDoneFlag;
extern char    dhs_staus_msg[64];

void dns_client_init(void);
void EthernetInit_Done(void);

#endif /* APP_CALL_H */
