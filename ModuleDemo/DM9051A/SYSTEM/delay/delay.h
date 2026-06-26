#ifndef __DELAY_H
#define __DELAY_H

#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif
#include "mh20xx.h"

void Delay_Init(void);
void Delay_Ms(uint16_t nms);
void Delay_Us(uint32_t nus);

#endif
