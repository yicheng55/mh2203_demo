#include "delay.h"

static uint8_t us_number = 0;
static uint16_t ms_number = 0;

void Delay_Init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    us_number = SystemCoreClock / 8000000;
    ms_number = (uint16_t)us_number * 1000;
}

void Delay_Us(uint32_t nus)
{
    uint32_t temp;

    SysTick->LOAD = nus * us_number;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1u << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}

void Delay_Ms(uint16_t nms)
{
    uint32_t temp;

    SysTick->LOAD = (uint32_t)nms * ms_number;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1u << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}
