/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MH22xx_CONF_H
#define __MH22xx_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
#include "mh22xx_adc.h"
#include "mh22xx_bkp.h"
#include "mh22xx_can.h"
#include "mh22xx_crc.h"
#include "mh22xx_dac.h"
#include "mh22xx_dbgmcu.h"
#include "mh22xx_dma.h"
#include "mh22xx_exti.h"
#include "mh22xx_flash.h"
#include "mh22xx_fsmc.h"
#include "mh22xx_gpio.h"
#include "mh22xx_i2c.h"
#include "mh22xx_iwdg.h"
#include "mh22xx_pwr.h"
#include "mh22xx_rcc.h"
#include "mh22xx_rtc.h"
#include "mh22xx_sdio.h"
#include "mh22xx_spi.h"
#include "mh22xx_tim.h"
#include "mh22xx_usart.h"
#include "mh22xx_wwdg.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __MH22xx_CONF_H */

/******************* (C) COPYRIGHT  MHSEMICONDUCTOR *****END OF FILE****/
