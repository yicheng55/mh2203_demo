#ifndef MH2203_UIP_PORT

#include "mh2203_spi1.h"
#include "delay.h"
#if DM9051A_SPI_DEBUG
#include <stdio.h>
#endif

#define DM9051A_SPI               SPI1
#define DM9051A_SPI_RX_DMA        DMA1_Channel2
#define DM9051A_SPI_TX_DMA        DMA1_Channel3
#define DM9051A_SPI_RX_DMA_FLAG   DMA1_FLAG_TC2
#define DM9051A_SPI_TX_DMA_FLAG   DMA1_FLAG_TC3
#define DM9051A_SPI_DMA_TIMEOUT   1000000u

/* Schematic:
 *   PA15 -> SPI_CS, driven as GPIO
 *   PB3  -> SPI1_SCK
 *   PB5  -> SPI1_MOSI
 *   PB4  -> SPI1_MISO
 */
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

static uint8_t DM9051A_DummyTx = 0x00u;
static uint8_t DM9051A_DummyRx;

#if DM9051A_SPI_DEBUG
#define DM9051A_DBG_PRINT         printf

static void DM9051A_DebugPrintPinState(void)
{
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] GPIOA MODER=0x%08lX IDR=0x%04X ODR=0x%04X AFRH=0x%08lX\r\n",
                     GPIOA->MODER, GPIOA->IDR, GPIOA->ODR, GPIOA->AFR[1]);
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] GPIOB MODER=0x%08lX IDR=0x%04X ODR=0x%04X AFRL=0x%08lX\r\n",
                     GPIOB->MODER, GPIOB->IDR, GPIOB->ODR, GPIOB->AFR[0]);
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] GPIOF MODER=0x%08lX IDR=0x%04X ODR=0x%04X\r\n",
                     GPIOF->MODER, GPIOF->IDR, GPIOF->ODR);
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] Pins CS(PA15)=%u SCK(PB3)=%u MOSI(PB5)=%u MISO(PB4)=%u RST(PF7)=%u\r\n",
                     (DM9051A_CS_PORT->IDR & DM9051A_CS_PIN)     ? 1u : 0u,
                     (DM9051A_SCK_PORT->IDR & DM9051A_SCK_PIN)   ? 1u : 0u,
                     (DM9051A_MOSI_PORT->ODR & DM9051A_MOSI_PIN) ? 1u : 0u,
                     (DM9051A_MISO_PORT->IDR & DM9051A_MISO_PIN) ? 1u : 0u,
                     (DM9051A_RST_PORT->IDR & DM9051A_RST_PIN)   ? 1u : 0u);
}

static void DM9051A_DebugPrintSpiState(void)
{
    uint32_t br;
    uint32_t pclk;
    uint32_t spi_clk;
    RCC_ClocksTypeDef clks;

    RCC_GetClocksFreq(&clks);
    pclk = clks.PCLK_Frequency;

    br = ((uint32_t)DM9051A_SPI->CR1 >> 3u) & 0x7u;
    spi_clk = pclk >> (br + 1u);

    DM9051A_DBG_PRINT("[MH2203 DMA DBG] SPI1 CR1=0x%04X CR2=0x%04X SR=0x%04X I2SCFGR=0x%04X\r\n",
                     DM9051A_SPI->CR1, DM9051A_SPI->CR2, DM9051A_SPI->SR, DM9051A_SPI->I2SCFGR);
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] SPI1 clock: PCLK=%lu Hz, BR=%lu (div=%lu), SPI_CLK=%lu Hz (%lu kHz)\r\n",
                     pclk, br, 1ul << (br + 1u), spi_clk, spi_clk / 1000ul);
}
#endif

static uint8_t DM9051A_WaitSpiIdle(void)
{
    uint32_t timeout = DM9051A_SPI_DMA_TIMEOUT;

    while (SPI_I2S_GetFlagStatus(DM9051A_SPI, SPI_I2S_FLAG_BSY) == SET) {
        if (--timeout == 0u) {
#if DM9051A_SPI_DEBUG
            DM9051A_DBG_PRINT("[MH2203 DMA DBG] SPI busy timeout, SR=0x%04X\r\n", DM9051A_SPI->SR);
#endif
            return 0u;
        }
    }

    return 1u;
}

