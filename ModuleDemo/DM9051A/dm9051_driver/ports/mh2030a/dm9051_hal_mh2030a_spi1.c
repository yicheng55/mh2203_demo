/*
 * MH2030A SPI1 polling/GPIO/delay binding for DM9051.
 *
 * Optional transports are split out by feature:
 *   dm9051_hal_mh2030a_spi1_dma.c  - SPI1 DMA FIFO transfer path
 *   dm9051_hal_mh2030a_int.c       - DM9051 INT/EXTI helpers
 */

#include "dm9051_hal_mh2030a_spi1_priv.h"

const dm9051_hal_ops_t dm9051_mh2030a_polling_ops = {
    dm9051_mh2030a_polling_read_reg,
    dm9051_mh2030a_polling_write_reg,
    dm9051_mh2030a_polling_read_mem,
    dm9051_mh2030a_polling_write_mem,
    dm9051_mh2030a_polling_reset,
    dm9051_mh2030a_delay_ms,
    dm9051_mh2030a_delay_us,
    dm9051_mh2030a_irq_enable_if_enabled,
    dm9051_mh2030a_irq_disable_if_enabled,
    dm9051_mh2030a_enter_critical,
    dm9051_mh2030a_exit_critical
};

static dm9051_mh2030a_config_t dm9051_mh2030a_bound_config;

void dm9051_mh2030a_select(void)
{
    GPIO_ResetBits(DM9051_MH2030A_CS_PORT, DM9051_MH2030A_CS_PIN);
}

void dm9051_mh2030a_deselect(void)
{
    GPIO_SetBits(DM9051_MH2030A_CS_PORT, DM9051_MH2030A_CS_PIN);
}

int dm9051_mh2030a_wait_spi_idle(const dm9051_mh2030a_config_t *config)
{
    uint32_t timeout;

    if (config == 0) {
        return DM9051_HAL_ERR_PARAM;
    }

    timeout = config->spi_timeout;
    while (SPI_I2S_GetFlagStatus(DM9051_MH2030A_SPI, SPI_I2S_FLAG_BSY) == SET) {
        if (timeout == 0u) {
            DM9051_MH2030A_DIAG_PRINTF("[DM9051 HAL] SPI timeout: BSY\r\n");
            return DM9051_HAL_ERR_TIMEOUT;
        }
        --timeout;
    }

    return DM9051_HAL_OK;
}

int dm9051_mh2030a_transfer_byte(const dm9051_mh2030a_config_t *config,
                                 uint8_t tx,
                                 uint8_t *rx)
{
    uint32_t timeout;

    if ((config == 0) || (rx == 0)) {
        return DM9051_HAL_ERR_PARAM;
    }

    timeout = config->spi_timeout;
    while (SPI_I2S_GetFlagStatus(DM9051_MH2030A_SPI, SPI_I2S_FLAG_TXE) == RESET) {
        if (timeout == 0u) {
            DM9051_MH2030A_DIAG_PRINTF("[DM9051 HAL] SPI timeout: TXE tx=0x%02X\r\n", tx);
            return DM9051_HAL_ERR_TIMEOUT;
        }
        --timeout;
    }
    SPI_SendData8(DM9051_MH2030A_SPI, tx);

    timeout = config->spi_timeout;
    while (SPI_I2S_GetFlagStatus(DM9051_MH2030A_SPI, SPI_I2S_FLAG_RXNE) == RESET) {
        if (timeout == 0u) {
            DM9051_MH2030A_DIAG_PRINTF("[DM9051 HAL] SPI timeout: RXNE tx=0x%02X\r\n", tx);
            return DM9051_HAL_ERR_TIMEOUT;
        }
        --timeout;
    }

    *rx = SPI_ReceiveData8(DM9051_MH2030A_SPI);
    return DM9051_HAL_OK;
}

int dm9051_mh2030a_finish_transfer(const dm9051_mh2030a_config_t *config)
{
    uint32_t timeout;
    int status;

    status = dm9051_mh2030a_wait_spi_idle(config);
    if (config == 0) {
        return status;
    }

    timeout = config->spi_timeout;
    while (SPI_I2S_GetFlagStatus(DM9051_MH2030A_SPI, SPI_I2S_FLAG_RXNE) == SET) {
        if (timeout == 0u) {
            DM9051_MH2030A_DIAG_PRINTF("[DM9051 HAL] SPI timeout: RXNE drain\r\n");
            return DM9051_HAL_ERR_TIMEOUT;
        }
        (void)SPI_ReceiveData8(DM9051_MH2030A_SPI);
        --timeout;
    }

    return status;
}

