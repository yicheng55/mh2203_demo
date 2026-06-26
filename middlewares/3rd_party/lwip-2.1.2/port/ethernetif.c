/**
 * @file ethernetif.c
 * @brief DM9051A SPI Ethernet 的標準 lwIP netif 移植層。
 *
 * 遵循 lwIP 2.1.2 標準 ethernetif 模板風格 (STM32Cube / NXP MCU 慣例)，
 * 底層委託 DM9051 Core Driver (dm9051_core_*) 處理 SPI 通訊。
 *
 * 使用方法 (bare-metal, NO_SYS=1)：
 *   1. 設定 MAC, IP, netmask, gateway
 *   2. netif_add(&netif, &ip, &mask, &gw, NULL,
 *               ethernetif_init, ethernet_input)
 *   3. netif_set_link_callback(netif, ethernetif_update_config);
 *   4. 主迴圈中呼叫 ethernetif_input(&netif) 輪詢 RX
 *
 * 若使用中斷模式，在 EXTI ISR 設旗號後於主迴圈呼叫 ethernetif_input()。
 */

#include "ethernetif.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/etharp.h"
#include "lwip/ethip6.h"
#include "netif/ethernet.h"

/*
 * DM9051 Core + HAL + Platform Port 的頭檔路徑。
 *
 * 編譯時需在 Keil C/C++ Include Paths 中加入：
 *   ModuleDemo/DM9051A/dm9051_driver/core/inc
 *   ModuleDemo/DM9051A/dm9051_driver/hal/inc
 *   ModuleDemo/DM9051A/dm9051_driver/ports/mh2030a
 *
 * 或改用相對路徑：
 */
#include "../../../../ModuleDemo/DM9051A/dm9051_driver/core/inc/dm9051_core.h"
#include "../../../../ModuleDemo/DM9051A/dm9051_driver/hal/inc/dm9051_hal.h"
#include "../../../../ModuleDemo/DM9051A/dm9051_driver/ports/mh2030a/dm9051_hal_mh2030a_spi1.h"

/* ---------------------------------------------------------------------------
 * 調試開關
 * ------------------------------------------------------------------------ */
#ifndef ETHERNETIF_DIAG
#define ETHERNETIF_DIAG 1
#endif

#if ETHERNETIF_DIAG
#define ETHERNETIF_PRINTF(...) printf(__VA_ARGS__)
#else
#define ETHERNETIF_PRINTF(...) do { } while (0)
#endif

/* ---------------------------------------------------------------------------
 * 移除 RX 尾端 4-byte FCS (CRC32)
 * 設為 1 時 low_level_input 會自動裁剪最後 4 bytes。
 * ------------------------------------------------------------------------ */
#ifndef ETHERNETIF_RX_STRIP_FCS
#define ETHERNETIF_RX_STRIP_FCS 0
#endif

/* ---------------------------------------------------------------------------
 * 靜態 buffer 尺寸 — DM9051 底層需要連續記憶體
 * ------------------------------------------------------------------------ */
#ifndef ETHERNETIF_MTU
#define ETHERNETIF_MTU         1500U
#endif

#define ETHERNETIF_ETH_FRAME_SIZE   1514U   /* 14 header + 1500 payload */

/* ---------------------------------------------------------------------------
 * Ethernet frame type 偵測 (僅供 diag)
 * ------------------------------------------------------------------------ */
static uint16_t eth_type(const uint8_t *frame, uint16_t len)
{
    if ((frame == NULL) || (len < 14U)) return 0U;
    return (uint16_t)(((uint16_t)frame[12] << 8) | frame[13]);
}

static const char *eth_type_name(uint16_t t)
{
    switch (t) {
    case 0x0800U: return "IPv4";
    case 0x0806U: return "ARP";
    case 0x86DDU: return "IPv6";
    default:      return "ETH";
    }
}

