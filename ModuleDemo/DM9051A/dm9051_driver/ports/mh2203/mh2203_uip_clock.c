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

/* SysTick 中斷服務常式 — 驅動 uIP 的軟體計時基準 (clock_time)。
 * 本專案 uIP target 未納入獨立的 mh20xx_it.c，故 SysTick_Handler 由 port 的
 * 時脈模組提供，覆寫 startup 的弱符號。DMA/IRQ 等其它中斷保留未來實作。 */
void SysTick_Handler(void)
{
    mh2203_uip_tick_isr();
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
