#ifndef __MH2203_SPI1_H
#define __MH2203_SPI1_H

#include <stdint.h>
#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif
#include "mh22xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM9051A 操作 ---- */
#define DM9051A_OP_REG_R          0x00u
#define DM9051A_OP_REG_W          0x80u

/* ---- DM9051A 暫存器位址 ---- */
#define DM9051A_NCR               0x00u
#define DM9051A_VIDL              0x28u
#define DM9051A_VIDH              0x29u
#define DM9051A_PIDL              0x2Au
#define DM9051A_PIDH              0x2Bu
#define DM9051A_CHIPR             0x2Cu
#define DM9051A_PAR               0x10u
#define DM9051A_MRCMD             0x72u
#define DM9051A_MWCMD             0x78u

/* ---- MH2203 針腳 (與 MH2030A 相同) ---- */
#define DM9051A_CS_PORT           GPIOA
#define DM9051A_CS_PIN            GPIO_Pin_15
#define DM9051A_SCK_PORT          GPIOB
#define DM9051A_SCK_PIN           GPIO_Pin_3
#define DM9051A_MOSI_PORT         GPIOB
#define DM9051A_MOSI_PIN          GPIO_Pin_5
#define DM9051A_MISO_PORT         GPIOB
#define DM9051A_MISO_PIN          GPIO_Pin_4
#define DM9051A_RST_PORT          GPIOF
#define DM9051A_RST_PIN           GPIO_Pin_7

/* ---- Debug 開關 ---- */
#ifndef DM9051A_SPI_DEBUG
#define DM9051A_SPI_DEBUG         1
#endif

/* SPI1 函式 */
void MH2203_SPI1_Init(void);
uint8_t MH2203_SPI1_Transfer(uint8_t tx);

/* 高階 DM9051A 函式 */
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