static void DM9051A_DmaConfig(uint8_t *rx, const uint8_t *tx, uint16_t len)
{
    DMA_InitTypeDef dma;

    DMA_Cmd(DM9051A_SPI_RX_DMA, DISABLE);
    DMA_Cmd(DM9051A_SPI_TX_DMA, DISABLE);
    DMA_ClearFlag(DM9051A_SPI_RX_DMA_FLAG | DM9051A_SPI_TX_DMA_FLAG);

    DMA_DeInit(DM9051A_SPI_RX_DMA);
    DMA_StructInit(&dma);
    dma.DMA_PeripheralBaseAddr = (uint32_t)&DM9051A_SPI->DR;
    dma.DMA_MemoryBaseAddr = (uint32_t)rx;
    dma.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize = len;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = (rx == &DM9051A_DummyRx) ? DMA_MemoryInc_Disable : DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_VeryHigh;
    dma.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DM9051A_SPI_RX_DMA, &dma);

    DMA_DeInit(DM9051A_SPI_TX_DMA);
    DMA_StructInit(&dma);
    dma.DMA_PeripheralBaseAddr = (uint32_t)&DM9051A_SPI->DR;
    dma.DMA_MemoryBaseAddr = (uint32_t)tx;
    dma.DMA_DIR = DMA_DIR_PeripheralDST;
    dma.DMA_BufferSize = len;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = (tx == &DM9051A_DummyTx) ? DMA_MemoryInc_Disable : DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_VeryHigh;
    dma.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DM9051A_SPI_TX_DMA, &dma);
}

static uint8_t DM9051A_DmaTransfer(uint8_t *rx, const uint8_t *tx, uint16_t len)
{
    uint32_t timeout;

    if (len == 0u) {
        return 1u;
    }

    if (!DM9051A_WaitSpiIdle()) {
        return 0u;
    }

    DM9051A_DmaConfig(rx, tx, len);
    DMA_Cmd(DM9051A_SPI_RX_DMA, ENABLE);
    DMA_Cmd(DM9051A_SPI_TX_DMA, ENABLE);

    timeout = DM9051A_SPI_DMA_TIMEOUT * (uint32_t)len;
    while (DMA_GetFlagStatus(DM9051A_SPI_RX_DMA_FLAG) == RESET) {
        if (--timeout == 0u) {
            DMA_Cmd(DM9051A_SPI_TX_DMA, DISABLE);
            DMA_Cmd(DM9051A_SPI_RX_DMA, DISABLE);
#if DM9051A_SPI_DEBUG
            DM9051A_DBG_PRINT("[MH2203 DMA DBG] DMA RX timeout, len=%u SR=0x%04X\r\n", len, DM9051A_SPI->SR);
#endif
            return 0u;
        }
    }

    timeout = DM9051A_SPI_DMA_TIMEOUT * (uint32_t)len;
    while (DMA_GetFlagStatus(DM9051A_SPI_TX_DMA_FLAG) == RESET) {
        if (--timeout == 0u) {
            DMA_Cmd(DM9051A_SPI_TX_DMA, DISABLE);
            DMA_Cmd(DM9051A_SPI_RX_DMA, DISABLE);
#if DM9051A_SPI_DEBUG
            DM9051A_DBG_PRINT("[MH2203 DMA DBG] DMA TX timeout, len=%u SR=0x%04X\r\n", len, DM9051A_SPI->SR);
#endif
            return 0u;
        }
    }

    DMA_Cmd(DM9051A_SPI_TX_DMA, DISABLE);
    DMA_Cmd(DM9051A_SPI_RX_DMA, DISABLE);
    DMA_ClearFlag(DM9051A_SPI_RX_DMA_FLAG | DM9051A_SPI_TX_DMA_FLAG);

    return DM9051A_WaitSpiIdle();
}

