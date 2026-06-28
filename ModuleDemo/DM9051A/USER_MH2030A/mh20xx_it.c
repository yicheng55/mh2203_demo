#include "mh20xx_it.h"

#if defined(MH2030A_LWIP_PORT)
#include "lwip/arch.h"
#include "arch/sys_arch.h"
#else
#include "hal_mh2030a.h"
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
#if defined(MH2030A_LWIP_PORT)
    ++lwip_sys_now;
#else
    mh2030a_uip_tick_isr();
#endif
}
