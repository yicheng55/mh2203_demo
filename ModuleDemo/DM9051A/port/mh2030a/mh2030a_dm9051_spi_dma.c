#include "hal_mh2030a.h"
#include "core/dm9051.h"
#include <stdio.h>

#define DM9051_SPI               SPI1
#define DM9051_SPI_RX_DMA        DMA1_Channel2
#define DM9051_SPI_TX_DMA        DMA1_Channel3
#define DM9051_SPI_RX_DMA_FLAG   DMA1_FLAG_TC2
#define DM9051_SPI_TX_DMA_FLAG   DMA1_FLAG_TC3
#define DM9051_CS_PORT           GPIOA
#define DM9051_CS_PIN            GPIO_Pin_15
#define DM9051_SCK_PORT          GPIOB
#define DM9051_SCK_PIN           GPIO_Pin_3
#define DM9051_MOSI_PORT         GPIOB
#define DM9051_MOSI_PIN          GPIO_Pin_5
#define DM9051_MISO_PORT         GPIOB
#define DM9051_MISO_PIN          GPIO_Pin_4
#define DM9051_RST_PORT          GPIOF
#define DM9051_RST_PIN           GPIO_Pin_7
#define DM9051_SPI_TIMEOUT       1000000u

#ifndef OPC_REG_R
#define OPC_REG_R                0x00u
#endif
#ifndef OPC_REG_W
#define OPC_REG_W                0x80u
#endif

static uint8_t dm9051_dummy_tx_byte = 0x00u;
static uint8_t dm9051_dummy_rx_byte;

static char *spi_info[] = {
    "MH2030A DM9051 SPI1 DMA transfer",
    "CS PA15, SCK PB3, MOSI PB5, MISO PB4, RST PF7, DMA1 CH2/CH3",
};

char *hal_spi_info(int index)
{
    return spi_info[index & 1];
}

static void select_dm9051(void)
{
    GPIO_ResetBits(DM9051_CS_PORT, DM9051_CS_PIN);
}

static void deselect_dm9051(void)
{
    GPIO_SetBits(DM9051_CS_PORT, DM9051_CS_PIN);
}

static void reset_dm9051_hardware(void)
{
    GPIO_ResetBits(DM9051_RST_PORT, DM9051_RST_PIN);
    Delay_Ms(2);
    GPIO_SetBits(DM9051_RST_PORT, DM9051_RST_PIN);
    Delay_Ms(10);
}

static uint8_t wait_for_spi_idle(void)
{
    uint32_t timeout = DM9051_SPI_TIMEOUT;

    while (SPI_I2S_GetFlagStatus(DM9051_SPI, SPI_I2S_FLAG_BSY) == SET) {
        if (--timeout == 0u) {
            return 0u;
        }
    }

    return 1u;
}

static uint8_t transfer_spi_byte(uint8_t tx)
{
    uint32_t timeout;

    timeout = DM9051_SPI_TIMEOUT;
    while (SPI_I2S_GetFlagStatus(DM9051_SPI, SPI_I2S_FLAG_TXE) == RESET) {
        if (--timeout == 0u) {
            return 0xffu;
        }
    }
    SPI_SendData8(DM9051_SPI, tx);

    timeout = DM9051_SPI_TIMEOUT;
    while (SPI_I2S_GetFlagStatus(DM9051_SPI, SPI_I2S_FLAG_RXNE) == RESET) {
        if (--timeout == 0u) {
            return 0xffu;
        }
    }
    return SPI_ReceiveData8(DM9051_SPI);
}

static void finish_spi_transfer(void)
{
    (void)wait_for_spi_idle();
    while (SPI_I2S_GetFlagStatus(DM9051_SPI, SPI_I2S_FLAG_RXNE) == SET) {
        (void)SPI_ReceiveData8(DM9051_SPI);
    }
}