void dm9051_mh2030a_spi1_bus_init_common(void)
{
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_MH2030A_CS_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_MH2030A_CS_PORT, &gpio);
    dm9051_mh2030a_deselect();

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_MH2030A_RST_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_MH2030A_RST_PORT, &gpio);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_0);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_MH2030A_SCK_PIN | DM9051_MH2030A_MOSI_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(DM9051_MH2030A_SCK_PORT, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DM9051_MH2030A_MISO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DM9051_MH2030A_MISO_PORT, &gpio);

    SPI_I2S_DeInit(DM9051_MH2030A_SPI);
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
    SPI_Init(DM9051_MH2030A_SPI, &spi);
    SPI_RxFIFOThresholdConfig(DM9051_MH2030A_SPI, SPI_RxFIFOThreshold_QF);
    SPI_Cmd(DM9051_MH2030A_SPI, ENABLE);
}

void dm9051_mh2030a_spi1_polling_bus_init(void)
{
    dm9051_mh2030a_spi1_bus_init_common();
    printf("[MH2030A uIP] DM9051 SPI bus initialized (polling transfer)\r\n");
}

void dm9051_mh2030a_reset_gpio_sequence(void)
{
    GPIO_ResetBits(DM9051_MH2030A_RST_PORT, DM9051_MH2030A_RST_PIN);
    Delay_Ms(2u);
    GPIO_SetBits(DM9051_MH2030A_RST_PORT, DM9051_MH2030A_RST_PIN);
    Delay_Ms(10u);
}

void dm9051_mh2030a_polling_reset(void *ctx)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;

    dm9051_mh2030a_spi1_polling_bus_init();
    dm9051_mh2030a_irq_init_if_enabled(config);
    dm9051_mh2030a_reset_gpio_sequence();
}

void dm9051_mh2030a_delay_ms(uint32_t ms)
{
    while (ms > 0xffffu) {
        Delay_Ms(0xffffu);
        ms -= 0xffffu;
    }

    Delay_Ms((uint16_t)ms);
}

void dm9051_mh2030a_delay_us(uint32_t us)
{
    Delay_Us(us);
}

uint32_t dm9051_mh2030a_enter_critical(void *ctx)
{
    uint32_t primask;

    (void)ctx;
    primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

void dm9051_mh2030a_exit_critical(void *ctx, uint32_t state)
{
    (void)ctx;
    __set_PRIMASK(state);
}

int dm9051_mh2030a_polling_read_reg(void *ctx,
                                    uint8_t reg,
                                    uint8_t *val)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;
    uint8_t dummy = 0u;
    uint8_t cmd;
    int status;

    if ((config == 0) || (val == 0)) {
        return DM9051_HAL_ERR_PARAM;
    }

    cmd = (uint8_t)(reg | DM9051_OPC_REG_R);
    dm9051_mh2030a_select();
    status = dm9051_mh2030a_transfer_byte(config, cmd, &dummy);
    DM9051_MH2030A_TRACE_PRINTF("[DM9051 HAL] read reg=0x%02X cmd=0x%02X cmd_status=%d dummy=0x%02X\r\n",
                                reg,
                                cmd,
                                status,
                                dummy);
    if (status == DM9051_HAL_OK) {
        status = dm9051_mh2030a_transfer_byte(config, 0x00u, val);
        DM9051_MH2030A_TRACE_PRINTF("[DM9051 HAL] read reg=0x%02X data_status=%d val=0x%02X\r\n",
                                    reg,
                                    status,
                                    *val);
    }
    if (status == DM9051_HAL_OK) {
        status = dm9051_mh2030a_finish_transfer(config);
    } else {
        (void)dm9051_mh2030a_finish_transfer(config);
    }
    dm9051_mh2030a_deselect();

    return status;
}

