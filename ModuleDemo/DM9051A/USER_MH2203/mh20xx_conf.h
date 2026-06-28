#ifndef __MH20XX_CONF_H
#define __MH20XX_CONF_H

#include "mh20xx_adc.h"
#include "mh20xx_crc.h"
#include "mh20xx_comp.h"
#include "mh20xx_dbgmcu.h"
#include "mh20xx_dma.h"
#include "mh20xx_exti.h"
#include "mh20xx_flash.h"
#include "mh20xx_gpio.h"
#include "mh20xx_syscfg.h"
#include "mh20xx_i2c.h"
#include "mh20xx_iwdg.h"
#include "mh20xx_pwr.h"
#include "mh20xx_rcc.h"
#include "mh20xx_rtc.h"
#include "mh20xx_spi.h"
#include "mh20xx_tim.h"
#include "mh20xx_usart.h"
#include "mh20xx_wwdg.h"
#include "mh20xx_misc.h"

#ifdef USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0)
#endif

#endif
