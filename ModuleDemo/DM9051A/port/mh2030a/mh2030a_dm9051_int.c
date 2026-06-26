#include "hal_mh2030a.h"

#if defined(DMPLUG_INT)
#include "core/dm9051.h"
#include "mh20xx_exti.h"
#include "mh20xx_gpio.h"
#include "mh20xx_misc.h"
#include "mh20xx_rcc.h"
#include "mh20xx_syscfg.h"

#define DM9051_INT_PORT          GPIOF
#define DM9051_INT_PIN           GPIO_Pin_6
#define DM9051_INT_PORT_CLK      RCC_AHBPeriph_GPIOF
#define DM9051_INT_SYSCFG_CLK    RCC_APB2Periph_SYSCFG
#define DM9051_INT_PORT_SOURCE   EXTI_PortSourceGPIOF
#define DM9051_INT_PIN_SOURCE    EXTI_PinSource6
#define DM9051_INT_LINE          EXTI_Line6
#define DM9051_INT_IRQn          EXTI4_15_IRQn

static char *int_info[] = {
    "MH2030A DM9051 interrupt GPIO",
    "INT PF6, EXTI line 6, falling edge",
};

char *hal_int_info(int index)
{
    return int_info[index & 1];
}

uint32_t hal_int_initialize(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;

    RCC_AHBPeriphClockCmd(DM9051_INT_PORT_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(DM9051_INT_SYSCFG_CLK, ENABLE);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_INT_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_INT_PORT, &gpio);

    SYSCFG_EXTILineConfig(DM9051_INT_PORT_SOURCE, DM9051_INT_PIN_SOURCE);

    EXTI_StructInit(&exti);
    exti.EXTI_Line = DM9051_INT_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);
    EXTI_ClearITPendingBit(DM9051_INT_LINE);

    return DM9051_INT_LINE;
}

void hal_enable_mcu_irq(void)
{
    NVIC_InitTypeDef nvic;

    nvic.NVIC_IRQChannel = DM9051_INT_IRQn;
    nvic.NVIC_IRQChannelPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void hal_disable_mcu_irq(void)
{
    NVIC_InitTypeDef nvic;

    nvic.NVIC_IRQChannel = DM9051_INT_IRQn;
    nvic.NVIC_IRQChannelPriority = 1;
    nvic.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvic);
}

uint32_t hal_irqline(void)
{
    return DM9051_INT_LINE;
}

void EXTI4_15_IRQHandler(void)
{
    if (EXTI_GetITStatus(DM9051_INT_LINE) != RESET) {
        dm9051_interrupt_set(DM9051_INT_LINE);
        EXTI_ClearITPendingBit(DM9051_INT_LINE);
    }
}
#else
static char *int_info[] = {
    "MH2030A DM9051 interrupt disabled",
    "polling mode bring-up",
};

char *hal_int_info(int index)
{
    return int_info[index & 1];
}

uint32_t hal_int_initialize(void)
{
    return 0u;
}

void hal_enable_mcu_irq(void)
{
}

void hal_disable_mcu_irq(void)
{
}

uint32_t hal_irqline(void)
{
    return 0u;
}
#endif
