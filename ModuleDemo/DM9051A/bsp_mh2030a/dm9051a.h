#ifndef __DM9051A_H
#define __DM9051A_H

#include <stdint.h>
#include "mh2030a_spi1.h"

#ifdef __cplusplus
extern "C" {
#endif

void DM9051A_Init(void);
uint8_t DM9051A_ReadReg(uint8_t reg);
void DM9051A_WriteReg(uint8_t reg, uint8_t val);
uint16_t DM9051A_ReadVID(void);
uint16_t DM9051A_ReadPID(void);

#ifdef __cplusplus
}
#endif

#endif
