/*
 * Future stack-neutral DM9051 core implementation.
 *
 * Current source of truth:
 *   drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_beta.c
 *
 * Planned internal sections:
 *   - public API compatibility wrappers
 *   - init/configuration
 *   - MAC address handling
 *   - register/PHY helpers
 *   - IRQ/event state
 *   - RX path
 *   - TX path
 *
 * Keep this file out of existing builds until the compatibility shim is ready.
 */

#include "dm9051_core.h"
#include "dm9051_hal.h"

#include <stdio.h>
#include <string.h>

#ifndef DM9051_TX_WAIT_DONE
#define DM9051_TX_WAIT_DONE 1
#endif

#ifndef DM9051_TX_WAIT_TIMEOUT_US
#define DM9051_TX_WAIT_TIMEOUT_US 100000u
#endif

#ifndef DM9051_TX_WAIT_POLL_DELAY_US
#define DM9051_TX_WAIT_POLL_DELAY_US 0u
#endif

#ifndef DM9051_RXB_RESET_THRESHOLD
#define DM9051_RXB_RESET_THRESHOLD 10u
#endif

#ifndef DM9051_DIAG
#define DM9051_DIAG 1
#endif

#if DM9051_DIAG
#define DM9051_DIAG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DM9051_DIAG_PRINTF(...) do { } while (0)
#endif

/* -------------------------------------------------------------------------
 * Staging rules
 * -------------------------------------------------------------------------
 *
 * - Do not include uIP, lwIP, or MH2030A platform headers here.
 * - Keep RX/TX/PHY/IRQ in this file as internal sections for now.
 * - Move only diagnostics/logging helpers to dm9051_debug.c.
 * - Add production symbols only after running impact analysis for each public
 *   function being copied or changed.
 */

/* -------------------------------------------------------------------------
 * Driver state
 * ---------------------------------------------------------------------- */

/* Future owner for dm9051_internal.h state:
 *   - instance configuration
 *   - interrupt event flag
 *   - EXTI line
 *   - HAL binding pointer/context
 */

/* Staging state model:
 *   dm9051_device_t
 *     runtime.config
 *     runtime.current_mac
 *     runtime.irq_line
 *     runtime.interrupt_event
 *     runtime.device_found
 *     hal
 *
 * The `hal` member is intentionally opaque here because dm9051_types.h should
 * stay independent from hal/inc. dm9051_core.c will cast it to dm9051_hal_t
 * once the staged implementation is wired.
 */

static int dm9051_core_hal_status(int status)
{
    if (status == DM9051_HAL_OK) {
        return DM9051_OK;
    }

    if (status == DM9051_HAL_ERR_TIMEOUT) {
        return DM9051_ERR_TIMEOUT;
    }

    if (status == DM9051_HAL_ERR_PARAM) {
        return DM9051_ERR_PARAM;
    }

    return DM9051_ERR_NOT_READY;
}

static int dm9051_core_hal_is_valid(const dm9051_hal_t *hal)
{
    if ((hal == 0) || (hal->ops == 0)) {
        return 0;
    }

    if ((hal->ops->read_reg == 0) ||
        (hal->ops->write_reg == 0) ||
        (hal->ops->read_mem == 0) ||
        (hal->ops->write_mem == 0) ||
        (hal->ops->delay_ms == 0) ||
        (hal->ops->delay_us == 0)) {
        return 0;
    }

    return 1;
}

static int dm9051_core_mac_addr_is_valid(const uint8_t *mac_addr)
{
    uint8_t i;
    uint8_t all_zero;
    uint8_t all_one;

    if (mac_addr == 0) {
        return 0;
    }

    all_zero = 1u;
    all_one = 1u;
    for (i = 0u; i < DM9051_MAC_ADDR_LENGTH; ++i) {
        if (mac_addr[i] != 0x00u) {
            all_zero = 0u;
        }
        if (mac_addr[i] != 0xffu) {
            all_one = 0u;
        }
    }

    if ((all_zero != 0u) || (all_one != 0u)) {
        return 0;
    }

    if ((mac_addr[0] & 0x01u) != 0u) {
        return 0;
    }

    return 1;
}

static int dm9051_core_hal_supports_critical(const dm9051_hal_t *hal)
{
    if ((hal == 0) || (hal->ops == 0)) {
        return 0;
    }

    return ((hal->ops->enter_critical != 0) &&
            (hal->ops->exit_critical != 0)) ? 1 : 0;
}

static uint32_t dm9051_core_ipv4_to_u32(const uint8_t ip[4])
{
    return ((uint32_t)ip[0] << 24) |
           ((uint32_t)ip[1] << 16) |
           ((uint32_t)ip[2] << 8) |
           (uint32_t)ip[3];
}

static int dm9051_core_ipv4_is_unicast(const uint8_t ip[4])
{
    if (ip == 0) {
        return 0;
    }

    if ((ip[0] == 0u) || (ip[0] >= 224u) || (ip[0] == 127u)) {
        return 0;
    }

    if ((ip[0] == 255u) &&
        (ip[1] == 255u) &&
        (ip[2] == 255u) &&
        (ip[3] == 255u)) {
        return 0;
    }

    return 1;
}

static int dm9051_core_ipv4_is_zero(const uint8_t ip[4])
{
    if (ip == 0) {
        return 0;
    }

    return ((ip[0] == 0u) &&
            (ip[1] == 0u) &&
            (ip[2] == 0u) &&
            (ip[3] == 0u)) ? 1 : 0;
}

