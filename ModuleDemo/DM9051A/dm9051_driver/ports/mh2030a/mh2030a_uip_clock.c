#include "mh2030a_uip_clock.h"
#include "mh2030a_platform.h"

#include "clock.h"

#include <stdio.h>

static volatile uint32_t mh2030a_uip_elapsed_ms;

void mh2030a_uip_tick_init(void)
{
    if (SysTick_Config(SystemCoreClock / (1000u / MH2030A_UIP_TICK_MS)) != 0u) {
        printf("[MH2030A uIP] SysTick config failed\r\n");
    }
}

void mh2030a_uip_tick_isr(void)
{
    mh2030a_uip_update_time();
}

void mh2030a_uip_update_time(void)
{
    mh2030a_uip_elapsed_ms += MH2030A_UIP_TICK_MS;
}

uint32_t mh2030a_uip_millis(void)
{
    return mh2030a_uip_elapsed_ms;
}

clock_time_t clock_time(void)
{
    return (clock_time_t)mh2030a_uip_elapsed_ms;
}
