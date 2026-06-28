#ifndef DM9051_HAL_MH2203_SPI1_PRIV_H
#define DM9051_HAL_MH2203_SPI1_PRIV_H

#include "dm9051_hal_mh2203_spi1.h"
#include "mh2203_platform.h"

#include "../../core/inc/dm9051_regs.h"

#include <stdio.h>

#define DM9051_MH2203_SPI          SPI1
#define DM9051_MH2203_CS_PORT      GPIOA
#define DM9051_MH2203_CS_PIN       GPIO_Pin_15
#define DM9051_MH2203_SCK_PORT     GPIOB
#define DM9051_MH2203_SCK_PIN      GPIO_Pin_3
#define DM9051_MH2203_MOSI_PORT    GPIOB
#define DM9051_MH2203_MOSI_PIN     GPIO_Pin_5
#define DM9051_MH2203_MISO_PORT    GPIOB
#define DM9051_MH2203_MISO_PIN     GPIO_Pin_4
#define DM9051_MH2203_RST_PORT     GPIOF
#define DM9051_MH2203_RST_PIN      GPIO_Pin_7
#define DM9051_MH2203_INT_PORT     GPIOF
#define DM9051_MH2203_INT_PIN      GPIO_Pin_6
#define DM9051_MH2203_INT_LINE     EXTI_Line6
#define DM9051_MH2203_INT_IRQn     EXTI9_5_IRQn

#if DM9051_MH2203_DIAG
#define DM9051_MH2203_DIAG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DM9051_MH2203_DIAG_PRINTF(...) do { } while (0)
#endif

#if DM9051_MH2203_TRACE
#define DM9051_MH2203_TRACE_PRINTF(...) printf(__VA_ARGS__)
#else
#define DM9051_MH2203_TRACE_PRINTF(...) do { } while (0)
#endif

extern const dm9051_hal_ops_t dm9051_mh2203_polling_ops;

void dm9051_mh2203_select(void);
void dm9051_mh2203_deselect(void);
int dm9051_mh2203_wait_spi_idle(const dm9051_mh2203_config_t *config);
int dm9051_mh2203_transfer_byte(const dm9051_mh2203_config_t *config,
                                 uint8_t tx,
                                 uint8_t *rx);
int dm9051_mh2203_finish_transfer(const dm9051_mh2203_config_t *config);
void dm9051_mh2203_spi1_polling_bus_init(void);
void dm9051_mh2203_spi1_bus_init_common(void);
void dm9051_mh2203_reset_gpio_sequence(void);
void dm9051_mh2203_delay_ms(uint32_t ms);
void dm9051_mh2203_delay_us(uint32_t us);
uint32_t dm9051_mh2203_enter_critical(void *ctx);
void dm9051_mh2203_exit_critical(void *ctx, uint32_t state);
void dm9051_mh2203_polling_reset(void *ctx);
int dm9051_mh2203_polling_read_reg(void *ctx, uint8_t reg, uint8_t *val);
int dm9051_mh2203_polling_write_reg(void *ctx, uint8_t reg, uint8_t val);
int dm9051_mh2203_polling_read_mem(void *ctx, uint8_t *buf, uint16_t len);
int dm9051_mh2203_polling_write_mem(void *ctx,
                                     const uint8_t *buf,
                                     uint16_t len);

#if DM9051_ENABLE_DMA
extern const dm9051_hal_ops_t dm9051_mh2203_dma_ops;
#endif

/* Always declared as extern — dm9051_hal_mh2203_int.c provides the definitions
 * unconditionally (runtime guard handles the no-IRQ case). This decouples the
 * declarations from per-file DM9051_MH2203_ENABLE_IRQ #define mismatches. */
void dm9051_mh2203_irq_init_if_enabled(const dm9051_mh2203_config_t *config);
void dm9051_mh2203_irq_enable_if_enabled(void *ctx);
void dm9051_mh2203_irq_disable_if_enabled(void *ctx);

#endif /* DM9051_HAL_MH2203_SPI1_PRIV_H */
