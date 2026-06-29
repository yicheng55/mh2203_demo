/*
 * Future uIP adapter implementation.
 *
 * Current source of truth:
 *   ModuleDemo/DM9051A/port/uip/dm9051_uip_adapter.c
 *
 * This staging file remains intentionally empty until the production adapter is
 * copied behind compatibility-preserving wrappers.
 */

#include "dm9051_uip.h"

#include "../../core/inc/dm9051_core.h"

static dm9051_device_t *dm9051_uip_attached_dev;
static int dm9051_uip_last_rx_status_value = DM9051_ERR_NOT_READY;

int dm9051_uip_init(const dm9051_netif_device_t *dev)
{
    if (!dm9051_netif_device_is_valid(dev)) {
        return DM9051_ERR_PARAM;
    }

    if (dm9051_uip_attached_dev == 0) {
        return DM9051_ERR_NOT_READY;
    }

    return DM9051_OK;
}

int dm9051_uip_attach(dm9051_device_t *dev)
{
    if ((dev == 0) || !dm9051_core_device_found(dev)) {
        return DM9051_ERR_PARAM;
    }

    dm9051_uip_attached_dev = dev;
    return DM9051_OK;
}

uint16_t dm9051_uip_input(uint8_t *buf, uint16_t buf_len)
{
    uint16_t rx_len;
    int status;

    if (dm9051_uip_attached_dev == 0) {
        dm9051_uip_last_rx_status_value = DM9051_ERR_NOT_READY;
        return 0u;
    }

    rx_len = 0u;
    status = dm9051_core_receive_ex(dm9051_uip_attached_dev,
                                    buf,
                                    buf_len,
                                    &rx_len);
    dm9051_uip_last_rx_status_value = status;
    if (status != DM9051_OK) {
        return 0u;
    }

    return rx_len;
}

int dm9051_uip_last_rx_status(void)
{
    return dm9051_uip_last_rx_status_value;
}

int dm9051_uip_output(const uint8_t *buf, uint16_t len)
{
    if (dm9051_uip_attached_dev == 0) {
        return DM9051_ERR_NOT_READY;
    }

    return dm9051_core_send(dm9051_uip_attached_dev, buf, len);
}

int dm9051_uip_interrupt_mode(void)
{
    if (dm9051_uip_attached_dev == 0) {
        return DM9051_INPUT_MODE_POLL;
    }

    return dm9051_uip_attached_dev->runtime.config.interrupt_mode;
}

int dm9051_uip_interrupt_take(void)
{
    if (dm9051_uip_attached_dev == 0) {
        return 0;
    }

    if (dm9051_uip_attached_dev->runtime.config.interrupt_mode == DM9051_INPUT_MODE_POLL) {
        return 1;
    }

    return dm9051_core_interrupt_take(dm9051_uip_attached_dev);
}

void dm9051_uip_interrupt_reset(void)
{
    if (dm9051_uip_attached_dev == 0) {
        return;
    }

    dm9051_core_interrupt_reset(dm9051_uip_attached_dev);
}

void dm9051_uip_poll(void)
{
#if !DM9051_TX_WAIT_DONE
    if (dm9051_uip_attached_dev != 0) {
        (void)dm9051_core_tx_poll_done(dm9051_uip_attached_dev);
    }
#endif
}

int dm9051_uip_link_poll(struct uip_ethernetif *eth)
{
    if ((eth == 0) || !dm9051_core_device_found(&eth->dev)) {
        return 0;
    }

    return dm9051_core_link_is_up(&eth->dev);
}

const char *dm9051_uip_target_mode(void)
{
    /* 依實際 interrupt 設定回報模式：interrupt 關閉時顯示 polling。 */
    switch (dm9051_uip_interrupt_mode()) {
    case DM9051_INPUT_MODE_INTERRUPT:
        return "interrupt";
    case DM9051_INPUT_MODE_INTERRUPT_CLKOUT:
        return "interrupt+clkout";
    case DM9051_INPUT_MODE_POLL:
    default:
        return "polling";
    }
}
