#include "mh22xx_it.h"

#if defined(MH2203_LWIP_PORT)
#include "lwip/arch.h"
#include "arch/sys_arch.h"
#else
/* uIP 心跳:僅需 tick ISR 的宣告,不引入整個平台 HAL header
 * （hal_mh2030a.h 會連帶拉進 mh20xx.h 與舊版 beta driver,
 *   兩者在 MH2203 樹中皆不存在）。實作於 port/uip/clock-arch.c。 */
void mh2030a_uip_tick_isr(void);
#endif

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
#if defined(MH2203_LWIP_PORT)
    ++lwip_sys_now;
#else
    mh2030a_uip_tick_isr();
#endif
}
