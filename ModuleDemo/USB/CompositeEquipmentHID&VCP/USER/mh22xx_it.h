/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MH2203_IT_H
#define __MH2203_IT_H

/* Includes ------------------------------------------------------------------*/
#include "mh22xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void USBWakeUp_IRQHandler(void);
void USB_FS_WKUP_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
#endif /* __mh22xx_IT_H */

/******************* (C) COPYRIGHT  MHSEMICONDUCTOR *****END OF FILE****/