static int dm9051_core_netmask_is_valid(const uint8_t netmask[4])
{
    uint32_t mask;
    uint32_t inverted;

    if (netmask == 0) {
        return 0;
    }

    mask = dm9051_core_ipv4_to_u32(netmask);
    if ((mask == 0u) || (mask == 0xffffffffu)) {
        return 0;
    }

    inverted = ~mask;
    return (((inverted + 1u) & inverted) == 0u) ? 1 : 0;
}

static int dm9051_core_ipv4_host_is_valid(const uint8_t ip[4],
                                          const uint8_t netmask[4])
{
    uint32_t addr;
    uint32_t mask;
    uint32_t host;

    if (!dm9051_core_ipv4_is_unicast(ip) ||
        !dm9051_core_netmask_is_valid(netmask)) {
        return 0;
    }

    addr = dm9051_core_ipv4_to_u32(ip);
    mask = dm9051_core_ipv4_to_u32(netmask);
    host = addr & ~mask;

    if ((host == 0u) || (host == ~mask)) {
        return 0;
    }

    return 1;
}

static int dm9051_core_ipv4_same_subnet(const uint8_t a[4],
                                        const uint8_t b[4],
                                        const uint8_t netmask[4])
{
    uint32_t mask;

    if ((a == 0) || (b == 0) || !dm9051_core_netmask_is_valid(netmask)) {
        return 0;
    }

    mask = dm9051_core_ipv4_to_u32(netmask);
    return ((dm9051_core_ipv4_to_u32(a) & mask) ==
            (dm9051_core_ipv4_to_u32(b) & mask)) ? 1 : 0;
}

static int dm9051_core_read_reg(const dm9051_hal_t *hal,
                                uint8_t reg,
                                uint8_t *val)
{
    if ((hal == 0) || (hal->ops == 0) || (hal->ops->read_reg == 0)) {
        return DM9051_ERR_PARAM;
    }

    return dm9051_core_hal_status(hal->ops->read_reg(hal->ctx, reg, val));
}

static int dm9051_core_write_reg(const dm9051_hal_t *hal,
                                 uint8_t reg,
                                 uint8_t val)
{
    if ((hal == 0) || (hal->ops == 0) || (hal->ops->write_reg == 0)) {
        return DM9051_ERR_PARAM;
    }

    return dm9051_core_hal_status(hal->ops->write_reg(hal->ctx, reg, val));
}

static int dm9051_core_read_mem(const dm9051_hal_t *hal,
                                uint8_t *buf,
                                uint16_t len)
{
    if ((hal == 0) || (hal->ops == 0) || (hal->ops->read_mem == 0)) {
        return DM9051_ERR_PARAM;
    }

    return dm9051_core_hal_status(hal->ops->read_mem(hal->ctx, buf, len));
}

static int dm9051_core_write_mem(const dm9051_hal_t *hal,
                                 const uint8_t *buf,
                                 uint16_t len)
{
    if ((hal == 0) || (hal->ops == 0) || (hal->ops->write_mem == 0)) {
        return DM9051_ERR_PARAM;
    }

    return dm9051_core_hal_status(hal->ops->write_mem(hal->ctx, buf, len));
}

static uint32_t dm9051_core_enter_critical(const dm9051_hal_t *hal)
{
    if ((hal == 0) || (hal->ops == 0) || (hal->ops->enter_critical == 0)) {
        return 0u;
    }

    return hal->ops->enter_critical(hal->ctx);
}

static void dm9051_core_exit_critical(const dm9051_hal_t *hal, uint32_t state)
{
    if ((hal == 0) || (hal->ops == 0) || (hal->ops->exit_critical == 0)) {
        return;
    }

    hal->ops->exit_critical(hal->ctx, state);
}

static int dm9051_core_bus_acquire(dm9051_device_t *dev,
                                   const dm9051_hal_t *hal)
{
    uint32_t state;

    if (dev == 0) {
        return DM9051_ERR_PARAM;
    }

    state = dm9051_core_enter_critical(hal);
    if (dev->runtime.bus_busy != 0u) {
        dm9051_core_exit_critical(hal, state);
        return DM9051_ERR_NOT_READY;
    }

    dev->runtime.bus_busy = 1u;
    dm9051_core_exit_critical(hal, state);
    return DM9051_OK;
}

static void dm9051_core_bus_release(dm9051_device_t *dev,
                                    const dm9051_hal_t *hal)
{
    uint32_t state;

    if (dev == 0) {
        return;
    }

    state = dm9051_core_enter_critical(hal);
    dev->runtime.bus_busy = 0u;
    dm9051_core_exit_critical(hal, state);
}

