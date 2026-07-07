/**
 * @file    at_net_shim.h
 * @brief   uIP API → 目標網路堆疊 對接層 —— 跨平台移植範本
 *
 * 原則 (見 doc/porting_v01/03-uip-api-mapping.md)：
 *   A 類｜純資料巨集 (IP 位元組運算)  → 原樣保留 (本檔重定義)
 *   B 類｜事件/連線 API               → #define 到 atshim_* 由目標平台實作
 *
 * 使用：複製到新平台 port/。AT_Command 的 atcommand.h 原本 #include "uip.h"，
 *       移植時改為 #include "at_net_shim.h" (本檔再決定是否轉接真 uip.h)。
 * 原 AT32F413 專案不引用此檔。
 */
#ifndef __AT_NET_SHIM_H__
#define __AT_NET_SHIM_H__

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 0. 後端選擇：定義其一
 *   AT_NET_BACKEND_UIP    沿用 uIP (移植到新 MCU 但仍帶 uIP；本檔直接 include uip.h)
 *   AT_NET_BACKEND_LWIP   對接 lwIP
 *   AT_NET_BACKEND_SOCKET 對接 BSD socket
 * ============================================================ */
#if !defined(AT_NET_BACKEND_UIP) && !defined(AT_NET_BACKEND_LWIP) && !defined(AT_NET_BACKEND_SOCKET)
#define AT_NET_BACKEND_LWIP   /* 預設示範 lwIP；請依目標改 */
#endif

#ifdef AT_NET_BACKEND_UIP
/* 仍帶 uIP：直接用原堆疊，本檔幾乎透通 */
#include "uip.h"
#else

/* ============================================================
 * 1. 型別 (A 類)
 *   uip_ipaddr_t = 32-bit IPv4，內部以 u16_t[2] 表示 (與原 uIP 相容，
 *   因此 struct at_funcation 不需修改)。
 * ============================================================ */
typedef uint16_t uip_ip4addr_t[2];
typedef uip_ip4addr_t uip_ipaddr_t;

#ifndef HTONS
#define HTONS(n) ((uint16_t)((((n) & 0xff) << 8) | (((n) >> 8) & 0xff)))
#endif

/* ============================================================
 * 2. A 類純資料巨集 —— 原樣保留 (抄自 uIP 純運算版本)
 *   AT_Command 用到：uip_ipaddr / uip_ipaddr1..4 / uip_ipaddr_copy
 * ============================================================ */
#define uip_ipaddr(addr, a0,a1,a2,a3) do {              \
    ((uint16_t*)(addr))[0] = HTONS(((a0) << 8) | (a1)); \
    ((uint16_t*)(addr))[1] = HTONS(((a2) << 8) | (a3)); \
  } while(0)

#define uip_ipaddr1(addr) (htons(((uint16_t*)(addr))[0]) >> 8)
#define uip_ipaddr2(addr) (htons(((uint16_t*)(addr))[0]) & 0xff)
#define uip_ipaddr3(addr) (htons(((uint16_t*)(addr))[1]) >> 8)
#define uip_ipaddr4(addr) (htons(((uint16_t*)(addr))[1]) & 0xff)

#define uip_ipaddr_copy(dest, src) \
    memcpy((dest), (src), sizeof(uip_ipaddr_t))
#define uip_ipaddr_cmp(addr1, addr2) \
    (memcmp((addr1), (addr2), sizeof(uip_ipaddr_t)) == 0)

#ifndef htons
#define htons(n) HTONS(n)
#endif

/* ============================================================
 * 3. B 類 連線/設定 API —— 對接到目標平台 (在 at_net_shim.c 實作 atshim_*)
 * ============================================================ */

/* --- IP / 介面設定 --- */
void atshim_set_ip(uip_ipaddr_t a);
void atshim_set_mask(uip_ipaddr_t a);
void atshim_set_gw(uip_ipaddr_t a);
void atshim_get_ip(uip_ipaddr_t *out);

#define uip_sethostaddr(a)   atshim_set_ip(a)
#define uip_setnetmask(a)    atshim_set_mask(a)
#define uip_setdraddr(a)     atshim_set_gw(a)
#define uip_gethostaddr(a)   atshim_get_ip(a)

/* --- TCP --- */
void atshim_tcp_init(void);
int  atshim_tcp_listen(uint16_t port_n);            /* port 為 network order */
int  atshim_tcp_connect(uip_ipaddr_t *ip, uint16_t port_n);

#define tcp_init()           atshim_tcp_init()
#define uip_listen(p)        atshim_tcp_listen(p)
#define uip_connect(ip,p)    atshim_tcp_connect((ip),(p))

/* --- UDP --- */
void *atshim_udp_new(uip_ipaddr_t *ip, uint16_t rport_n);
int   atshim_udp_bind(void *conn, uint16_t lport_n);

#define uip_udp_new(ip,p)    atshim_udp_new((ip),(p))
#define uip_udp_bind(c,p)    atshim_udp_bind((c),(p))

/* --- 應用層送出 (取代 uip_appdata 單緩衝) ---
 * 原碼: sprintf(uip_appdata, "..."); + (由堆疊送出)
 * 移植: sprintf(atshim_appbuf(), "..."); atshim_send(strlen(...));
 * 為相容，提供 uip_appdata 指向 shim 的 TX buffer。 */
char *atshim_appbuf(void);
int   atshim_send(uint16_t len);
#define uip_appdata          (atshim_appbuf())

/* --- DHCP / DNS (多為啟用旗標，真正動作在堆疊) --- */
void atshim_dhcp_start(void);
void atshim_dhcp_stop(void);
int  atshim_dns_query(const char *name, uip_ipaddr_t *out);

#endif /* !AT_NET_BACKEND_UIP */

#ifdef __cplusplus
}
#endif

#endif /* __AT_NET_SHIM_H__ */
