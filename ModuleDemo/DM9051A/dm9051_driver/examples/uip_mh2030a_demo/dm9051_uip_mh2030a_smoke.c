/*
 * Staged MH2030A/uIP smoke glue.
 *
 * This file belongs to the example layer. It may bind the MH2030A port to the
 * stack-neutral DM9051 core, but it must not replace the production uIP path.
 */

#include "dm9051_uip_mh2030a_smoke.h"

#include "../../core/inc/dm9051_core.h"
#include "../../hal/inc/dm9051_hal.h"
#include "../../ports/mh2030a/dm9051_hal_mh2030a_spi1.h"
#if DM9051_MH2030A_USE_IRQ
#include "../../ports/mh2030a/dm9051_hal_mh2030a_int.h"
#endif

static struct uip_ethernetif s_smoke_eth_inst;
static int dm9051_uip_mh2030a_smoke_status = DM9051_ERR_NOT_READY;

static int dm9051_uip_mh2030a_hal_status_to_core_status(int status)
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

int dm9051_uip_mh2030a_smoke_open(const uint8_t *mac_addr)
{
    dm9051_config_t core_config;
    dm9051_mh2030a_config_t port_config;
    int status;

    dm9051_core_default_config(&core_config);
    core_config.mac_addr = mac_addr;
#if DM9051_MH2030A_USE_IRQ
    core_config.interrupt_mode = DM9051_INPUT_MODE_INTERRUPT;
#else
    core_config.interrupt_mode = DM9051_INPUT_MODE_POLL;
#endif
    core_config.flow_control = 0u;

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

    status = dm9051_mh2030a_hal_bind(&s_smoke_eth_inst.hal,
                                     &port_config);
    if (status != DM9051_HAL_OK) {
        dm9051_uip_mh2030a_smoke_status =
            dm9051_uip_mh2030a_hal_status_to_core_status(status);
        return dm9051_uip_mh2030a_smoke_status;
    }

#if DM9051_MH2030A_USE_IRQ
    dm9051_mh2030a_irq_attach_device(&s_smoke_eth_inst.dev);
#endif

    dm9051_uip_mh2030a_smoke_status =
        dm9051_core_open(&s_smoke_eth_inst.dev,
                         &core_config,
                         &s_smoke_eth_inst.hal);
#if DM9051_MH2030A_USE_IRQ
    if (dm9051_uip_mh2030a_smoke_status != DM9051_OK) {
        dm9051_mh2030a_irq_detach_device();
    }
#endif

    return dm9051_uip_mh2030a_smoke_status;
}

struct uip_ethernetif *dm9051_uip_mh2030a_smoke_eth(void)
{
    return &s_smoke_eth_inst;
}

const dm9051_device_t *dm9051_uip_mh2030a_smoke_device(void)
{
    return &s_smoke_eth_inst.dev;
}

dm9051_device_t *dm9051_uip_mh2030a_smoke_mutable_device(void)
{
    return &s_smoke_eth_inst.dev;
}

int dm9051_uip_mh2030a_smoke_last_status(void)
{
    return dm9051_uip_mh2030a_smoke_status;
}
