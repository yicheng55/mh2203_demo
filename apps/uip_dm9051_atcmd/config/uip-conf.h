/**
 * uip-conf.h — MH2203 AT_Command 專案專用複本
 *
 * 複製自 apps/uip_dm9051_example_e1/uip_conf_inc/uip-conf.h，獨立成一份，
 * 不與既有 uip demo 專案共用 (避免動到已驗證可動的 DM9051A_mh2203_uip.uvprojx)。
 * 差異：明確定義 UIP_CONF_UDP_CONNS (原本靠 uipopt.h 的預設值 10，這裡
 * 比照 AT32 原版顯式定義)。其餘設定值原樣保留。
 */
#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <inttypes.h>

typedef uint8_t u8_t;

#ifndef UIP_U16_T_DEFINED
typedef uint16_t u16_t;
#define UIP_U16_T_DEFINED
#endif

typedef unsigned short uip_stats_t;

#define UIP_CONF_MAX_CONNECTIONS 10
#define UIP_CONF_MAX_LISTENPORTS 40

/* AT_Command dataflash.c / etherbridge.c 未定義 UIP_USE_BIGDATA_SIZE，
 * RecData_Size 固定 1460，沿用既有 buffer size 設定即可。 */
#define UIP_CONF_BUFFER_SIZE     	1200
#define UIP_CONF_BUFFER_SIZE_OLD	420

#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN
#define UIP_CONF_LOGGING         1
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_UDP_CONNS       5
#define UIP_CONF_STATISTICS      1

#include "app_call.h"

#if TCP_APP_EN
 void tcp_conn_list(char *HeadStr, u8_t nConnShow);
#else
 #define tcp_conn_list(headstr, nconnshow)
#endif

#endif /* __UIP_CONF_H__ */
