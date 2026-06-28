#ifndef __MH22XX_CONF_H
#define __MH22XX_CONF_H

#include "mh22xx_adc.h"
#include "mh22xx_crc.h"
#include "mh22xx_dbgmcu.h"
#include "mh22xx_dma.h"
#include "mh22xx_exti.h"
#include "mh22xx_flash.h"
#include "mh22xx_gpio.h"
#include "mh22xx_i2c.h"
#include "mh22xx_iwdg.h"
#include "mh22xx_pwr.h"
#include "mh22xx_rcc.h"
#include "mh22xx_rtc.h"
#include "mh22xx_spi.h"
#include "mh22xx_tim.h"
#include "mh22xx_usart.h"
#include "mh22xx_wwdg.h"
#include "misc.h"

#ifdef USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0)
#endif

#endif