/* ---------------------------------------------------------------------------
 * low_level_init — 硬體初始化 (被 ethernetif_init 呼叫)
 *
 * 1. 填充 hal vtable (MH2030A SPI1 / DMA / IRQ)
 * 2. dm9051_core_open() 初始化 DM9051 晶片
 * 3. 將有效 MAC 寫回 netif->hwaddr
 * ------------------------------------------------------------------------ */
static void low_level_init(struct netif *netif)
{
    struct ethernetif *eth = (struct ethernetif *)netif->state;
    dm9051_config_t core_config;
    dm9051_mh2030a_config_t port_config;
    const uint8_t *active_mac;
    int status;

    /* --- Core config --- */
    dm9051_core_default_config(&core_config);
    core_config.mac_addr = netif->hwaddr;

#if DM9051_MH2030A_USE_IRQ
    core_config.interrupt_mode = DM9051_INPUT_MODE_INTERRUPT;
#else
    core_config.interrupt_mode = DM9051_INPUT_MODE_POLL;
#endif
    core_config.flow_control = 0U;

    /* --- Platform port config --- */
    dm9051_mh2030a_default_config(&port_config);
#if DM9051_MH2030A_USE_DMA
    port_config.transport = DM9051_MH2030A_TRANSPORT_DMA;
#else
    port_config.transport = DM9051_MH2030A_TRANSPORT_POLLING;
#endif
#if DM9051_MH2030A_USE_IRQ
    port_config.irq_mode = DM9051_MH2030A_IRQ_EXTI;
#else
    port_config.irq_mode = DM9051_MH2030A_IRQ_OFF;
#endif

    /* --- Bind HAL --- */
    status = dm9051_mh2030a_hal_bind(&eth->hal, &port_config);
    if (status != DM9051_HAL_OK) {
        ETHERNETIF_PRINTF("[etherif] HAL bind failed (%d)\r\n", status);
        return;
    }

#if DM9051_MH2030A_USE_IRQ
    dm9051_mh2030a_irq_attach_device(&eth->dev);
#endif

    /* --- Open DM9051 core --- */
    status = dm9051_core_open(&eth->dev, &core_config, &eth->hal);
    if (status != DM9051_OK) {
        ETHERNETIF_PRINTF("[etherif] core_open failed (%d)\r\n", status);
#if DM9051_MH2030A_USE_IRQ
        dm9051_mh2030a_irq_detach_device();
#endif
        return;
    }

    /* --- 回讀有效 MAC --- */
    active_mac = dm9051_core_mac(&eth->dev);
    if (active_mac != NULL) {
        memcpy(netif->hwaddr, active_mac, ETH_HWADDR_LEN);
    }

    /* --- 設定 netif 屬性 --- */
    netif->hwaddr_len = ETH_HWADDR_LEN;
    netif->mtu        = ETHERNETIF_MTU;
    netif->flags      = NETIF_FLAG_BROADCAST |
                        NETIF_FLAG_ETHARP |
                        NETIF_FLAG_ETHERNET;

#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    ETHERNETIF_PRINTF("[etherif] hw MAC=%02X:%02X:%02X:%02X:%02X:%02X "
                      "mtu=%u flags=0x%02X\r\n",
                      netif->hwaddr[0], netif->hwaddr[1],
                      netif->hwaddr[2], netif->hwaddr[3],
                      netif->hwaddr[4], netif->hwaddr[5],
                      (unsigned)netif->mtu,
                      (unsigned)netif->flags);
}

