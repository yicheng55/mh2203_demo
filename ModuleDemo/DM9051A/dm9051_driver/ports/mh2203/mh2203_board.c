#include "mh2203_board.h"
#include "mh2203_platform.h"
#include "udp_printf.h"

#include <stdarg.h>
#include <stdio.h>

static USART_TypeDef *uip_uart = USART2;

static void configure_debug_uart(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

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
    usart.USART_BaudRate = baudrate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uip_uart, &usart);
    USART_Cmd(uip_uart, ENABLE);
}

void mh2203_uip_clock_init(void)
{
    RCC_DeInit();

    RCC_HSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) {
    }

    RCC_PLLCmd(DISABLE);
    FLASH_Unlock();
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_Lock();

    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_18);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
    }

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
}

void mh2203_uip_board_init(uint32_t baudrate)
{
    RCC_ClocksTypeDef clocks;

    mh2203_uip_clock_init();
    Delay_Init();
    configure_debug_uart(baudrate);

    RCC_GetClocksFreq(&clocks);
    printf("\r\n[MH2203 uIP] board init\r\n");
    printf("[MH2203 uIP] SYSCLK=%lu HCLK=%lu PCLK=%lu\r\n",
           clocks.SYSCLK_Frequency,
           clocks.HCLK_Frequency,
           clocks.PCLK1_Frequency);
}

int SER_PutChar(int ch)
{
    while (USART_GetFlagStatus(uip_uart, USART_FLAG_TC) == RESET) {
    }
    USART_SendData(uip_uart, (uint8_t)ch);
    return ch;
}

int fputc(int ch, FILE *f)
{
    (void)f;

#if PRINTF_DEBUG_OUTPUT == PRINTF_DEBUG_OUTPUT_UDP
    /* 尚未有 UDP peer (udp_printf_is_ready()==0) 時退回 UART，
     * 避免開機/連線建立階段的診斷訊息在對端連上前被吃掉。 */
    if (udp_printf_is_ready()) {
        if (ch == '\n') {
            udp_printf_putchar('\r');
        }
        return udp_printf_putchar(ch);
    }
#endif

    if (ch == '\n') {
        SER_PutChar('\r');
    }
    return SER_PutChar(ch);
}
