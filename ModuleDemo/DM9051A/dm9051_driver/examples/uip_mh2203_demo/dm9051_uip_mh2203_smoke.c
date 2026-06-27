#include "dm9051_uip_mh2203_smoke.h"
#include "../../ports/mh2203/dm9051_hal_mh2203_spi1.h"
#include "../../core/inc/dm9051_core.h"
#include "../../hal/inc/dm9051_hal.h"
#if DM9051_MH2203_USE_IRQ
#include "../../ports/mh2203/dm9051_hal_mh2203_int.h"
#endif
#include "mh2203_board.h"

#include <stdio.h>

/* netif 私有資料 (dev + hal + rx/tx buffer)，供 uIP stack 共用。 */
static struct uip_ethernetif s_smoke_eth_inst;
static int s_smoke_status = DM9051_ERR_NOT_READY;

/* 純探測用的預設 MAC；整合路徑請改用 dm9051_uip_mh2203_smoke_open(mac)。 */
static const uint8_t s_smoke_default_mac[DM9051_MAC_ADDR_LENGTH] = {
    0x00u, 0x60u, 0x6Eu, 0x90u, 0x51u, 0x01u
};

static int dm9051_uip_mh2203_hal_status_to_core_status(int status)
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

int dm9051_uip_mh2203_smoke_open(const uint8_t *mac_addr)
{
    dm9051_config_t core_config;
    dm9051_mh2203_config_t port_config;
    int status;

    if (mac_addr == 0) {
        s_smoke_status = DM9051_ERR_PARAM;
        return s_smoke_status;
    }

    dm9051_core_default_config(&core_config);
    core_config.mac_addr = mac_addr;
#if DM9051_MH2203_USE_IRQ
    core_config.interrupt_mode = DM9051_INPUT_MODE_INTERRUPT;
#else
    core_config.interrupt_mode = DM9051_INPUT_MODE_POLL;
#endif
    core_config.flow_control = 0u;

    dm9051_mh2203_default_config(&port_config);
#if DM9051_MH2203_USE_DMA
    port_config.transport = DM9051_MH2203_TRANSPORT_DMA;
#else
    port_config.transport = DM9051_MH2203_TRANSPORT_POLLING;
#endif
#if DM9051_MH2203_USE_IRQ
    port_config.irq_mode = DM9051_MH2203_IRQ_EXTI;
#else
    port_config.irq_mode = DM9051_MH2203_IRQ_OFF;
#endif

    status = dm9051_mh2203_hal_bind(&s_smoke_eth_inst.hal, &port_config);
    if (status != DM9051_HAL_OK) {
        s_smoke_status =
            dm9051_uip_mh2203_hal_status_to_core_status(status);
        return s_smoke_status;
    }

#if DM9051_MH2203_USE_IRQ
    dm9051_mh2203_irq_attach_device(&s_smoke_eth_inst.dev);
#endif

    s_smoke_status = dm9051_core_open(&s_smoke_eth_inst.dev,
                                      &core_config,
                                      &s_smoke_eth_inst.hal);
#if DM9051_MH2203_USE_IRQ
    if (s_smoke_status != DM9051_OK) {
        dm9051_mh2203_irq_detach_device();
    }
#endif

    return s_smoke_status;
}

struct uip_ethernetif *dm9051_uip_mh2203_smoke_eth(void)
{
    return &s_smoke_eth_inst;
}

const dm9051_device_t *dm9051_uip_mh2203_smoke_device(void)
{
    return &s_smoke_eth_inst.dev;
}

dm9051_device_t *dm9051_uip_mh2203_smoke_mutable_device(void)
{
    return &s_smoke_eth_inst.dev;
}

int dm9051_uip_mh2203_smoke_last_status(void)
{
    return s_smoke_status;
}

int dm9051_uip_mh2203_smoke_init(void)
{
    int status;

    mh2203_uip_board_init(115200u);

    status = dm9051_uip_mh2203_smoke_open(s_smoke_default_mac);
    if (status != DM9051_OK) {
        printf("[MH2203 smoke] DM9051 open failed: %d\r\n", status);
        return status;
    }

    printf("VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n",
           dm9051_core_vendor_id(&s_smoke_eth_inst.dev),
           dm9051_core_product_id(&s_smoke_eth_inst.dev),
           dm9051_core_chip_revision(&s_smoke_eth_inst.dev));
    printf("DM9051 found and opened successfully!\r\n");

    return DM9051_OK;
}
