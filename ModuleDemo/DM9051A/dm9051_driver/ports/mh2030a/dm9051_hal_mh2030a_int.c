#include "dm9051_hal_mh2030a_int.h"
#include "dm9051_hal_mh2030a_spi1_priv.h"

#include "../../core/inc/dm9051_core.h"

#if DM9051_MH2030A_ENABLE_IRQ

static volatile dm9051_device_t *dm9051_mh2030a_irq_device;
static volatile uint32_t dm9051_mh2030a_irq_event_count;

void dm9051_mh2030a_irq_init_if_enabled(const dm9051_mh2030a_config_t *config)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;

    if ((config == 0) || (config->irq_mode != DM9051_MH2030A_IRQ_EXTI)) {
        return;
    }

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_MH2030A_INT_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_MH2030A_INT_PORT, &gpio);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource6);

    EXTI_StructInit(&exti);
    exti.EXTI_Line = DM9051_MH2030A_INT_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);
    EXTI_ClearITPendingBit(DM9051_MH2030A_INT_LINE);

    printf("[MH2030A uIP] DM9051 interrupt initialized (INT PF6, EXTI line 6, falling edge)\r\n");
}

void dm9051_mh2030a_irq_enable_if_enabled(void *ctx)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;
    NVIC_InitTypeDef nvic;

    if ((config == 0) || (config->irq_mode != DM9051_MH2030A_IRQ_EXTI)) {
        return;
    }

    EXTI_ClearITPendingBit(DM9051_MH2030A_INT_LINE);
    nvic.NVIC_IRQChannel = DM9051_MH2030A_INT_IRQn;
    nvic.NVIC_IRQChannelPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void dm9051_mh2030a_irq_disable_if_enabled(void *ctx)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;
    NVIC_InitTypeDef nvic;

    if ((config == 0) || (config->irq_mode != DM9051_MH2030A_IRQ_EXTI)) {
        return;
    }

    nvic.NVIC_IRQChannel = DM9051_MH2030A_INT_IRQn;
    nvic.NVIC_IRQChannelPriority = 1;
    nvic.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvic);
    EXTI_ClearITPendingBit(DM9051_MH2030A_INT_LINE);
}

void dm9051_mh2030a_irq_attach_device(dm9051_device_t *dev)
{
    dm9051_mh2030a_irq_device = dev;
}

void dm9051_mh2030a_irq_detach_device(void)
{
    dm9051_mh2030a_irq_device = 0;
}

uint32_t dm9051_mh2030a_irq_line(void)
{
    return DM9051_MH2030A_INT_LINE;
}

uint32_t dm9051_mh2030a_irq_count(void)
{
    return dm9051_mh2030a_irq_event_count;
}

void dm9051_mh2030a_irq_handler(void)
{
    dm9051_device_t *dev;

    if (EXTI_GetITStatus(DM9051_MH2030A_INT_LINE) == RESET) {
        return;
    }

    ++dm9051_mh2030a_irq_event_count;

    dev = (dm9051_device_t *)dm9051_mh2030a_irq_device;
    if (dev != 0) {
        dm9051_core_interrupt_set(dev, DM9051_MH2030A_INT_LINE);
    }

    EXTI_ClearITPendingBit(DM9051_MH2030A_INT_LINE);
}

#if DM9051_MH2030A_OWN_EXTI4_15_HANDLER
void EXTI4_15_IRQHandler(void)
{
    dm9051_mh2030a_irq_handler();
}
#endif

#endif /* DM9051_MH2030A_ENABLE_IRQ */
