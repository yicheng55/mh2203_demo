#include "clock-arch.h"
#include "mh2203_platform.h"

static volatile uint32_t atcmd_web_elapsed_ms;
__IO int32_t g_RunTime;

void atcmd_web_tick_init(void)
{
    (void)SysTick_Config(SystemCoreClock / 1000u);
}

void SysTick_Handler(void)
{
    atcmd_web_elapsed_ms++;
    g_RunTime++;
}

uint32_t atcmd_web_clock_millis(void)
{
    return atcmd_web_elapsed_ms;
}

clock_time_t clock_time(void)
{
    return (clock_time_t)atcmd_web_elapsed_ms;
}