static int dm9051_core_probe(dm9051_device_t *dev, const dm9051_hal_t *hal)
{
    uint8_t vidl;
    uint8_t vidh;
    uint8_t pidl;
    uint8_t pidh;
    uint8_t chipr;
    int status;

    if ((dev == 0) || (hal == 0)) {
        return DM9051_ERR_PARAM;
    }

    status = dm9051_core_read_reg(hal, DM9051_VIDL, &vidl);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_VIDH, &vidh);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_PIDL, &pidl);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_PIDH, &pidh);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_CHIPR, &chipr);
    if (status != DM9051_OK) {
        return status;
    }

    dev->runtime.vendor_id = (uint16_t)((uint16_t)vidl | ((uint16_t)vidh << 8));
    dev->runtime.product_id = (uint16_t)((uint16_t)pidl | ((uint16_t)pidh << 8));
    dev->runtime.chip_revision = chipr;

    if ((dev->runtime.vendor_id != DM9051_VENDOR_ID) ||
        (dev->runtime.product_id != DM9051_PRODUCT_ID)) {
        return DM9051_ERR_NOT_READY;
    }

    if (chipr == 0xffu) {
        return DM9051_ERR_NOT_READY;
    }

    dev->runtime.device_found = 1u;
    return DM9051_OK;
}

static int dm9051_core_set_par(dm9051_device_t *dev, const dm9051_hal_t *hal)
{
    uint8_t i;
    int status;

    for (i = 0u; i < DM9051_MAC_ADDR_LENGTH; ++i) {
        status = dm9051_core_write_reg(hal,
                                       (uint8_t)(DM9051_PAR + i),
                                       dev->runtime.current_mac[i]);
        if (status != DM9051_OK) {
            return status;
        }
    }

    return DM9051_OK;
}

static int dm9051_core_set_mar(const dm9051_hal_t *hal)
{
    uint8_t i;
    int status;

    for (i = 0u; i < 8u; ++i) {
        status = dm9051_core_write_reg(hal,
                                       (uint8_t)(DM9051_MAR + i),
                                       (i == 7u) ? 0x80u : 0x00u);
        if (status != DM9051_OK) {
            return status;
        }
    }

    return DM9051_OK;
}

static uint8_t dm9051_core_imr_value(const dm9051_config_t *config)
{
    if (config->interrupt_mode == DM9051_INPUT_MODE_POLL) {
        return DM9051_IMR_POL_DEFAULT;
    }

    return DM9051_IMR_INT_DEFAULT;
}

static uint8_t dm9051_core_rcr_value(const dm9051_config_t *config)
{
    uint8_t value = (uint8_t)(DM9051_RCR_DEFAULT | DM9051_RCR_RXEN);

    if (config->accept_all != 0u) {
        value = (uint8_t)(value | DM9051_RCR_ALL | DM9051_RCR_PRMSC);
    }

    return value;
}

static int dm9051_core_set_checksum(const dm9051_hal_t *hal,
                                    const dm9051_config_t *config)
{
    uint8_t tx_value = 0u;
    uint8_t rx_value = 0u;
    int status;

    if (config->tx_checksuming != 0u) {
        tx_value = DM9051_TCSCR_ALL_ENABLE;
    }

    if (config->rx_checksuming != 0u) {
        rx_value = DM9051_RCSSR_RX_ENABLE;
    }

    status = dm9051_core_write_reg(hal, DM9051_CSCR, tx_value);
    if (status != DM9051_OK) {
        return status;
    }

    return dm9051_core_write_reg(hal, DM9051_RCSSR, rx_value);
}

static int dm9051_core_phy_write_raw(const dm9051_hal_t *hal,
                                     uint16_t reg,
                                     uint16_t value)
{
    uint16_t timeout = 500u;
    uint8_t epcr;
    int status;

    status = dm9051_core_write_reg(hal, DM9051_EPAR, (uint8_t)(DM9051_PHY | reg));
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_EPDRL, (uint8_t)(value & 0xffu));
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_EPDRH, (uint8_t)(value >> 8));
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_EPCR, DM9051_EPCR_PHY_WRITE);
    if (status != DM9051_OK) {
        return status;
    }
    hal->ops->delay_us(1u);

    do {
        status = dm9051_core_read_reg(hal, DM9051_EPCR, &epcr);
        if (status != DM9051_OK) {
            return status;
        }

        if ((epcr & DM9051_EPCR_BUSY) == 0u) {
            break;
        }

        hal->ops->delay_us(1u);
        --timeout;
    } while (timeout != 0u);

    status = dm9051_core_write_reg(hal, DM9051_EPCR, 0x00u);
    if (status != DM9051_OK) {
        return status;
    }

    if (timeout == 0u) {
        return DM9051_ERR_TIMEOUT;
    }

    return DM9051_OK;
}

static int dm9051_core_phy_read_raw(const dm9051_hal_t *hal,
                                    uint16_t reg,
                                    uint16_t *value)
{
    uint16_t timeout = 500u;
    uint8_t epcr;
    uint8_t low;
    uint8_t high;
    int status;

    if (value == 0) {
        return DM9051_ERR_PARAM;
    }

    status = dm9051_core_write_reg(hal, DM9051_EPAR, (uint8_t)(DM9051_PHY | reg));
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_EPCR, DM9051_EPCR_PHY_READ);
    if (status != DM9051_OK) {
        return status;
    }
    hal->ops->delay_us(1u);

    do {
        status = dm9051_core_read_reg(hal, DM9051_EPCR, &epcr);
        if (status != DM9051_OK) {
            return status;
        }

        if ((epcr & DM9051_EPCR_BUSY) == 0u) {
            break;
        }

        hal->ops->delay_us(1u);
        --timeout;
    } while (timeout != 0u);

    status = dm9051_core_write_reg(hal, DM9051_EPCR, 0x00u);
    if (status != DM9051_OK) {
        return status;
    }

    if (timeout == 0u) {
        return DM9051_ERR_TIMEOUT;
    }

    status = dm9051_core_read_reg(hal, DM9051_EPDRL, &low);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_EPDRH, &high);
    if (status != DM9051_OK) {
        return status;
    }

    *value = (uint16_t)(((uint16_t)high << 8) | low);
    return DM9051_OK;
}

