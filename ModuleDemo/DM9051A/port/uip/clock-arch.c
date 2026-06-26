#include "hal_mh2030a.h"
#include "clock-arch.h"
#include "developer_conf.h"
#include <stdio.h>

extern void mh2030a_uip_update_time(void);
extern volatile uint32_t uip_elapsed_ms;
extern uint32_t g_RunTime;

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

uint32_t mh2030a_uip_millis(void)
{
    return uip_elapsed_ms;
}

clock_time_t clock_time(void)
{
    return (clock_time_t)uip_elapsed_ms;
}


