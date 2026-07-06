#include "at_port.h"
#include "mh22xx.h"
#include "mh22xx_usart.h"
#include "mh22xx_flash.h"
#include "mh22xx_gpio.h"
#include "mh22xx_rcc.h"
#include <string.h>

/* AT command UART: USART1 (PA9/PA10), separate from debug USART2 */
#define AT_CMD_USART        USART1
#define AT_CMD_USART_RCC    RCC_APB2Periph_USART1
#define AT_CMD_TX_PIN       GPIO_Pin_9
#define AT_CMD_TX_GPIO      GPIOA
#define AT_CMD_TX_GPIO_RCC  RCC_APB2Periph_GPIOA
#define AT_CMD_RX_PIN       GPIO_Pin_10
#define AT_CMD_RX_GPIO      GPIOA
#define AT_CMD_RX_GPIO_RCC  RCC_APB2Periph_GPIOA

/* --- AT cmd RX globals (same semantics as original atcommand.c) --- */
#define AT_RX_BUF_SIZE 1460
static char         s_rx_buf[AT_RX_BUF_SIZE];
static unsigned int s_rx_len = 0;
static int          s_rx_ready = 0;

/* ------------------------------------------------------------
 * 1. UART
 * ------------------------------------------------------------ */
void atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(AT_CMD_USART_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(AT_CMD_TX_GPIO_RCC, ENABLE);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = AT_CMD_TX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(AT_CMD_TX_GPIO, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = AT_CMD_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(AT_CMD_RX_GPIO, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate = baud;

    if (wordlen == 9)
        usart.USART_WordLength = USART_WordLength_9b;
    else
        usart.USART_WordLength = USART_WordLength_8b;

    switch (stop) {
    case 2:
        usart.USART_StopBits = USART_StopBits_2;
        break;
    default:
        usart.USART_StopBits = USART_StopBits_1;
        break;
    }

    switch (parity) {
    case 1:
        usart.USART_Parity = USART_Parity_Odd;
        break;
    case 2:
        usart.USART_Parity = USART_Parity_Even;
        break;
    default:
        usart.USART_Parity = USART_Parity_No;
        break;
    }

    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(AT_CMD_USART, &usart);
    USART_Cmd(AT_CMD_USART, ENABLE);
}

void atp_uart_putc(char c)
{
    USART_SendData(AT_CMD_USART, (uint8_t)c);
    while (USART_GetFlagStatus(AT_CMD_USART, USART_FLAG_TC) == RESET);
}

char *atp_uart_rx_buf(void)
{
    return s_rx_buf;
}

unsigned int atp_uart_rx_len(void)
{
    return s_rx_len;
}

int atp_uart_rx_ready(void)
{
    return s_rx_ready;
}

void atp_uart_rx_clear(void)
{
    memset(s_rx_buf, 0, sizeof(s_rx_buf));
    s_rx_len = 0;
    s_rx_ready = 0;
}

/* Called from USART1 RX ISR or polling */
void atp_uart_rx_feed(char c)
{
    if (s_rx_len < sizeof(s_rx_buf) - 1)
        s_rx_buf[s_rx_len++] = c;
    if (c == '\n')
        s_rx_ready = 1;
}

/* ------------------------------------------------------------
 * 2. Non-volatile storage
 *    Uses the last 2KB of the last flash page for AT settings.
 *    Adjust AT_NV_BASE_ADDR per MCU flash layout.
 * ------------------------------------------------------------ */
#define AT_NV_PAGE_ADDR     (0x0801F800) /* near end of 128KB flash */
#define AT_NV_WORDS         64

void atp_nv_erase(void)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(AT_NV_PAGE_ADDR);
}

void atp_nv_write_word(uint32_t idx, uint32_t word)
{
    FLASH_ProgramWord(AT_NV_PAGE_ADDR + idx * 4, word);
}

uint32_t atp_nv_read_word(uint32_t idx)
{
    return *(volatile uint32_t *)(AT_NV_PAGE_ADDR + idx * 4);
}

void atp_nv_commit(void)
{
    FLASH_Lock();
}

/* ------------------------------------------------------------
 * 3. System reset
 * ------------------------------------------------------------ */
void atp_system_reset(void)
{
    NVIC_SystemReset();
    for (;;);
}

/* ------------------------------------------------------------
 * 4. Timer (millisecond tick)
 * ------------------------------------------------------------ */
static volatile uint32_t s_tick_count = 0;

void atp_tick_isr(void)
{
    s_tick_count++;
}

uint32_t atp_mstick(void)
{
    return s_tick_count;
}