static int dm9051_core_soft_default(const dm9051_hal_t *hal,
                                    const dm9051_config_t *config)
{
    int status;

    status = dm9051_core_write_reg(hal, DM9051_MBNDRY, DM9051_BOUND_CONF_BIT);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_PPCR, DM9051_PPCR_PAUSE_COUNT);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_LMCR, DM9051_LMCR_MODE1);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_INTR, DM9051_INTR_ACTIVE_LOW);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_set_checksum(hal, config);
    if (status != DM9051_OK) {
        return status;
    }

    if (config->interrupt_mode == DM9051_INPUT_MODE_INTERRUPT_CLKOUT) {
        status = dm9051_core_write_reg(hal,
                                       DM9051_IPCOCR,
                                       (uint8_t)(DM9051_IPCOCR_CLKOUT |
                                                 DM9051_IPCOCR_DUTY_LEN));
        if (status != DM9051_OK) {
            return status;
        }
    }

    return DM9051_OK;
}

static int dm9051_core_init_device(dm9051_device_t *dev,
                                   const dm9051_hal_t *hal)
{
    int status;

    status = dm9051_core_write_reg(hal, DM9051_GPR, 0x00u);
    if (status != DM9051_OK) {
        return status;
    }
    hal->ops->delay_ms(25u);

    status = dm9051_core_write_reg(hal, DM9051_NCR, DM9051_NCR_RESET);
    if (status != DM9051_OK) {
        return status;
    }
    hal->ops->delay_ms(5u);

    status = dm9051_core_soft_default(hal, &dev->runtime.config);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_set_par(dev, hal);
    if (status != DM9051_OK) {
        return status;
    }

    return DM9051_OK;
}

static int dm9051_core_set_flow_control(const dm9051_hal_t *hal,
                                        const dm9051_config_t *config)
{
    int status;

    if (config->flow_control == 0u) {
        return DM9051_OK;
    }

    status = dm9051_core_write_reg(hal, DM9051_FCR, DM9051_FCR_DEFAULT);
    if (status != DM9051_OK) {
        return status;
    }

    return dm9051_core_phy_write_raw(hal,
                                     DM9051_PHY_ADV_REG,
                                     DM9051_PHY_ADV_FLOW_CTRL);
}

static int dm9051_core_start_receive(dm9051_device_t *dev,
                                     const dm9051_hal_t *hal)
{
    int status;

    dev->runtime.interrupt_event = 0u;

    if ((dev->runtime.config.interrupt_mode != DM9051_INPUT_MODE_POLL) &&
        (hal->ops->irq_enable != 0)) {
        hal->ops->irq_enable(hal->ctx);
    }

    status = dm9051_core_set_mar(hal);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_set_flow_control(hal, &dev->runtime.config);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_write_reg(hal,
                                   DM9051_IMR,
                                   dm9051_core_imr_value(&dev->runtime.config));
    if (status != DM9051_OK) {
        return status;
    }

    return dm9051_core_write_reg(hal,
                                 DM9051_RCR,
                                 dm9051_core_rcr_value(&dev->runtime.config));
}

