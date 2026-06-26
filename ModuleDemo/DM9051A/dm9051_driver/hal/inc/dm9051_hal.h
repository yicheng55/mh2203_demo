#ifndef DM9051_HAL_H
#define DM9051_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Staging copy of the portable HAL contract. The current production driver
 * still uses drivers/dm9051_edriver_v1.6.1a_beta/include/dm9051_hal.h. */

#define DM9051_HAL_OK             0
#define DM9051_HAL_ERR           -1
#define DM9051_HAL_ERR_TIMEOUT   -2
#define DM9051_HAL_ERR_PARAM     -3
#define DM9051_HAL_ERR_NOT_READY -4

typedef struct dm9051_hal_ops {
    /* Register access: reg is the DM9051 register address without SPI opcode. */
    int (*read_reg)(void *ctx, uint8_t reg, uint8_t *val);
    int (*write_reg)(void *ctx, uint8_t reg, uint8_t val);

    /* FIFO access: read_mem uses MRCMD, write_mem uses MWCMD. */
    int (*read_mem)(void *ctx, uint8_t *buf, uint16_t len);
    int (*write_mem)(void *ctx, const uint8_t *buf, uint16_t len);

    /* Optional hardware reset and timing hooks. */
    void (*reset)(void *ctx);
    void (*delay_ms)(uint32_t ms);
    void (*delay_us)(uint32_t us);

    /* MCU IRQ gating for the DM9051 INT line. */
    void (*irq_enable)(void *ctx);
    void (*irq_disable)(void *ctx);

    /* Optional critical section hooks. Return value is passed to exit. */
    uint32_t (*enter_critical)(void *ctx);
    void (*exit_critical)(void *ctx, uint32_t state);
} dm9051_hal_ops_t;

typedef struct dm9051_hal {
    const dm9051_hal_ops_t *ops;
    void *ctx;
} dm9051_hal_t;

#ifdef __cplusplus
}
#endif

#endif /* DM9051_HAL_H */
