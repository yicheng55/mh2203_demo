#ifndef DM9051_HAL_MH2030A_SPI1_H
#define DM9051_HAL_MH2030A_SPI1_H

#include <stdint.h>

#include "../../core/inc/dm9051_types.h"
#include "../../hal/inc/dm9051_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MH2030A port binding for the DM9051 HAL interface.
 *
 * File split:
 *   dm9051_hal_mh2030a_spi1.c      - SPI1 polling, GPIO, delay, HAL bind
 *   dm9051_hal_mh2030a_spi1_dma.c  - optional SPI1 DMA FIFO transport
 *   dm9051_hal_mh2030a_int.c       - optional PF6/EXTI6 interrupt support
 */

#ifndef DM9051_MH2030A_USE_DMA
#define DM9051_MH2030A_USE_DMA 0
#endif

#ifndef DM9051_MH2030A_USE_IRQ
#define DM9051_MH2030A_USE_IRQ 0
#endif

#ifndef DM9051_MH2030A_ENABLE_DMA
#define DM9051_MH2030A_ENABLE_DMA DM9051_MH2030A_USE_DMA
#endif

#ifndef DM9051_MH2030A_ENABLE_IRQ
#define DM9051_MH2030A_ENABLE_IRQ DM9051_MH2030A_USE_IRQ
#endif

#ifndef DM9051_MH2030A_DIAG
#define DM9051_MH2030A_DIAG 1
#endif

#ifndef DM9051_MH2030A_TRACE
#define DM9051_MH2030A_TRACE 0
#endif

#ifndef DM9051_MH2030A_OWN_EXTI4_15_HANDLER
#define DM9051_MH2030A_OWN_EXTI4_15_HANDLER 1
#endif

typedef enum dm9051_mh2030a_transport {
    DM9051_MH2030A_TRANSPORT_POLLING = 0,
    DM9051_MH2030A_TRANSPORT_DMA = 1
} dm9051_mh2030a_transport_t;

typedef enum dm9051_mh2030a_irq_mode {
    DM9051_MH2030A_IRQ_OFF = 0,
    DM9051_MH2030A_IRQ_EXTI = 1
} dm9051_mh2030a_irq_mode_t;

typedef struct dm9051_mh2030a_pins {
    uint32_t cs_port;
    uint16_t cs_pin;
    uint32_t sck_port;
    uint16_t sck_pin;
    uint32_t miso_port;
    uint16_t miso_pin;
    uint32_t mosi_port;
    uint16_t mosi_pin;
    uint32_t rst_port;
    uint16_t rst_pin;
    uint32_t int_port;
    uint16_t int_pin;
} dm9051_mh2030a_pins_t;

typedef struct dm9051_mh2030a_config {
    dm9051_mh2030a_transport_t transport;
    dm9051_mh2030a_irq_mode_t irq_mode;
    dm9051_mh2030a_pins_t pins;
    uint32_t spi_timeout;
} dm9051_mh2030a_config_t;

#define DM9051_MH2030A_DEFAULT_SPI_TIMEOUT 1000000u

void dm9051_mh2030a_default_config(dm9051_mh2030a_config_t *config);
int dm9051_mh2030a_config_is_valid(const dm9051_mh2030a_config_t *config);
int dm9051_mh2030a_hal_bind(dm9051_hal_t *hal,
                             const dm9051_mh2030a_config_t *config);
const char *dm9051_mh2030a_transport_name(dm9051_mh2030a_transport_t transport);
const char *dm9051_mh2030a_irq_name(dm9051_mh2030a_irq_mode_t irq_mode);

/* Optional IRQ helpers. Link dm9051_hal_mh2030a_int.c and define
 * DM9051_MH2030A_ENABLE_IRQ=1 before calling these. */
void dm9051_mh2030a_irq_attach_device(dm9051_device_t *dev);
void dm9051_mh2030a_irq_detach_device(void);
uint32_t dm9051_mh2030a_irq_line(void);
void dm9051_mh2030a_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_HAL_MH2030A_SPI1_H */
