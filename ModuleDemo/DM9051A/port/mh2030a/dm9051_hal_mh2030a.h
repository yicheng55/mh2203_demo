/**
 * @file  dm9051_hal_mh2030a.h
 * @brief MH2030A vtable binding for the DM9051 HAL ops.
 *
 * This file lives in ModuleDemo/DM9051A/port/mh2030a/ — the platform-specific
 * port layer.  It is NOT included by the DM9051 core driver.  Only code that
 * needs to construct or initialise the dm9051_hal_t struct (e.g. a future
 * abstraction layer or test harness) should include this header.
 *
 * The original copy at drivers/dm9051_edriver_v1.6.1a_beta/hal/ is now
 * superseded by this file.  The drivers/ copy is kept temporarily for
 * backward compatibility but will be removed in a follow-up cleanup.
 */

#ifndef __DM9051_HAL_MH2030A_PORT_H
#define __DM9051_HAL_MH2030A_PORT_H

/* Pull in the vtable struct definition and the flat HAL function declarations
 * that the inline wrappers below will delegate to. */
#include "../../../drivers/dm9051_edriver_v1.6.1a_beta/include/dm9051_hal.h"
#include "../../../drivers/dm9051_edriver_v1.6.1a_beta/include/dm9051_hal_api.h"
#include "hal_mh2030a.h"

#ifdef __cplusplus
extern "C" {
#endif

static __inline int dm9051_hal_mh2030a_read_reg(void *ctx, uint8_t reg, uint8_t *val)
{
    (void)ctx;
    if (val == 0) {
        return DM9051_HAL_ERR_PARAM;
    }
    *val = hal_read_reg(reg);
    return DM9051_HAL_OK;
}

static __inline int dm9051_hal_mh2030a_write_reg(void *ctx, uint8_t reg, uint8_t val)
{
    (void)ctx;
    hal_write_reg(reg, val);
    return DM9051_HAL_OK;
}

static __inline int dm9051_hal_mh2030a_read_mem(void *ctx, uint8_t *buf, uint16_t len)
{
    (void)ctx;
    if ((buf == 0) && (len != 0u)) {
        return DM9051_HAL_ERR_PARAM;
    }
    hal_read_mem(buf, len);
    return DM9051_HAL_OK;
}

static __inline int dm9051_hal_mh2030a_write_mem(void *ctx, const uint8_t *buf, uint16_t len)
{
    (void)ctx;
    if ((buf == 0) && (len != 0u)) {
        return DM9051_HAL_ERR_PARAM;
    }
    hal_write_mem(buf, len);
    return DM9051_HAL_OK;
}

static __inline void dm9051_hal_mh2030a_delay_ms(uint32_t ms)
{
    while (ms > 0xffffu) {
        Delay_Ms(0xffffu);
        ms -= 0xffffu;
    }
    if (ms != 0u) {
        Delay_Ms((uint16_t)ms);
    }
}

static __inline void dm9051_hal_mh2030a_delay_us(uint32_t us)
{
    Delay_Us(us);
}

static __inline void dm9051_hal_mh2030a_irq_enable(void *ctx)
{
    (void)ctx;
    hal_enable_mcu_irq();
}

static __inline void dm9051_hal_mh2030a_irq_disable(void *ctx)
{
    (void)ctx;
    hal_disable_mcu_irq();
}

static __inline uint32_t dm9051_hal_mh2030a_enter_critical(void *ctx)
{
    uint32_t primask;
    (void)ctx;
    primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static __inline void dm9051_hal_mh2030a_exit_critical(void *ctx, uint32_t state)
{
    (void)ctx;
    __set_PRIMASK(state);
}

/**
 * @brief  Bind an MH2030A-specific dm9051_hal_t instance.
 * @param  hal  Pointer to an uninitialised dm9051_hal_t struct.
 * @param  ctx  Optional user context pointer (pass NULL for bare-metal use).
 */
static __inline void dm9051_hal_mh2030a_bind(dm9051_hal_t *hal, void *ctx)
{
    static const dm9051_hal_ops_t ops = {
        dm9051_hal_mh2030a_read_reg,
        dm9051_hal_mh2030a_write_reg,
        dm9051_hal_mh2030a_read_mem,
        dm9051_hal_mh2030a_write_mem,
        0,                                   /* reset — handled by hal_spi_initialize */
        dm9051_hal_mh2030a_delay_ms,
        dm9051_hal_mh2030a_delay_us,
        dm9051_hal_mh2030a_irq_enable,
        dm9051_hal_mh2030a_irq_disable,
        dm9051_hal_mh2030a_enter_critical,
        dm9051_hal_mh2030a_exit_critical
    };

    if (hal != 0) {
        hal->ops = &ops;
        hal->ctx = ctx;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __DM9051_HAL_MH2030A_PORT_H */
