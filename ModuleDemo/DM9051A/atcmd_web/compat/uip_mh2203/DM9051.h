#ifndef ATCMD_WEB_MH2203_DM9051_COMPAT_H
#define ATCMD_WEB_MH2203_DM9051_COMPAT_H

#include <stdint.h>

#include "../../../dm9051_driver/core/inc/dm9051_regs.h"

#ifndef emacETHADDR0
#define emacETHADDR0 0x00u
#define emacETHADDR1 0x60u
#define emacETHADDR2 0x6Eu
#define emacETHADDR3 0xB4u
#define emacETHADDR4 0x7Eu
#define emacETHADDR5 0x34u
#endif
#ifndef NSR_LINKST
#define NSR_LINKST 0x40u
#endif

extern uint8_t DM9051_RX_INT_TRIGGER;

uint8_t DM9051_Read_Reg(uint8_t reg);
void DM9051_Write_Reg(uint8_t reg, uint8_t value);
int32_t DM9051_Init(void);
uint32_t DM9051_TX(void);
uint16_t DM9051_RX(void);

#define DM9051_RX_Final 1u

#endif /* ATCMD_WEB_MH2203_DM9051_COMPAT_H */
