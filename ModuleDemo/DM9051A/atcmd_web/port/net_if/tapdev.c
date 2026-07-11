#include "tapdev.h"
#include "DM9051.h"

#include "../../../dm9051_driver/adapters/uip/dm9051_uip.h"
#include "../../../dm9051_driver/core/inc/dm9051_core.h"
#include "../../../dm9051_driver/examples/uip_mh2203_demo/dm9051_uip_mh2203_demo.h"

#include "uip.h"
#include "udp_printf.h"

#include <string.h>

uint8_t DM9051_RX_INT_TRIGGER;

static struct uip_ethernetif *atcmd_web_eth;
static int atcmd_web_link_up;

static void atcmd_web_copy_uip_mac(uint8_t *mac)
{
    mac[0] = uip_ethaddr.addr[0];
    mac[1] = uip_ethaddr.addr[1];
    mac[2] = uip_ethaddr.addr[2];
    mac[3] = uip_ethaddr.addr[3];
    mac[4] = uip_ethaddr.addr[4];
    mac[5] = uip_ethaddr.addr[5];
}

void tapdev_init(void)
{
    uint8_t mac[6];

    atcmd_web_copy_uip_mac(mac);
    if (dm9051_uip_mh2203_demo_open(mac) != DM9051_OK) {
        atcmd_web_eth = 0;
        return;
    }

    atcmd_web_eth = dm9051_uip_mh2203_demo_eth();
    (void)dm9051_uip_attach(&atcmd_web_eth->dev);
}

unsigned int tapdev_read(void)
{
    if (atcmd_web_eth == 0) {
        return 0u;
    }

    return dm9051_uip_input(uip_buf, UIP_BUFSIZE);
}

void tapdev_send(void)
{
    if ((atcmd_web_eth == 0) || (uip_len == 0u)) {
        return;
    }

    (void)dm9051_uip_output(uip_buf, uip_len);

    /* 每一次實際送出的幀都要在此解鎖，否則 udp_printf 只要送出過一次
     * 就會卡在 output_active=1，之後所有 printf() 都被吃掉。 */
    udp_printf_output_done();
}

uint8_t DM9051_Read_Reg(uint8_t reg)
{
    if (reg == DM9051_NSR) {
        if (atcmd_web_eth != 0) {
            atcmd_web_link_up = dm9051_uip_link_poll(atcmd_web_eth);
        }
        return atcmd_web_link_up ? NSR_LINKST : 0u;
    }

    if (reg == DM9051_ISR) {
        return atcmd_web_link_up ? 0x20u : 0u;
    }

    return 0u;
}

void DM9051_Write_Reg(uint8_t reg, uint8_t value)
{
    (void)reg;
    (void)value;
}

int32_t DM9051_Init(void)
{
    tapdev_init();
    return (atcmd_web_eth != 0) ? 0 : -1;
}

uint32_t DM9051_TX(void)
{
    tapdev_send();
    return 0u;
}

uint16_t DM9051_RX(void)
{
    return (uint16_t)tapdev_read();
}