void MH2203_SPI1_Init(void)
{
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    DMA_RemapConfig(DMA1, DMA1_CH2_SPI1_RX);
    DMA_RemapConfig(DMA1, DMA1_CH3_SPI1_TX);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051A_CS_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051A_CS_PORT, &gpio);
    DM9051A_CS_High();

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051A_RST_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051A_RST_PORT, &gpio);
    DM9051A_HardwareReset();

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_0);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051A_SCK_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(DM9051A_SCK_PORT, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051A_MOSI_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(DM9051A_MOSI_PORT, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051A_MISO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051A_MISO_PORT, &gpio);

    SPI_I2S_DeInit(DM9051A_SPI);
    SPI_StructInit(&spi);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7;
    SPI_Init(DM9051A_SPI, &spi);
    SPI_RxFIFOThresholdConfig(DM9051A_SPI, SPI_RxFIFOThreshold_QF);
    SPI_I2S_DMACmd(DM9051A_SPI, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(DM9051A_SPI, ENABLE);

#if DM9051A_SPI_DEBUG
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] SPI1 DMA init done\r\n");
    DM9051A_DebugPrintPinState();
    DM9051A_DebugPrintSpiState();
#endif
}

uint8_t MH2203_SPI1_Transfer(uint8_t tx)
{
    uint32_t timeout;

    timeout = DM9051A_SPI_DMA_TIMEOUT;
    while (SPI_I2S_GetFlagStatus(DM9051A_SPI, SPI_I2S_FLAG_TXE) == RESET) {
        if (--timeout == 0u) {
#if DM9051A_SPI_DEBUG
            DM9051A_DBG_PRINT("[MH2203 DMA DBG] TXE timeout, SR=0x%04X\r\n", DM9051A_SPI->SR);
#endif
            return 0x00u;
        }
    }
    SPI_SendData8(DM9051A_SPI, tx);

    timeout = DM9051A_SPI_DMA_TIMEOUT;
    while (SPI_I2S_GetFlagStatus(DM9051A_SPI, SPI_I2S_FLAG_RXNE) == RESET) {
        if (--timeout == 0u) {
#if DM9051A_SPI_DEBUG
            DM9051A_DBG_PRINT("[MH2203 DMA DBG] RXNE timeout, tx=0x%02X SR=0x%04X CR1=0x%04X CR2=0x%04X\r\n",
                             tx, DM9051A_SPI->SR, DM9051A_SPI->CR1, DM9051A_SPI->CR2);
#endif
            return 0x00u;
        }
    }
    return SPI_ReceiveData8(DM9051A_SPI);
}

void DM9051A_CS_Low(void)
{
    GPIO_ResetBits(DM9051A_CS_PORT, DM9051A_CS_PIN);
}

void DM9051A_CS_High(void)
{
    GPIO_SetBits(DM9051A_CS_PORT, DM9051A_CS_PIN);
}

void DM9051A_HardwareReset(void)
{
#if DM9051A_SPI_DEBUG
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] Hardware reset start\r\n");
#endif
    GPIO_ResetBits(DM9051A_RST_PORT, DM9051A_RST_PIN);
    Delay_Ms(2);
    GPIO_SetBits(DM9051A_RST_PORT, DM9051A_RST_PIN);
    Delay_Ms(10);
#if DM9051A_SPI_DEBUG
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] Hardware reset done, RST(PF7)=%u\r\n",
                     (GPIOF->IDR & DM9051A_RST_PIN) ? 1u : 0u);
#endif
}