static void configure_spi_dma_transfer(uint8_t *rx, const uint8_t *tx, uint16_t len)
{
    DMA_InitTypeDef dma;

    DMA_Cmd(DM9051_SPI_RX_DMA, DISABLE);
    DMA_Cmd(DM9051_SPI_TX_DMA, DISABLE);
    DMA_ClearFlag(DM9051_SPI_RX_DMA_FLAG | DM9051_SPI_TX_DMA_FLAG);

    DMA_DeInit(DM9051_SPI_RX_DMA);
    DMA_StructInit(&dma);
    dma.DMA_PeripheralBaseAddr = (uint32_t)&DM9051_SPI->DR;
    dma.DMA_MemoryBaseAddr = (uint32_t)rx;
    dma.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize = len;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = (rx == &dm9051_dummy_rx_byte) ? DMA_MemoryInc_Disable : DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_VeryHigh;
    dma.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DM9051_SPI_RX_DMA, &dma);

    DMA_DeInit(DM9051_SPI_TX_DMA);
    DMA_StructInit(&dma);
    dma.DMA_PeripheralBaseAddr = (uint32_t)&DM9051_SPI->DR;
    dma.DMA_MemoryBaseAddr = (uint32_t)tx;
    dma.DMA_DIR = DMA_DIR_PeripheralDST;
    dma.DMA_BufferSize = len;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = (tx == &dm9051_dummy_tx_byte) ? DMA_MemoryInc_Disable : DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_VeryHigh;
    dma.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DM9051_SPI_TX_DMA, &dma);
}

static uint8_t transfer_spi_dma_buffer(uint8_t *rx, const uint8_t *tx, uint16_t len)
{
    uint32_t timeout;

    if (len == 0u) {
        return 1u;
    }

    if (!wait_for_spi_idle()) {
        return 0u;
    }

    configure_spi_dma_transfer(rx, tx, len);
    DMA_Cmd(DM9051_SPI_RX_DMA, ENABLE);
    DMA_Cmd(DM9051_SPI_TX_DMA, ENABLE);

    timeout = DM9051_SPI_TIMEOUT * (uint32_t)len;
    while (DMA_GetFlagStatus(DM9051_SPI_RX_DMA_FLAG) == RESET) {
        if (--timeout == 0u) {
            DMA_Cmd(DM9051_SPI_TX_DMA, DISABLE);
            DMA_Cmd(DM9051_SPI_RX_DMA, DISABLE);
            return 0u;
        }
    }

    timeout = DM9051_SPI_TIMEOUT * (uint32_t)len;
    while (DMA_GetFlagStatus(DM9051_SPI_TX_DMA_FLAG) == RESET) {
        if (--timeout == 0u) {
            DMA_Cmd(DM9051_SPI_TX_DMA, DISABLE);
            DMA_Cmd(DM9051_SPI_RX_DMA, DISABLE);
            return 0u;
        }
    }

    DMA_Cmd(DM9051_SPI_TX_DMA, DISABLE);
    DMA_Cmd(DM9051_SPI_RX_DMA, DISABLE);
    DMA_ClearFlag(DM9051_SPI_RX_DMA_FLAG | DM9051_SPI_TX_DMA_FLAG);

    return wait_for_spi_idle();
}

void hal_spi_initialize(void)
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
    gpio.GPIO_Pin = DM9051_CS_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_CS_PORT, &gpio);
    deselect_dm9051();

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_RST_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_RST_PORT, &gpio);
    reset_dm9051_hardware();

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_0);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_SCK_PIN | DM9051_MOSI_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_MISO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);

    SPI_I2S_DeInit(DM9051_SPI);
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
    SPI_Init(DM9051_SPI, &spi);
    SPI_RxFIFOThresholdConfig(DM9051_SPI, SPI_RxFIFOThreshold_QF);
    SPI_I2S_DMACmd(DM9051_SPI, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(DM9051_SPI, ENABLE);

    printf("[MH2030A uIP] DM9051 SPI bus initialized (DMA transfer)\r\n");
}

uint8_t hal_read_reg(uint8_t reg)
{
    uint8_t val;

    select_dm9051();
    (void)transfer_spi_byte((uint8_t)(reg | OPC_REG_R));
    val = transfer_spi_byte(0x00u);
    finish_spi_transfer();
    deselect_dm9051();
    return val;
}

void hal_write_reg(uint8_t reg, uint8_t val)
{
    select_dm9051();
    (void)transfer_spi_byte((uint8_t)(reg | OPC_REG_W));
    (void)transfer_spi_byte(val);
    finish_spi_transfer();
    deselect_dm9051();
}

void hal_read_mem(uint8_t *buf, uint16_t len)
{
    select_dm9051();
    (void)transfer_spi_byte((uint8_t)(DM9051_MRCMD | OPC_REG_R));
    (void)transfer_spi_dma_buffer(buf, &dm9051_dummy_tx_byte, len);
    finish_spi_transfer();
    deselect_dm9051();
}

void hal_write_mem(const uint8_t *buf, uint16_t len)
{
    select_dm9051();
    (void)transfer_spi_byte((uint8_t)(DM9051_MWCMD | OPC_REG_W));
    (void)transfer_spi_dma_buffer(&dm9051_dummy_rx_byte, buf, len);
    finish_spi_transfer();
    deselect_dm9051();
}