/* ---------------------------------------------------------------------------
 * low_level_output — lwIP → DM9051 發送路徑
 *
 * netif->linkoutput 會指向此函式。
 * lwIP 的 pbuf 可能為 chain，必須先複製到連續 tx_buf 才能送給 DM9051。
 * ------------------------------------------------------------------------ */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct ethernetif *eth = (struct ethernetif *)netif->state;

    if (p == NULL) return ERR_ARG;

    if (!netif_is_link_up(netif)) {
        LINK_STATS_INC(link.drop);
        return ERR_RTE;
    }

    if (p->tot_len > sizeof(eth->tx_buf)) {
        LINK_STATS_INC(link.lenerr);
        LINK_STATS_INC(link.drop);
        return ERR_BUF;
    }

    if (pbuf_copy_partial(p, eth->tx_buf, p->tot_len, 0) != p->tot_len) {
        LINK_STATS_INC(link.err);
        return ERR_BUF;
    }

    ETHERNETIF_PRINTF("[etherif] TX len=%u type=%s(0x%04X)\r\n",
                      (unsigned)p->tot_len,
                      eth_type_name(eth_type(eth->tx_buf, p->tot_len)),
                      eth_type(eth->tx_buf, p->tot_len));

    if (dm9051_core_send(&eth->dev, eth->tx_buf, p->tot_len) != DM9051_OK) {
        LINK_STATS_INC(link.err);
        return ERR_IF;
    }

    LINK_STATS_INC(link.xmit);
    return ERR_OK;
}

/* ---------------------------------------------------------------------------
 * low_level_input — 從 DM9051 接收一幀，回傳 pbuf
 *
 * 若無封包可讀則回傳 NULL。
 * 呼叫者 (ethernetif_input) 負責將 pbuf 餵入 netif->input() 後釋放。
 * ------------------------------------------------------------------------ */
static struct pbuf *low_level_input(struct netif *netif)
{
    struct ethernetif *eth = (struct ethernetif *)netif->state;
    struct pbuf *p;
    uint16_t len;
    uint16_t raw_len;

    len = dm9051_core_receive(&eth->dev, eth->rx_buf, sizeof(eth->rx_buf));
    if (len == 0U) return NULL;

    if (len > sizeof(eth->rx_buf)) {
        LINK_STATS_INC(link.lenerr);
        LINK_STATS_INC(link.drop);
        return NULL;
    }

    raw_len = len;
#if ETHERNETIF_RX_STRIP_FCS
    if (len > 4U) len = (uint16_t)(len - 4U);
#endif

    if (len < 14U) {
        LINK_STATS_INC(link.lenerr);
        LINK_STATS_INC(link.drop);
        return NULL;
    }

    ETHERNETIF_PRINTF("[etherif] RX raw=%u len=%u type=%s(0x%04X) "
                      "dst=%02X:%02X:%02X:%02X:%02X:%02X "
                      "src=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                      (unsigned)raw_len, (unsigned)len,
                      eth_type_name(eth_type(eth->rx_buf, len)),
                      eth_type(eth->rx_buf, len),
                      eth->rx_buf[0], eth->rx_buf[1],
                      eth->rx_buf[2], eth->rx_buf[3],
                      eth->rx_buf[4], eth->rx_buf[5],
                      eth->rx_buf[6], eth->rx_buf[7],
                      eth->rx_buf[8], eth->rx_buf[9],
                      eth->rx_buf[10], eth->rx_buf[11]);

    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (p == NULL) {
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        return NULL;
    }

    if (pbuf_take(p, eth->rx_buf, len) != ERR_OK) {
        pbuf_free(p);
        LINK_STATS_INC(link.err);
        return NULL;
    }

    return p;
}

/* ---------------------------------------------------------------------------
 * ethernetif_init — lwIP netif_add() 的 init callback
 *
 * 設定 netif->state, output / linkoutput 鏈結，再呼叫 low_level_init 做硬體初始化。
 *
 * 使用方式：
 *   netif_add(&netif, &ip, &mask, &gw, NULL,
 *             ethernetif_init, ethernet_input);
 *   netif_set_link_callback(netif, ethernetif_update_config);
 * ------------------------------------------------------------------------ */