static int dm9051_core_reset_and_start(dm9051_device_t *dev)
{
    const dm9051_hal_t *hal;
    int status;

    if ((dev == 0) || (dev->hal == 0)) {
        return DM9051_ERR_PARAM;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    status = dm9051_core_init_device(dev, hal);
    if (status != DM9051_OK) {
        return status;
    }

    return dm9051_core_start_receive(dev, hal);
}

static uint16_t dm9051_core_rx_pad_len(uint16_t len)
{
    return len;
}

static int dm9051_core_reset_after_error(dm9051_device_t *dev)
{
    (void)dm9051_core_reset_and_start(dev);
    return DM9051_ERR_NOT_READY;
}

static void dm9051_show_rxbstatistic(uint8_t *htc, int n)
{
    int i;

    if ((htc == 0) || (n <= 0)) {
        return;
    }

    DM9051_DIAG_PRINTF("SHW rxbStatistic, %d wrngs\r\n", n);

    for (i = 0; i < (n + 2); ++i) {
        if (((i % 32) == 0) && (i != 0)) {
            DM9051_DIAG_PRINTF("\r\n");
        }
        if (((i % 32) == 0) || ((i % 16) == 0)) {
            DM9051_DIAG_PRINTF("%02x:", i);
        }
        if ((i % 8) == 0) {
            DM9051_DIAG_PRINTF(" ");
        }
        if ((i == 0) || (i == 1)) {
            DM9051_DIAG_PRINTF("  ");
            continue;
        }

        DM9051_DIAG_PRINTF("%u ", htc[i - 2]);
    }
    DM9051_DIAG_PRINTF("\r\n");
}

/**
 * 這是 dm9051_core_rxb_error 函數的功能解說。
 *
 * 這個函數用於處理 DM9051 裝置中的 RxB 錯誤。它會接收一個 DM9051 裝置的指標和 RxB 的值作為參數。
 * 如果 RxB 的值小於 2，則會將 DM9051 裝置重置，並返回 DM9051_ERR_NOT_READY 的錯誤代碼。
 * 否則，它會計算出 RxB 在錯誤記錄陣列中的索引，並將這個索引的值加 1。如果這個索引的值小於 0xff，則會返回 DM9051_ERR_NOT_READY 的錯誤代碼。
 * 如果這個索引的值等於 0xff，則會將 DM9051 裝置重置，並返回 DM9051_ERR_NOT_READY 的錯誤代碼。
 * 最後，它會將 DM9051 裝置的錯誤記錄陣列更新後，返回 DM9051_ERR_NOT_READY 的錯誤代碼。
 *
 * @param dev DM9051 裝置的指標
 * @param rxb RxB 的值
 * @return DM9051_ERR_NOT_READY 的錯誤代碼
 */
static int dm9051_core_rxb_error(dm9051_device_t *dev, uint8_t rxb)
{
    uint8_t index;

    if (rxb < 2u) {
        DM9051_DIAG_PRINTF(" _dm9051f rxb %02x invalid, reset dm9051\r\n", rxb);
        return dm9051_core_reset_after_error(dev);
    }

    index = (uint8_t)(rxb - 2u);
    if (dev->runtime.rxb_error_hist[index] < 0xffu) {
        dev->runtime.rxb_error_hist[index]++;
    }

    DM9051_DIAG_PRINTF(" _dm9051f rxb %02x (times %2u)%c\r\n",
                       rxb,
                       dev->runtime.rxb_error_hist[index],
                       (dev->runtime.rxb_error_hist[index] == 2u) ? '*' : ' ');

    if (dev->runtime.rxb_error_hist[index] >= DM9051_RXB_RESET_THRESHOLD) {
        dm9051_show_rxbstatistic(dev->runtime.rxb_error_hist,
                                 DM9051_RXB_HIST_SIZE);
        DM9051_DIAG_PRINTF("_dm9051f rxb error accumunation times : %u\r\n",
                           DM9051_RXB_RESET_THRESHOLD);
        dev->runtime.rxb_error_hist[index] = 1u;
        return dm9051_core_reset_after_error(dev);
    }

    return DM9051_ERR_NOT_READY;
}

static int dm9051_core_rx_ready(dm9051_device_t *dev, uint8_t *ready_byte)
{
    const dm9051_hal_t *hal;
    uint8_t value;
    int status;

    if ((dev == 0) || (dev->hal == 0)) {
        return DM9051_ERR_PARAM;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    status = dm9051_core_read_reg(hal, DM9051_MRCMDX, &value);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_MRCMDX, &value);
    if (status != DM9051_OK) {
        return status;
    }

    if (ready_byte != 0) {
        *ready_byte = value;
    }

    if (value == 0u) {
        return DM9051_ERR_NOT_READY;
    }

    if (dev->runtime.config.rx_checksuming != 0u) {
        if ((value & 0x03u) != DM9051_PKT_RDY) {
            return dm9051_core_rxb_error(dev, value);
        }
    } else if (value != DM9051_PKT_RDY) {
        return dm9051_core_rxb_error(dev, value);
    }

    return DM9051_OK;
}

static int dm9051_core_rx_header(const dm9051_hal_t *hal,
                                 uint8_t header[DM9051_RX_HEAD_SIZE],
                                 uint16_t *rx_len)
{
    uint8_t rx_status;
    int status;

    if ((header == 0) || (rx_len == 0)) {
        return DM9051_ERR_PARAM;
    }

    *rx_len = 0u;

    status = dm9051_core_read_mem(hal, header, DM9051_RX_HEAD_SIZE);
    if (status != DM9051_OK) {
        return status;
    }

    rx_status = header[1];
    *rx_len = (uint16_t)((uint16_t)header[2] | ((uint16_t)header[3] << 8));

    if ((rx_status & DM9051_RSR_ERR_BITS) != 0u) {
        return DM9051_ERR;
    }

    if ((*rx_len == 0u) || (*rx_len > DM9051_ETH_FRAME_MAX)) {
        return DM9051_ERR;
    }

    return DM9051_OK;
}

static int dm9051_core_rx_discard(const dm9051_hal_t *hal, uint16_t len)
{
    uint8_t drop[32];
    uint16_t left = dm9051_core_rx_pad_len(len);
    uint16_t chunk;
    int status;

    while (left != 0u) {
        chunk = (left > sizeof(drop)) ? (uint16_t)sizeof(drop) : left;
        status = dm9051_core_read_mem(hal, drop, chunk);
        if (status != DM9051_OK) {
            return status;
        }
        left = (uint16_t)(left - chunk);
    }

    return dm9051_core_write_reg(hal, DM9051_ISR, DM9051_ISR_CLEAR_RX);
}

static uint16_t dm9051_core_tx_pad_len(uint16_t len)
{
    return len;
}

static int dm9051_core_tx_set_len(const dm9051_hal_t *hal, uint16_t len)
{
    int status;

    status = dm9051_core_write_reg(hal, DM9051_TXPLL, (uint8_t)(len & 0xffu));
    if (status != DM9051_OK) {
        return status;
    }

    return dm9051_core_write_reg(hal, DM9051_TXPLH, (uint8_t)(len >> 8));
}

#if DM9051_TX_WAIT_DONE
static int dm9051_core_tx_wait_done(const dm9051_hal_t *hal)
{
    uint32_t timeout = DM9051_TX_WAIT_TIMEOUT_US;
    uint8_t tcr;
    int status;

    do {
        status = dm9051_core_read_reg(hal, DM9051_TCR, &tcr);
        if (status != DM9051_OK) {
            return status;
        }

        if ((tcr & DM9051_TCR_TXREQ) == 0u) {
            return DM9051_OK;
        }

#if DM9051_TX_WAIT_POLL_DELAY_US
        if ((hal != 0) && (hal->ops != 0) && (hal->ops->delay_us != 0)) {
            hal->ops->delay_us(DM9051_TX_WAIT_POLL_DELAY_US);
        }
#endif
        --timeout;
    } while (timeout != 0u);

    DM9051_DIAG_PRINTF("[DM9051 core] TX wait timeout TCR=0x%02X\r\n", tcr);
    return DM9051_ERR_TIMEOUT;
}
#endif

/* -------------------------------------------------------------------------
 * Public API compatibility wrappers
 * ---------------------------------------------------------------------- */

/* Future public API location:
 *   dm9051_conf
 *   dm9051_init
 *   dm9051_rx
 *   dm9051_tx
 *   cspi_phy_read
 *   dm9051_interrupt_set
 *   dm9051_interrupt_get
 *   dm9051_interrupt_reset
 */

void dm9051_core_default_config(dm9051_config_t *config)
{
    if (config == 0) {
        return;
    }

    (void)memset(config, 0, sizeof(*config));
    config->tx_checksuming = 1u;
    config->rx_checksuming = 0u;
    config->flow_control = 1u;
    config->accept_all = 0u;
    config->interrupt_mode = DM9051_INPUT_MODE_POLL;
    config->force_stop_if_not_found = 0u;
}

int dm9051_core_config_is_valid(const dm9051_config_t *config)
{
    if (config == 0) {
        return 0;
    }

    if ((config->interrupt_mode != DM9051_INPUT_MODE_POLL) &&
        (config->interrupt_mode != DM9051_INPUT_MODE_INTERRUPT) &&
        (config->interrupt_mode != DM9051_INPUT_MODE_INTERRUPT_CLKOUT)) {
        return 0;
    }

    return 1;
}

int dm9051_netif_device_is_valid(const dm9051_netif_device_t *dev)
{
    if (dev == 0) {
        return 0;
    }

    if (!dm9051_core_mac_addr_is_valid(dev->mac_addr)) {
        return 0;
    }

    if (!dm9051_core_netmask_is_valid(dev->netmask_ip)) {
        return 0;
    }

    if (!dm9051_core_ipv4_host_is_valid(dev->static_ip, dev->netmask_ip)) {
        return 0;
    }

    if (!dm9051_core_ipv4_is_zero(dev->gateway_ip) &&
        !dm9051_core_ipv4_is_unicast(dev->gateway_ip)) {
        return 0;
    }

    if (!dm9051_core_ipv4_is_zero(dev->gateway_ip)) {
        if (!dm9051_core_ipv4_host_is_valid(dev->gateway_ip,
                                            dev->netmask_ip)) {
            return 0;
        }

        if (!dm9051_core_ipv4_same_subnet(dev->static_ip,
                                          dev->gateway_ip,
                                          dev->netmask_ip)) {
            return 0;
        }

        if (dm9051_core_ipv4_to_u32(dev->static_ip) ==
            dm9051_core_ipv4_to_u32(dev->gateway_ip)) {
            return 0;
        }
    }

    return 1;
}

int dm9051_core_open(dm9051_device_t *dev,
                     const dm9051_config_t *config,
                     struct dm9051_hal *hal)
{
    int status;

    if ((dev == 0) ||
        !dm9051_core_config_is_valid(config) ||
        !dm9051_core_hal_is_valid((const dm9051_hal_t *)hal)) {
        return DM9051_ERR_PARAM;
    }

    if ((config->interrupt_mode != DM9051_INPUT_MODE_POLL) &&
        !dm9051_core_hal_supports_critical((const dm9051_hal_t *)hal)) {
        return DM9051_ERR_PARAM;
    }

    dev->runtime.config = *config;
    dev->runtime.irq_line = 0u;
    dev->runtime.interrupt_event = 0u;
    dev->runtime.bus_busy = 0u;
    dev->runtime.device_found = 0u;
    dev->runtime.vendor_id = 0u;
    dev->runtime.product_id = 0u;
    dev->runtime.chip_revision = 0u;
    dev->hal = hal;
    (void)memset(dev->runtime.rxb_error_hist,
                 0,
                 sizeof(dev->runtime.rxb_error_hist));

    if (config->mac_addr != 0) {
        (void)memcpy(dev->runtime.current_mac,
                     config->mac_addr,
                     DM9051_MAC_ADDR_LENGTH);
    } else {
        (void)memset(dev->runtime.current_mac, 0, DM9051_MAC_ADDR_LENGTH);
    }

    if (((dm9051_hal_t *)hal)->ops->reset != 0) {
        ((dm9051_hal_t *)hal)->ops->reset(((dm9051_hal_t *)hal)->ctx);
    }

    status = dm9051_core_probe(dev, (const dm9051_hal_t *)hal);
    if (status != DM9051_OK) {
        return status;
    }

    return dm9051_core_reset_and_start(dev);
}

int dm9051_core_close(dm9051_device_t *dev)
{
    if (dev == 0) {
        return DM9051_ERR_PARAM;
    }

    (void)memset(dev, 0, sizeof(*dev));
    return DM9051_OK;
}

uint16_t dm9051_core_receive(dm9051_device_t *dev,
                             uint8_t *buf,
                             uint16_t buf_len)
{
    uint16_t rx_len;

    rx_len = 0u;
    if (dm9051_core_receive_ex(dev, buf, buf_len, &rx_len) != DM9051_OK) {
        return 0u;
    }

    return rx_len;
}

int dm9051_core_receive_ex(dm9051_device_t *dev,
                           uint8_t *buf,
                           uint16_t buf_len,
                           uint16_t *out_len)
{
    const dm9051_hal_t *hal;
    uint8_t ready_byte;
    uint8_t header[DM9051_RX_HEAD_SIZE];
    uint16_t rx_len;
    int status;

    if (out_len != 0) {
        *out_len = 0u;
    }

    if ((dev == 0) || (dev->hal == 0)) {
        return DM9051_ERR_PARAM;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    status = dm9051_core_bus_acquire(dev, hal);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_rx_ready(dev, &ready_byte);
    if (status != DM9051_OK) {
        dm9051_core_bus_release(dev, hal);
        return status;
    }

    status = dm9051_core_rx_header(hal, header, &rx_len);
    if (status != DM9051_OK) {
        if ((rx_len != 0u) && (rx_len <= DM9051_ETH_FRAME_MAX)) {
            (void)dm9051_core_rx_discard(hal, rx_len);
        } else if (status == DM9051_ERR) {
            (void)dm9051_core_reset_after_error(dev);
        }
        dm9051_core_bus_release(dev, hal);
        return status;
    }

    if ((buf == 0) || (buf_len < rx_len)) {
        (void)dm9051_core_rx_discard(hal, rx_len);
        dm9051_core_bus_release(dev, hal);
        return DM9051_ERR_PARAM;
    }

    status = dm9051_core_read_mem(hal, buf, dm9051_core_rx_pad_len(rx_len));
    if (status != DM9051_OK) {
        dm9051_core_bus_release(dev, hal);
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_ISR, DM9051_ISR_CLEAR_RX);
    if (status != DM9051_OK) {
        dm9051_core_bus_release(dev, hal);
        return status;
    }

    if (out_len != 0) {
        *out_len = rx_len;
    }

    dm9051_core_bus_release(dev, hal);
    return DM9051_OK;
}

int dm9051_core_send(dm9051_device_t *dev, const uint8_t *buf, uint16_t len)
{
    const dm9051_hal_t *hal;
    int status;

    if ((dev == 0) || (dev->hal == 0) || (buf == 0)) {
        return DM9051_ERR_PARAM;
    }

    if ((len == 0u) || (len > DM9051_ETH_FRAME_MAX)) {
        return DM9051_ERR_PARAM;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    status = dm9051_core_bus_acquire(dev, hal);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_tx_set_len(hal, len);
    if (status != DM9051_OK) {
        dm9051_core_bus_release(dev, hal);
        return status;
    }

    status = dm9051_core_write_mem(hal, buf, dm9051_core_tx_pad_len(len));
    if (status != DM9051_OK) {
        dm9051_core_bus_release(dev, hal);
        return status;
    }

    status = dm9051_core_write_reg(hal, DM9051_TCR, DM9051_TCR_TXREQ);
    if (status != DM9051_OK) {
        dm9051_core_bus_release(dev, hal);
        return status;
    }

#if DM9051_TX_WAIT_DONE
    status = dm9051_core_tx_wait_done(hal);
#else
    status = DM9051_OK;
#endif
    dm9051_core_bus_release(dev, hal);
    return status;
}

#if !DM9051_TX_WAIT_DONE
int dm9051_core_tx_poll_done(dm9051_device_t *dev)
{
    const dm9051_hal_t *hal;
    uint8_t tcr;
    int status;

    if ((dev == 0) || (dev->hal == 0)) {
        return DM9051_ERR_PARAM;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    status = dm9051_core_bus_acquire(dev, hal);
    if (status != DM9051_OK) {
        return status;
    }

    status = dm9051_core_read_reg(hal, DM9051_TCR, &tcr);
    dm9051_core_bus_release(dev, hal);
    if (status != DM9051_OK) {
        return status;
    }

    return ((tcr & DM9051_TCR_TXREQ) == 0u) ? DM9051_OK : DM9051_ERR_NOT_READY;
}
#endif

uint16_t dm9051_core_phy_read(dm9051_device_t *dev, uint16_t reg)
{
    uint16_t value;

    if ((dev == 0) || (dev->hal == 0)) {
        return 0xffffu;
    }

    value = 0xffffu;
    if (dm9051_core_phy_read_raw((const dm9051_hal_t *)dev->hal,
                                 reg,
                                 &value) != DM9051_OK) {
        return 0xffffu;
    }

    return value;
}

int dm9051_core_phy_write(dm9051_device_t *dev, uint16_t reg, uint16_t value)
{
    if ((dev == 0) || (dev->hal == 0)) {
        return DM9051_ERR_PARAM;
    }

    return dm9051_core_phy_write_raw((const dm9051_hal_t *)dev->hal,
                                     reg,
                                     value);
}

int dm9051_core_link_is_up(dm9051_device_t *dev)
{
    uint8_t nsr;

    if ((dev == 0) || (dev->hal == 0) || (dev->runtime.device_found == 0u)) {
        return 0;
    }

    if (dm9051_core_read_reg((const dm9051_hal_t *)dev->hal,
                             DM9051_NSR,
                             &nsr) != DM9051_OK) {
        return 0;
    }

    return ((nsr & DM9051_NSR_LINKST) != 0u) ? 1 : 0;
}

void dm9051_core_interrupt_set(dm9051_device_t *dev, uint32_t irq_line)
{
    if (dev == 0) {
        return;
    }

    dev->runtime.irq_line = irq_line;
    dev->runtime.interrupt_event = 1u;
}

int dm9051_core_interrupt_take(dm9051_device_t *dev)
{
    const dm9051_hal_t *hal;
    uint32_t state;
    int taken;

    if (dev == 0) {
        return 0;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    state = dm9051_core_enter_critical(hal);
    taken = 0;

    if (dev->runtime.interrupt_event == 0u) {
        dm9051_core_exit_critical(hal, state);
        return taken;
    }

    dev->runtime.interrupt_event = 0u;
    taken = 1;
    dm9051_core_exit_critical(hal, state);
    return taken;
}

void dm9051_core_interrupt_reset(dm9051_device_t *dev)
{
    const dm9051_hal_t *hal;
    uint32_t state;
    uint8_t isr;

    if (dev == 0) {
        return;
    }

    dev->runtime.interrupt_event = 0u;

    if ((dev->runtime.config.interrupt_mode == DM9051_INPUT_MODE_POLL) ||
        (dev->hal == 0)) {
        return;
    }

    hal = (const dm9051_hal_t *)dev->hal;
    state = dm9051_core_enter_critical(hal);
    dev->runtime.interrupt_event = 0u;
    dm9051_core_exit_critical(hal, state);

    if (dm9051_core_read_reg(hal, DM9051_ISR, &isr) == DM9051_OK) {
        (void)dm9051_core_write_reg(hal, DM9051_ISR, isr);
    }

    (void)dm9051_core_write_reg(hal,
                                DM9051_IMR,
                                dm9051_core_imr_value(&dev->runtime.config));

    if (hal->ops->irq_enable != 0) {
        hal->ops->irq_enable(hal->ctx);
    }
}

const uint8_t *dm9051_core_mac(const dm9051_device_t *dev)
{
    if (dev == 0) {
        return 0;
    }

    return dev->runtime.current_mac;
}

int dm9051_core_device_found(const dm9051_device_t *dev)
{
    if (dev == 0) {
        return 0;
    }

    return dev->runtime.device_found != 0u;
}

uint16_t dm9051_core_vendor_id(const dm9051_device_t *dev)
{
    if (dev == 0) {
        return 0u;
    }

    return dev->runtime.vendor_id;
}

uint16_t dm9051_core_product_id(const dm9051_device_t *dev)
{
    if (dev == 0) {
        return 0u;
    }

    return dev->runtime.product_id;
}

uint8_t dm9051_core_chip_revision(const dm9051_device_t *dev)
{
    if (dev == 0) {
        return 0u;
    }

    return dev->runtime.chip_revision;
}

/* -------------------------------------------------------------------------
 * Init / configuration
 * ---------------------------------------------------------------------- */

/* Future internal owner for:
 *   cspi_core_reset
 *   cspi_soft_default
 *   cspi_core_start1
 *   env_chip_id_and_ticks
 *   check_force_stop
 *   cspi_get_chipid
 *   cspi_get_control_status
 */

/* -------------------------------------------------------------------------
 * MAC address handling
 * ---------------------------------------------------------------------- */

/* Future internal owner for:
 *   is_zero_ether_addr
 *   validate_macaddr
 *   cspi_macaddr_process
 *   cspi_get_par
 *   cspi_set_par
 */

/* -------------------------------------------------------------------------
 * Register / PHY helpers
 * ---------------------------------------------------------------------- */

/* Future internal owner for:
 *   cspi_write_regs
 *   cspi_read_regs
 *   cspi_phy_read
 *   cspi_phy_write
 *   cspi_phycore_on
 */

/* -------------------------------------------------------------------------
 * IRQ / event state
 * ---------------------------------------------------------------------- */

/* Future internal owner for:
 *   conf_ext_line
 *   dm9051_isr_enab
 *   dm9051_imr_disab
 *   dm9051_imr_enab
 *   cspi_set_imr
 *   cspi_disble_irq
 *   cspi_enable_irq
 */

/* -------------------------------------------------------------------------
 * RX path
 * ---------------------------------------------------------------------- */

/* Future internal owner for:
 *   cspi_set_recv
 *   cspi_set_mar
 *   cspi_set_fcr
 *   cspi_set_rcr
 *   cspi_rx_ready
 *   cspi_rx_head
 *   rx_head_takelen
 *   cspi_rx_read
 *   cspi_rx_discard
 *   cspi_get_rwpa
 *   cspi_get_mrrl
 *   rx_pointers_equ
 */

/* -------------------------------------------------------------------------
 * TX path
 * ---------------------------------------------------------------------- */

/* Future internal owner for:
 *   cspi_tx_len
 *   cspi_tx_packet_len
 *   cspi_tx_write
 *   cspi_tx_req
 */
