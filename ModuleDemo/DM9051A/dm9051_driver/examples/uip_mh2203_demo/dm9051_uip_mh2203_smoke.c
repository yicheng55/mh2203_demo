#include "dm9051_uip_mh2203_smoke.h"
#include "../../ports/mh2203/dm9051_hal_mh2203_spi1.h"
#include "../../core/inc/dm9051_core.h"
#include "mh2203_board.h"

#include <stdio.h>

static dm9051_device_t dm9051_mh2203_smoke_device;

int dm9051_uip_mh2203_smoke_init(void)
{
    dm9051_hal_t hal;
    dm9051_config_t config;
    dm9051_mh2203_config_t port_config;
    int status;

    mh2203_uip_board_init(115200u);
    mh2203_uip_clock_init();

    dm9051_core_default_config(&config);

    dm9051_mh2203_default_config(&port_config);
    port_config.transport = DM9051_MH2203_TRANSPORT_POLLING;
    port_config.irq_mode = DM9051_MH2203_IRQ_OFF;

    status = dm9051_mh2203_hal_bind(&hal, &port_config);
    if (status != DM9051_OK) {
        printf("[MH2203 smoke] HAL bind failed: %d\r\n", status);
        return status;
    }

    status = dm9051_core_open(&dm9051_mh2203_smoke_device, &config, &hal);
    if (status != DM9051_OK) {
        printf("[MH2203 smoke] DM9051 open failed: %d\r\n", status);
        return status;
    }

    printf("VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n",
           dm9051_core_vendor_id(&dm9051_mh2203_smoke_device),
           dm9051_core_product_id(&dm9051_mh2203_smoke_device),
           dm9051_core_chip_revision(&dm9051_mh2203_smoke_device));
    printf("DM9051 found and opened successfully!\r\n");

    return DM9051_OK;
}
