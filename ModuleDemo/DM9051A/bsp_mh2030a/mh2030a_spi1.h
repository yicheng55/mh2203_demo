#ifndef __MH2030A_SPI1_H
#define __MH2030A_SPI1_H

#include <stdint.h>
#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif
#include "mh20xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM9051A_OP_REG_R          0x00u
#define DM9051A_OP_REG_W          0x80u

#define DM9051A_NCR               0x00u
#define DM9051A_VIDL              0x28u
#define DM9051A_VIDH              0x29u
#define DM9051A_PIDL              0x2Au
#define DM9051A_PIDH              0x2Bu
#define DM9051A_CHIPR             0x2Cu
#define DM9051A_PAR               0x10u
#define DM9051A_MRCMD             0x72u
#define DM9051A_MWCMD             0x78u

#ifndef DM9051A_SPI_DEBUG
#define DM9051A_SPI_DEBUG         1
#endif

void MH2030A_SPI1_Init(void);
uint8_t MH2030A_SPI1_Transfer(uint8_t tx);

void DM9051A_CS_Low(void);
void DM9051A_CS_High(void);
void DM9051A_HardwareReset(void);
void DM9051A_DebugDump(const char *tag);

uint8_t DM9051A_ReadReg(uint8_t reg);
void DM9051A_WriteReg(uint8_t reg, uint8_t val);
void DM9051A_ReadRegBuf(uint8_t reg, uint8_t *buf, uint16_t len);
void DM9051A_WriteRegBuf(uint8_t reg, const uint8_t *buf, uint16_t len);
void DM9051A_ReadMem(uint8_t *buf, uint16_t len);
void DM9051A_WriteMem(const uint8_t *buf, uint16_t len);
uint16_t DM9051A_ReadVID(void);
uint16_t DM9051A_ReadPID(void);
void DM9051A_ReadMac(uint8_t mac[6]);
void DM9051A_WriteMac(const uint8_t mac[6]);

#ifdef __cplusplus
}
#endif

#endif