err_t ethernetif_init(struct netif *netif)
{
    struct ethernetif *eth;
    LWIP_ASSERT("netif != NULL", netif != NULL);

    if (netif->state == NULL) {
        eth = (struct ethernetif *)mem_malloc(sizeof(struct ethernetif));
        if (eth == NULL) {
            ETHERNETIF_PRINTF("[etherif] malloc failed\r\n");
            return ERR_MEM;
        }
        memset(eth, 0, sizeof(struct ethernetif));
        netif->state = eth;
    } else {
        eth = (struct ethernetif *)netif->state;
    }

    netif->name[0] = 'd';
    netif->name[1] = 'm';

    netif->output     = etharp_output;
    netif->linkoutput = low_level_output;

    low_level_init(netif);

    ETHERNETIF_PRINTF("[etherif] init done\r\n");
    return ERR_OK;
}

/* ---------------------------------------------------------------------------
 * ethernetif_input — 輪詢 DM9051 RX 並餵入 lwIP
 *
 * 適用於 NO_SYS=1 bare-metal 主迴圈。
 * 若使用中斷，建議在 ISR 中設 flag，主迴圈檢測到 flag 後呼叫此函式。
 * ------------------------------------------------------------------------ */
err_t ethernetif_input(struct netif *netif)
{
    struct pbuf *p;

    if ((netif == NULL) || (netif->input == NULL)) return ERR_ARG;

    p = low_level_input(netif);
    if (p == NULL) return ERR_INPROGRESS;

    if (!netif_is_link_up(netif)) {
        netif_set_link_up(netif);
    }

    if (netif->input(p, netif) != ERR_OK) {
        pbuf_free(p);
        LINK_STATS_INC(link.drop);
        return ERR_IF;
    }

    LINK_STATS_INC(link.recv);
    return ERR_OK;
}

/* ---------------------------------------------------------------------------
 * ethernetif_update_config — link 狀態變更回呼
 *
 * 實際輪詢 PHY 硬體 (DM9051 NSR 暫存器 bit 6 LINKST) 並同步 lwIP 旗標。
 * 透過 netif_set_link_callback(netif, ethernetif_update_config) 註冊。
 * ------------------------------------------------------------------------ */
void ethernetif_update_config(struct netif *netif)
{
    struct ethernetif *eth = (struct ethernetif *)netif->state;
    int link_up = 0;

    if (eth != NULL) {
        /* 真正讀取 DM9051 PHY 狀態 (NSR bit 6) */
        link_up = dm9051_core_link_is_up(&eth->dev);
    }

    if (link_up) {
        if (!netif_is_link_up(netif)) {
            netif_set_link_up(netif);
            ETHERNETIF_PRINTF("[etherif] Link UP (PHY polled)\r\n");
        }
    } else {
        if (netif_is_link_up(netif)) {
            netif_set_link_down(netif);
            ETHERNETIF_PRINTF("[etherif] Link DOWN (PHY polled)\r\n");
        }
    }
}

/* ---------------------------------------------------------------------------
 * ethernetif_link_poll — 輪詢 PHY link 狀態並同步 lwIP 旗標
 *
 * 主迴圈中定期呼叫 (建議 100-500ms 間隔)，偵測實體網路連線狀態變化。
 * 內部會讀取 DM9051 NSR 暫存器 bit 6 (LINKST)，並呼叫 netif_set_link_up/down。
 * 此為 dm9051_lwip_link_poll 的標準 netif 移植版本。
 * ------------------------------------------------------------------------ */
void ethernetif_link_poll(struct netif *netif)
{
    struct ethernetif *eth;
    int link_up;

    if (netif == NULL || netif->state == NULL) {
        return;
    }

    eth = (struct ethernetif *)netif->state;
    link_up = dm9051_core_link_is_up(&eth->dev);

    if (link_up) {
        if (!netif_is_link_up(netif)) {
            netif_set_link_up(netif);
        }
    } else {
        if (netif_is_link_up(netif)) {
            netif_set_link_down(netif);
        }
    }
}

