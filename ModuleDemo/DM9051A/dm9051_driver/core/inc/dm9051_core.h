#ifndef DM9051_CORE_H
#define DM9051_CORE_H

#include <stdint.h>

#include "dm9051_regs.h"
#include "dm9051_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DM9051_TX_WAIT_DONE
#define DM9051_TX_WAIT_DONE 1
#endif

struct dm9051_hal;

/* Staging snapshot of the current public DM9051 core API.
 * Current source of truth:
 *   drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051.h
 *
 * This header is intentionally not included by existing targets yet. */

/* -------------------------------------------------------------------------
 * Future context-based API
 * ---------------------------------------------------------------------- */

void dm9051_core_default_config(dm9051_config_t *config);
int dm9051_core_config_is_valid(const dm9051_config_t *config);
int dm9051_netif_device_is_valid(const dm9051_netif_device_t *dev);

int dm9051_core_open(dm9051_device_t *dev,
                     const dm9051_config_t *config,
                     struct dm9051_hal *hal);
int dm9051_core_close(dm9051_device_t *dev);

uint16_t dm9051_core_receive(dm9051_device_t *dev,
                             uint8_t *buf,
                             uint16_t buf_len);
int dm9051_core_receive_ex(dm9051_device_t *dev,
                           uint8_t *buf,
                           uint16_t buf_len,
                           uint16_t *out_len);
int dm9051_core_send(dm9051_device_t *dev,
                     const uint8_t *buf,
                     uint16_t len);
#if !DM9051_TX_WAIT_DONE
/* Useful when DM9051_TX_WAIT_DONE is overridden to 0 for non-blocking TX. */
int dm9051_core_tx_poll_done(dm9051_device_t *dev);
#endif

uint16_t dm9051_core_phy_read(dm9051_device_t *dev, uint16_t reg);
int dm9051_core_phy_write(dm9051_device_t *dev, uint16_t reg, uint16_t value);
int dm9051_core_link_is_up(dm9051_device_t *dev);

void dm9051_core_interrupt_set(dm9051_device_t *dev, uint32_t irq_line);
int dm9051_core_interrupt_take(dm9051_device_t *dev);
void dm9051_core_interrupt_reset(dm9051_device_t *dev);

const uint8_t *dm9051_core_mac(const dm9051_device_t *dev);
int dm9051_core_device_found(const dm9051_device_t *dev);
uint16_t dm9051_core_vendor_id(const dm9051_device_t *dev);
uint16_t dm9051_core_product_id(const dm9051_device_t *dev);
uint8_t dm9051_core_chip_revision(const dm9051_device_t *dev);

/* -------------------------------------------------------------------------
 * Legacy compatibility API
 * ---------------------------------------------------------------------- */

int dm9051_conf(void);
const uint8_t *dm9051_init(const uint8_t *adr);
uint16_t dm9051_rx(uint8_t *buf, uint16_t buf_len);
void dm9051_tx(uint8_t *buf, uint16_t len);
uint16_t cspi_phy_read(uint16_t reg);

void dm9051_interrupt_set(uint32_t exint_line);
int dm9051_interrupt_get(void);
void dm9051_interrupt_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_CORE_H */