void DM9051A_DebugDump(const char *tag)
{
#if DM9051A_SPI_DEBUG
    uint8_t raw1;
    uint8_t raw2;
    uint8_t ncr;
    uint8_t nsr;
    uint8_t vidl;
    uint8_t vidh;
    uint8_t pidl;
    uint8_t pidh;
    uint8_t chipr;
    uint8_t isr;
    uint8_t imr;

    DM9051A_DBG_PRINT("\r\n[MH2203 DMA DBG] dump: %s\r\n", tag ? tag : "");
    DM9051A_DebugPrintPinState();
    DM9051A_DebugPrintSpiState();

    DM9051A_CS_Low();
    raw1 = MH2203_SPI1_Transfer(DM9051A_CHIPR | DM9051A_OP_REG_R);
    raw2 = MH2203_SPI1_Transfer(0x00u);
    DM9051A_CS_High();
    DM9051A_DBG_PRINT("[MH2203 DMA DBG] raw CHIPR read: cmd=0x%02X data=0x%02X\r\n", raw1, raw2);

    ncr = DM9051A_ReadReg(DM9051A_NCR);
    nsr = DM9051A_ReadReg(0x01u);
    vidl = DM9051A_ReadReg(DM9051A_VIDL);
    vidh = DM9051A_ReadReg(DM9051A_VIDH);
    pidl = DM9051A_ReadReg(DM9051A_PIDL);
    pidh = DM9051A_ReadReg(DM9051A_PIDH);
    chipr = DM9051A_ReadReg(DM9051A_CHIPR);
    isr = DM9051A_ReadReg(0x7Eu);
    imr = DM9051A_ReadReg(0x7Fu);

    DM9051A_DBG_PRINT("[MH2203 DMA DBG] regs NCR=0x%02X NSR=0x%02X VIDL=0x%02X VIDH=0x%02X PIDL=0x%02X PIDH=0x%02X CHIPR=0x%02X ISR=0x%02X IMR=0x%02X\r\n",
                     ncr, nsr, vidl, vidh, pidl, pidh, chipr, isr, imr);
    DM9051A_DebugPrintSpiState();
#else
    (void)tag;
#endif
}

uint8_t DM9051A_ReadReg(uint8_t reg)
{
    uint8_t val;

    DM9051A_CS_Low();
    MH2203_SPI1_Transfer((uint8_t)(reg | DM9051A_OP_REG_R));
    val = MH2203_SPI1_Transfer(0x00u);
    DM9051A_CS_High();

    return val;
}

void DM9051A_WriteReg(uint8_t reg, uint8_t val)
{
    DM9051A_CS_Low();
    MH2203_SPI1_Transfer((uint8_t)(reg | DM9051A_OP_REG_W));
    MH2203_SPI1_Transfer(val);
    DM9051A_CS_High();
}

void DM9051A_ReadRegBuf(uint8_t reg, uint8_t *buf, uint16_t len)
{
    DM9051A_CS_Low();
    MH2203_SPI1_Transfer((uint8_t)(reg | DM9051A_OP_REG_R));
    (void)DM9051A_DmaTransfer(buf, &DM9051A_DummyTx, len);
    DM9051A_CS_High();
}

void DM9051A_WriteRegBuf(uint8_t reg, const uint8_t *buf, uint16_t len)
{
    DM9051A_CS_Low();
    MH2203_SPI1_Transfer((uint8_t)(reg | DM9051A_OP_REG_W));
    (void)DM9051A_DmaTransfer(&DM9051A_DummyRx, buf, len);
    DM9051A_CS_High();
}

void DM9051A_ReadMem(uint8_t *buf, uint16_t len)
{
    DM9051A_CS_Low();
    MH2203_SPI1_Transfer((uint8_t)(DM9051A_MRCMD | DM9051A_OP_REG_R));
    (void)DM9051A_DmaTransfer(buf, &DM9051A_DummyTx, len);
    DM9051A_CS_High();
}

void DM9051A_WriteMem(const uint8_t *buf, uint16_t len)
{
    DM9051A_CS_Low();
    MH2203_SPI1_Transfer((uint8_t)(DM9051A_MWCMD | DM9051A_OP_REG_W));
    (void)DM9051A_DmaTransfer(&DM9051A_DummyRx, buf, len);
    DM9051A_CS_High();
}

uint16_t DM9051A_ReadVID(void)
{
    return (uint16_t)DM9051A_ReadReg(DM9051A_VIDL) |
           ((uint16_t)DM9051A_ReadReg(DM9051A_VIDH) << 8);
}

uint16_t DM9051A_ReadPID(void)
{
    return (uint16_t)DM9051A_ReadReg(DM9051A_PIDL) |
           ((uint16_t)DM9051A_ReadReg(DM9051A_PIDH) << 8);
}

void DM9051A_ReadMac(uint8_t mac[6])
{
    DM9051A_ReadRegBuf(DM9051A_PAR, mac, 6);
}

void DM9051A_WriteMac(const uint8_t mac[6])
{
    DM9051A_WriteRegBuf(DM9051A_PAR, mac, 6);
}

#endif /* MH2203_UIP_PORT */
