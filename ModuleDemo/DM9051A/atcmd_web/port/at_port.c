#define ATCMD_WEB_MH2203_FLASH_COMPAT_IMPL

#include "at_port.h"
#include "at32f4xx.h"
#include "atcommand.h"

#include <string.h>

#ifndef ATCMD_WEB_UART
#define ATCMD_WEB_UART USART2
#endif

#ifndef ATCMD_WEB_UART_IRQn
#define ATCMD_WEB_UART_IRQn USART2_IRQn
#endif

#ifndef ATCMD_WEB_NV_BASE
#define ATCMD_WEB_NV_BASE ((uint32_t)0x08020000u - (1024u * 2u))
#endif

static USART_TypeDef *atcmd_uart = ATCMD_WEB_UART;

static void atcmd_mh2203_uart_send_byte(uint8_t data)
{
    atcmd_uart->DR = (uint16_t)(data & 0x01FFu);
}

static uint16_t atcmd_wordlen_to_usart(uint8_t wordlen)
{
    (void)wordlen;
    return USART_WordLength_8b;
}

static uint16_t atcmd_parity_to_usart(uint8_t parity)
{
    if (parity == 1u) {
        return USART_Parity_Odd;
    }
    if (parity == 2u) {
        return USART_Parity_Even;
    }
    return USART_Parity_No;
}

static uint16_t atcmd_stop_to_usart(uint8_t stop)
{
    if (stop == 2u) {
        return USART_StopBits_2;
    }
    return USART_StopBits_1;
}

void atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate = baud;
    usart.USART_WordLength = atcmd_wordlen_to_usart(wordlen);
    usart.USART_StopBits = atcmd_stop_to_usart(stop);
    usart.USART_Parity = atcmd_parity_to_usart(parity);
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(atcmd_uart, &usart);
    USART_ITConfig(atcmd_uart, USART_IT_RXNE, ENABLE);
    USART_Cmd(atcmd_uart, ENABLE);

    nvic.NVIC_IRQChannel = ATCMD_WEB_UART_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1u;
    nvic.NVIC_IRQChannelSubPriority = 1u;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void atp_uart_putc(char c)
{
    while (USART_GetFlagStatus(atcmd_uart, USART_FLAG_TXE) == RESET) {
    }
    atcmd_mh2203_uart_send_byte((uint8_t)c);
    while (USART_GetFlagStatus(atcmd_uart, USART_FLAG_TC) == RESET) {
    }
}

char *atp_uart_rx_buf(void)
{
    return g_u8RecData;
}

unsigned int atp_uart_rx_len(void)
{
    return gt_u32comRbytes;
}

void atp_uart_rx_clear(void)
{
    memset(g_u8RecData, 0, sizeof(g_u8RecData));
    gt_u32comRbytes = 0u;
    atcmd_flag = FALSE;
}

int atp_uart_rx_ready(void)
{
    return atcmd_flag != FALSE;
}

void atp_nv_erase(void)
{
    (void)atcmd_mh2203_flash_erase_page(ATCMD_WEB_NV_BASE);
}

void atp_nv_write_word(uint32_t idx, uint32_t word)
{
    (void)atcmd_mh2203_flash_program_word(ATCMD_WEB_NV_BASE + (idx * 4u), word);
}

uint32_t atp_nv_read_word(uint32_t idx)
{
    return atcmd_mh2203_flash_read_word(ATCMD_WEB_NV_BASE + (idx * 4u));
}

void atp_nv_commit(void)
{
    atcmd_mh2203_flash_lock();
}

void atp_system_reset(void)
{
    NVIC_SystemReset();
}

uint32_t atp_mstick(void)
{
    extern uint32_t atcmd_web_clock_millis(void);
    return atcmd_web_clock_millis();
}

void UART_CMD_Init_Update(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
    atp_uart_init(baud, wordlen, parity, stop);
}

void AT_USART_DMA_RxToggle(uint16_t length)
{
    (void)length;
}

void DMA_Init_AT(void)
{
}

void atcmd_mh2203_flash_unlock(void)
{
    FLASH_Unlock();
}

void atcmd_mh2203_flash_lock(void)
{
    FLASH_Lock();
}

void atcmd_mh2203_flash_clear_flag(uint32_t flags)
{
    FLASH_ClearFlag(flags);
}

FLASH_Status atcmd_mh2203_flash_erase_page(uint32_t address)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_EOP);
    return FLASH_ErasePage(address);
}

FLASH_Status atcmd_mh2203_flash_program_word(uint32_t address, uint32_t data)
{
    return FLASH_ProgramWord(address, data);
}

uint32_t atcmd_mh2203_flash_read_word(uint32_t address)
{
    return *(volatile uint32_t *)address;
}

uint32_t generate_random_number(void)
{
    static uint32_t seed = 0x90512203u;

    seed = (seed * 1103515245u) + 12345u + atp_mstick();
    return seed;
}
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(atcmd_uart, USART_IT_RXNE) != RESET) {
        char ch = (char)USART_ReceiveData(atcmd_uart);

        if (gt_u32comRbytes < (RecData_Size - 1u)) {
            g_u8RecData[gt_u32comRbytes++] = ch;
            g_u8RecData[gt_u32comRbytes] = '\0';
        }

        if ((ch == '\n') || (ch == '\r')) {
            atcmd_flag = TRUE;
        }

        USART_ClearITPendingBit(atcmd_uart, USART_IT_RXNE);
    }
}
