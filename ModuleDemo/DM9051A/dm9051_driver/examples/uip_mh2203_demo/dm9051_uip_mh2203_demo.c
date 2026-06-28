#include "dm9051_uip_mh2203_demo.h"
#include "../../ports/mh2203/dm9051_hal_mh2203_spi1.h"
#include "../../core/inc/dm9051_core.h"
#include "../../hal/inc/dm9051_hal.h"
#if DM9051_USE_IRQ
#include "../../ports/mh2203/dm9051_hal_mh2203_int.h"
#endif
#include "mh2203_board.h"

#include <stdio.h>

#if DM9051_USE_DMA
#pragma message("[DM9051 MH2203 uIP build] demo: DM9051_USE_DMA=1")
#else
#pragma message("[DM9051 MH2203 uIP build] demo: DM9051_USE_DMA=0")
#endif

#if DM9051_USE_IRQ
#pragma message("[DM9051 MH2203 uIP build] demo: DM9051_USE_IRQ=1")
#else
#pragma message("[DM9051 MH2203 uIP build] demo: DM9051_USE_IRQ=0")
#endif

/* netif 私有資料 (dev + hal + rx/tx buffer)，供 uIP stack 共用。 */
static struct uip_ethernetif s_demo_eth_inst;
static int s_demo_status = DM9051_ERR_NOT_READY;

/* 純探測用的預設 MAC；整合路徑請改用 dm9051_uip_mh2203_demo_open(mac)。 */
static const uint8_t s_demo_default_mac[DM9051_MAC_ADDR_LENGTH] = {
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

int dm9051_uip_mh2203_demo_open(const uint8_t *mac_addr)
{
    dm9051_config_t core_config;
    dm9051_mh2203_config_t port_config;
    int status;

    if (mac_addr == 0) {
        s_demo_status = DM9051_ERR_PARAM;
        return s_demo_status;
    }

    dm9051_core_default_config(&core_config);
    core_config.mac_addr = mac_addr;
#if DM9051_USE_IRQ
    core_config.interrupt_mode = DM9051_INPUT_MODE_INTERRUPT;
#else
    core_config.interrupt_mode = DM9051_INPUT_MODE_POLL;
#endif
    core_config.flow_control = 0u;

    dm9051_mh2203_default_config(&port_config);
#if DM9051_USE_DMA
    port_config.transport = DM9051_MH2203_TRANSPORT_DMA;
#else
    port_config.transport = DM9051_MH2203_TRANSPORT_POLLING;
#endif
#if DM9051_USE_IRQ
    port_config.irq_mode = DM9051_MH2203_IRQ_EXTI;
#else
    port_config.irq_mode = DM9051_MH2203_IRQ_OFF;
#endif

    status = dm9051_mh2203_hal_bind(&s_demo_eth_inst.hal, &port_config);
    if (status != DM9051_HAL_OK) {
        s_demo_status =
            dm9051_uip_mh2203_hal_status_to_core_status(status);
        return s_demo_status;
    }

#if DM9051_USE_IRQ
    dm9051_mh2203_irq_attach_device(&s_demo_eth_inst.dev);
#endif

    s_demo_status = dm9051_core_open(&s_demo_eth_inst.dev,
                                     &core_config,
                                     &s_demo_eth_inst.hal);
#if DM9051_USE_IRQ
    if (s_demo_status != DM9051_OK) {
        dm9051_mh2203_irq_detach_device();
    }
#endif

    return s_demo_status;
}

struct uip_ethernetif *dm9051_uip_mh2203_demo_eth(void)
{
    return &s_demo_eth_inst;
}

const dm9051_device_t *dm9051_uip_mh2203_demo_device(void)
{
    return &s_demo_eth_inst.dev;
}

dm9051_device_t *dm9051_uip_mh2203_demo_mutable_device(void)
{
    return &s_demo_eth_inst.dev;
}

int dm9051_uip_mh2203_demo_last_status(void)
{
    return s_demo_status;
}

int dm9051_uip_mh2203_demo_init(void)
{
    int status;

    mh2203_uip_board_init(115200u);

    status = dm9051_uip_mh2203_demo_open(s_demo_default_mac);
    if (status != DM9051_OK) {
        printf("[MH2203 demo] DM9051 open failed: %d\r\n", status);
        return status;
    }

    printf("VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n",
           dm9051_core_vendor_id(&s_demo_eth_inst.dev),
           dm9051_core_product_id(&s_demo_eth_inst.dev),
           dm9051_core_chip_revision(&s_demo_eth_inst.dev));
    printf("DM9051 found and opened successfully!\r\n");

    return DM9051_OK;
}
