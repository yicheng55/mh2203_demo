#include "mh2203_uip_clock.h"
#include "mh2203_platform.h"

#include "clock.h"

#include <stdio.h>

static volatile uint32_t mh2203_uip_elapsed_ms;

void mh2203_uip_tick_init(void)
{
    if (SysTick_Config(SystemCoreClock / (1000u / MH2203_UIP_TICK_MS)) != 0u) {
        printf("[MH2203 uIP] SysTick config failed\r\n");
    }
}

void mh2203_uip_tick_isr(void)
{
    mh2203_uip_update_time();
}

void mh2203_uip_update_time(void)
{
    mh2203_uip_elapsed_ms += MH2203_UIP_TICK_MS;
}

uint32_t mh2203_uip_millis(void)
{
    return mh2203_uip_elapsed_ms;
}

clock_time_t clock_time(void)
{
    return (clock_time_t)mh2203_uip_elapsed_ms;
}
