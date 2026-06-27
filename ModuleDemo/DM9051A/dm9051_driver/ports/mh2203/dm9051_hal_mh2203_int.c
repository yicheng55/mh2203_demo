#include "dm9051_hal_mh2203_int.h"
#include "dm9051_hal_mh2203_spi1_priv.h"

#include "../../core/inc/dm9051_core.h"

#if DM9051_MH2203_ENABLE_IRQ
#pragma message("[DM9051 MH2203 build] int: DM9051_MH2203_ENABLE_IRQ=1")
#else
#pragma message("[DM9051 MH2203 build] int: DM9051_MH2203_ENABLE_IRQ=0")
#endif

#if DM9051_MH2203_ENABLE_IRQ

static volatile dm9051_device_t *dm9051_mh2203_irq_device;
static volatile uint32_t dm9051_mh2203_irq_event_count;

void dm9051_mh2203_irq_init_if_enabled(const dm9051_mh2203_config_t *config)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;

    if ((config == 0) || (config->irq_mode != DM9051_MH2203_IRQ_EXTI)) {
        return;
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    gpio.GPIO_Pin = DM9051_MH2203_INT_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DM9051_MH2203_INT_PORT, &gpio);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource6);

    exti.EXTI_Line = DM9051_MH2203_INT_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);
    EXTI_ClearITPendingBit(DM9051_MH2203_INT_LINE);

    printf("[MH2203 uIP] DM9051 interrupt initialized (INT PF6, EXTI line 6, falling edge)\r\n");
}

void dm9051_mh2203_irq_enable_if_enabled(void *ctx)
{
    const dm9051_mh2203_config_t *config = (const dm9051_mh2203_config_t *)ctx;
    NVIC_InitTypeDef nvic;

    if ((config == 0) || (config->irq_mode != DM9051_MH2203_IRQ_EXTI)) {
        return;
    }

    EXTI_ClearITPendingBit(DM9051_MH2203_INT_LINE);
    nvic.NVIC_IRQChannel = DM9051_MH2203_INT_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void dm9051_mh2203_irq_disable_if_enabled(void *ctx)
{
    const dm9051_mh2203_config_t *config = (const dm9051_mh2203_config_t *)ctx;
    NVIC_InitTypeDef nvic;

    if ((config == 0) || (config->irq_mode != DM9051_MH2203_IRQ_EXTI)) {
        return;
    }

    nvic.NVIC_IRQChannel = DM9051_MH2203_INT_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvic);
    EXTI_ClearITPendingBit(DM9051_MH2203_INT_LINE);
}

void dm9051_mh2203_irq_attach_device(dm9051_device_t *dev)
{
    dm9051_mh2203_irq_device = dev;
}

void dm9051_mh2203_irq_detach_device(void)
{
    dm9051_mh2203_irq_device = 0;
}

uint32_t dm9051_mh2203_irq_line(void)
{
    return DM9051_MH2203_INT_LINE;
}

uint32_t dm9051_mh2203_irq_count(void)
{
    return dm9051_mh2203_irq_event_count;
}

void dm9051_mh2203_irq_handler(void)
{
    dm9051_device_t *dev;

    if (EXTI_GetITStatus(DM9051_MH2203_INT_LINE) == RESET) {
        return;
    }

    ++dm9051_mh2203_irq_event_count;

    dev = (dm9051_device_t *)dm9051_mh2203_irq_device;
    if (dev != 0) {
        dm9051_core_interrupt_set(dev, DM9051_MH2203_INT_LINE);
    }

    EXTI_ClearITPendingBit(DM9051_MH2203_INT_LINE);
}

#if DM9051_MH2203_OWN_EXTI9_5_HANDLER
void EXTI9_5_IRQHandler(void)
{
    dm9051_mh2203_irq_handler();
}
#endif

#else /* !DM9051_MH2203_ENABLE_IRQ — provide no-op stubs so the linker is satisfied */

void dm9051_mh2203_irq_init_if_enabled(const dm9051_mh2203_config_t *config)
{
    (void)config;
}

void dm9051_mh2203_irq_enable_if_enabled(void *ctx)
{
    (void)ctx;
}

void dm9051_mh2203_irq_disable_if_enabled(void *ctx)
{
    (void)ctx;
}

void dm9051_mh2203_irq_attach_device(dm9051_device_t *dev)
{
    (void)dev;
}

void dm9051_mh2203_irq_detach_device(void)
{
}

uint32_t dm9051_mh2203_irq_line(void)
{
    return 0;
}

uint32_t dm9051_mh2203_irq_count(void)
{
    return 0;
}

void dm9051_mh2203_irq_handler(void)
{
}

#endif /* DM9051_MH2203_ENABLE_IRQ */
