#ifndef DM9051_TYPES_H
#define DM9051_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM9051_MAC_ADDR_LENGTH    6u
#define DM9051_ETH_FRAME_MAX      1514u
#define DM9051_RX_HEAD_SIZE       4u
#define DM9051_RX_BUFFER_SIZE     (DM9051_ETH_FRAME_MAX + DM9051_RX_HEAD_SIZE)
#define DM9051_RXB_HIST_SIZE      254u

#define DM9051_INPUT_MODE_POLL              0
#define DM9051_INPUT_MODE_INTERRUPT         1
#define DM9051_INPUT_MODE_INTERRUPT_CLKOUT  2

#if defined(DM9051_USE_UIP) && defined(DM9051_USE_LWIP)
#error "DM9051 driver supports only one active stack owner per device instance"
#endif

#define DM9051_OK                 0
#define DM9051_ERR               -1
#define DM9051_ERR_TIMEOUT       -2
#define DM9051_ERR_PARAM         -3
#define DM9051_ERR_NOT_READY     -4

typedef uint8_t dm9051_mac_t[DM9051_MAC_ADDR_LENGTH];

typedef struct dm9051_config {
    const uint8_t *mac_addr;
    uint8_t tx_checksuming;
    uint8_t rx_checksuming;
    uint8_t flow_control;
    uint8_t accept_all;
    uint8_t interrupt_mode;
    uint8_t force_stop_if_not_found;
} dm9051_config_t;

typedef struct dm9051_runtime {
    dm9051_config_t config;
    dm9051_mac_t current_mac;
    uint16_t vendor_id;
    uint16_t product_id;
    uint32_t irq_line;
    volatile uint8_t interrupt_event;
    volatile uint8_t bus_busy;
    uint8_t rxb_error_hist[DM9051_RXB_HIST_SIZE];
    uint8_t chip_revision;
    uint8_t device_found;
} dm9051_runtime_t;

typedef struct dm9051_device {
    dm9051_runtime_t runtime;
    void *hal;
} dm9051_device_t;

typedef struct dm9051_netif_device {
    const uint8_t *mac_addr;
    uint8_t static_ip[4];
    uint8_t gateway_ip[4];
    uint8_t netmask_ip[4];
} dm9051_netif_device_t;

#ifdef __cplusplus
}
#endif

#endif /* DM9051_TYPES_H */