int dm9051_mh2030a_polling_write_reg(void *ctx,
                                     uint8_t reg,
                                     uint8_t val)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;
    uint8_t dummy = 0u;
    uint8_t cmd;
    int status;

    if (config == 0) {
        return DM9051_HAL_ERR_PARAM;
    }

    cmd = (uint8_t)(reg | DM9051_OPC_REG_W);
    dm9051_mh2030a_select();
    status = dm9051_mh2030a_transfer_byte(config, cmd, &dummy);
    if (status == DM9051_HAL_OK) {
        status = dm9051_mh2030a_transfer_byte(config, val, &dummy);
    }
    if (status == DM9051_HAL_OK) {
        status = dm9051_mh2030a_finish_transfer(config);
    } else {
        (void)dm9051_mh2030a_finish_transfer(config);
    }
    dm9051_mh2030a_deselect();

    return status;
}

int dm9051_mh2030a_polling_read_mem(void *ctx,
                                    uint8_t *buf,
                                    uint16_t len)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;
    uint8_t dummy;
    uint16_t i;
    int status;

    if ((config == 0) || ((buf == 0) && (len != 0u))) {
        return DM9051_HAL_ERR_PARAM;
    }

    if (len == 0u) {
        return DM9051_HAL_OK;
    }

    dm9051_mh2030a_select();
    status = dm9051_mh2030a_transfer_byte(config,
                                          (uint8_t)(DM9051_MRCMD | DM9051_OPC_REG_R),
                                          &dummy);
    for (i = 0u; (i < len) && (status == DM9051_HAL_OK); ++i) {
        status = dm9051_mh2030a_transfer_byte(config, 0x00u, &buf[i]);
    }
    if (status == DM9051_HAL_OK) {
        status = dm9051_mh2030a_finish_transfer(config);
    } else {
        (void)dm9051_mh2030a_finish_transfer(config);
    }
    dm9051_mh2030a_deselect();

    DM9051_MH2030A_DIAG_PRINTF("[DM9051 HAL] read mem len=%u status=%d\r\n",
                               len,
                               status);

    return status;
}

int dm9051_mh2030a_polling_write_mem(void *ctx,
                                     const uint8_t *buf,
                                     uint16_t len)
{
    const dm9051_mh2030a_config_t *config = (const dm9051_mh2030a_config_t *)ctx;
    uint8_t dummy;
    uint16_t i;
    int status;

    if ((config == 0) || ((buf == 0) && (len != 0u))) {
        return DM9051_HAL_ERR_PARAM;
    }

    if (len == 0u) {
        return DM9051_HAL_OK;
    }

    dm9051_mh2030a_select();
    status = dm9051_mh2030a_transfer_byte(config,
                                          (uint8_t)(DM9051_MWCMD | DM9051_OPC_REG_W),
                                          &dummy);
    for (i = 0u; (i < len) && (status == DM9051_HAL_OK); ++i) {
        status = dm9051_mh2030a_transfer_byte(config, buf[i], &dummy);
    }
    if (status == DM9051_HAL_OK) {
        status = dm9051_mh2030a_finish_transfer(config);
    } else {
        (void)dm9051_mh2030a_finish_transfer(config);
    }
    dm9051_mh2030a_deselect();

    DM9051_MH2030A_DIAG_PRINTF("[DM9051 HAL] write mem len=%u status=%d\r\n",
                               len,
                               status);

    return status;
}

void dm9051_mh2030a_default_config(dm9051_mh2030a_config_t *config)
{
    if (config == 0) {
        return;
    }

    config->transport = DM9051_MH2030A_TRANSPORT_POLLING;
    config->irq_mode = DM9051_MH2030A_IRQ_OFF;
    config->pins.cs_port = (uint32_t)DM9051_MH2030A_CS_PORT;
    config->pins.cs_pin = DM9051_MH2030A_CS_PIN;
    config->pins.sck_port = (uint32_t)DM9051_MH2030A_SCK_PORT;
    config->pins.sck_pin = DM9051_MH2030A_SCK_PIN;
    config->pins.miso_port = (uint32_t)DM9051_MH2030A_MISO_PORT;
    config->pins.miso_pin = DM9051_MH2030A_MISO_PIN;
    config->pins.mosi_port = (uint32_t)DM9051_MH2030A_MOSI_PORT;
    config->pins.mosi_pin = DM9051_MH2030A_MOSI_PIN;
    config->pins.rst_port = (uint32_t)DM9051_MH2030A_RST_PORT;
    config->pins.rst_pin = DM9051_MH2030A_RST_PIN;
    config->pins.int_port = (uint32_t)DM9051_MH2030A_INT_PORT;
    config->pins.int_pin = DM9051_MH2030A_INT_PIN;
    config->spi_timeout = DM9051_MH2030A_DEFAULT_SPI_TIMEOUT;
}

