#include "mh2030a_board.h"
#include "mh2030a_platform.h"
#include "dm9051_uip_mh2030a_smoke.h"
#include "../../adapters/uip/dm9051_uip.h"

#include <stdio.h>

static const uint8_t dm9051_smoke_mac[DM9051_MAC_ADDR_LENGTH] = {
    0x00u, 0x60u, 0x6Eu, 0x90u, 0x51u, 0x01u
};

static uint8_t dm9051_smoke_rx_buf[DM9051_ETH_FRAME_MAX];
static uint8_t dm9051_smoke_tx_buf[60];

static void dm9051_smoke_build_tx_frame(void)
{
    uint16_t i;

    for (i = 0u; i < 6u; ++i) {
        dm9051_smoke_tx_buf[i] = 0xffu;
    }

    for (i = 0u; i < DM9051_MAC_ADDR_LENGTH; ++i) {
        dm9051_smoke_tx_buf[6u + i] = dm9051_smoke_mac[i];
    }

    dm9051_smoke_tx_buf[12] = 0x88u;
    dm9051_smoke_tx_buf[13] = 0xB5u;

    for (i = 14u; i < sizeof(dm9051_smoke_tx_buf); ++i) {
        dm9051_smoke_tx_buf[i] = (uint8_t)i;
    }
}

void mh2030a_uip_tick_isr(void)
{
}

int main(void)
{
    const dm9051_device_t *dev;
    uint32_t loops = 0u;
    int adapter_status;
    int status;

    mh2030a_uip_board_init(115200);
    dm9051_smoke_build_tx_frame();

    printf("[DM9051 staging] MH2030A polling smoke start\r\n");

    status = dm9051_uip_mh2030a_smoke_open(dm9051_smoke_mac);
    dev = dm9051_uip_mh2030a_smoke_device();

    printf("[DM9051 staging] open status=%d found=%d VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n",
           status,
           dm9051_core_device_found(dev),
           dm9051_core_vendor_id(dev),
           dm9051_core_product_id(dev),
           dm9051_core_chip_revision(dev));

    adapter_status = dm9051_uip_attach(dm9051_uip_mh2030a_smoke_mutable_device());
    printf("[DM9051 staging] uip attach status=%d mode=%s\r\n",
           adapter_status,
           dm9051_uip_target_mode());

    while (1) {
        uint16_t rx_len;

        rx_len = dm9051_uip_input(dm9051_smoke_rx_buf,
                                  sizeof(dm9051_smoke_rx_buf));
        ++loops;

        if (rx_len != 0u) {
            printf("[DM9051 staging] rx len=%u first=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                   rx_len,
                   dm9051_smoke_rx_buf[0],
                   dm9051_smoke_rx_buf[1],
                   dm9051_smoke_rx_buf[2],
                   dm9051_smoke_rx_buf[3],
                   dm9051_smoke_rx_buf[4],
                   dm9051_smoke_rx_buf[5],
                   dm9051_smoke_rx_buf[6],
                   dm9051_smoke_rx_buf[7],
                   dm9051_smoke_rx_buf[8],
                   dm9051_smoke_rx_buf[9]);
        } else if ((loops % 5u) == 0u) {
            printf("[DM9051 staging] rx len=0\r\n");
        }

        if ((loops % 5u) == 0u) {
            status = dm9051_uip_output(dm9051_smoke_tx_buf,
                                       sizeof(dm9051_smoke_tx_buf));
            printf("[DM9051 staging] tx len=%u status=%d\r\n",
                   (unsigned int)sizeof(dm9051_smoke_tx_buf),
                   status);
        }

        Delay_Ms(1000u);
    }
}