int dm9051_mh2030a_config_is_valid(const dm9051_mh2030a_config_t *config)
{
    if (config == 0) {
        return 0;
    }

    switch (config->transport) {
    case DM9051_MH2030A_TRANSPORT_POLLING:
    case DM9051_MH2030A_TRANSPORT_DMA:
        break;
    default:
        return 0;
    }

    switch (config->irq_mode) {
    case DM9051_MH2030A_IRQ_OFF:
    case DM9051_MH2030A_IRQ_EXTI:
        break;
    default:
        return 0;
    }

    if (config->spi_timeout == 0u) {
        return 0;
    }

    if ((config->pins.cs_port != (uint32_t)DM9051_MH2030A_CS_PORT) ||
        (config->pins.cs_pin != DM9051_MH2030A_CS_PIN) ||
        (config->pins.sck_port != (uint32_t)DM9051_MH2030A_SCK_PORT) ||
        (config->pins.sck_pin != DM9051_MH2030A_SCK_PIN) ||
        (config->pins.miso_port != (uint32_t)DM9051_MH2030A_MISO_PORT) ||
        (config->pins.miso_pin != DM9051_MH2030A_MISO_PIN) ||
        (config->pins.mosi_port != (uint32_t)DM9051_MH2030A_MOSI_PORT) ||
        (config->pins.mosi_pin != DM9051_MH2030A_MOSI_PIN) ||
        (config->pins.rst_port != (uint32_t)DM9051_MH2030A_RST_PORT) ||
        (config->pins.rst_pin != DM9051_MH2030A_RST_PIN) ||
        (config->pins.int_port != (uint32_t)DM9051_MH2030A_INT_PORT) ||
        (config->pins.int_pin != DM9051_MH2030A_INT_PIN)) {
        return 0;
    }

    return 1;
}

int dm9051_mh2030a_hal_bind(dm9051_hal_t *hal,
                             const dm9051_mh2030a_config_t *config)
{
    if ((hal == 0) || !dm9051_mh2030a_config_is_valid(config)) {
        return DM9051_HAL_ERR_PARAM;
    }

#if !DM9051_ENABLE_DMA
    if (config->transport == DM9051_MH2030A_TRANSPORT_DMA) {
        return DM9051_HAL_ERR_NOT_READY;
    }
#endif

#if !DM9051_ENABLE_IRQ
    if (config->irq_mode == DM9051_MH2030A_IRQ_EXTI) {
        return DM9051_HAL_ERR_NOT_READY;
    }
#endif

    dm9051_mh2030a_bound_config = *config;
    hal->ctx = &dm9051_mh2030a_bound_config;

    if (config->transport == DM9051_MH2030A_TRANSPORT_POLLING) {
        hal->ops = &dm9051_mh2030a_polling_ops;
        return DM9051_HAL_OK;
    }

#if DM9051_ENABLE_DMA
    hal->ops = &dm9051_mh2030a_dma_ops;
    return DM9051_HAL_OK;
#else
    return DM9051_HAL_ERR_NOT_READY;
#endif
}

const char *dm9051_mh2030a_transport_name(dm9051_mh2030a_transport_t transport)
{
    switch (transport) {
    case DM9051_MH2030A_TRANSPORT_POLLING:
        return "polling";
    case DM9051_MH2030A_TRANSPORT_DMA:
        return "dma";
    default:
        return "unknown";
    }
}

const char *dm9051_mh2030a_irq_name(dm9051_mh2030a_irq_mode_t irq_mode)
{
    switch (irq_mode) {
    case DM9051_MH2030A_IRQ_OFF:
        return "off";
    case DM9051_MH2030A_IRQ_EXTI:
        return "exti";
    default:
        return "unknown";
    }
}
