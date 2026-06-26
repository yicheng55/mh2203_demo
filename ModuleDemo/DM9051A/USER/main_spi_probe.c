#ifndef MH2203_UIP_PORT

#include <stdio.h>
#include <stdarg.h>
#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif
#include "mh22xx.h"
#include "delay.h"
#include "mh2203_spi1.h"
#include "dm9051a.h"

#define PRINTF_LOG               printf

static USART_TypeDef *USART_TEST = USART2;

static void CLK_Configuration(void);
static void UART_Configuration(uint32_t baud);
static void DM9051A_PrintProbe(void);
static void Clock_Log(const char *fmt, ...);
static void Clock_PrintConfig(void);
int SER_PutChar(int ch);

int main(void)
{
    CLK_Configuration();
    Delay_Init();
    UART_Configuration(115200);

    Clock_PrintConfig();

    PRINTF_LOG("\r\nDM9051A SPI1 probe start (MH2203)\r\n");
    PRINTF_LOG("Pins: PA15 CS(GPIO), PB3 SCK, PB5 MOSI, PB4 MISO\r\n");

    DM9051A_Init();
    Delay_Ms(10);
#if DM9051A_SPI_DEBUG
    DM9051A_DebugDump("after init");
#endif
    DM9051A_PrintProbe();

    while (1) {
        Delay_Ms(1000);
    }
}

static void DM9051A_PrintProbe(void)
{
    uint8_t chipr;
    uint16_t vid, pid;

    vid = DM9051A_ReadVID();
    pid = DM9051A_ReadPID();
    chipr = DM9051A_ReadReg(DM9051A_CHIPR);

    PRINTF_LOG("DM9051A VID=0x%04X PID=0x%04X CHIPR=0x%02X\r\n", vid, pid, chipr);

    if (vid == 0x0A46u) {
        PRINTF_LOG("DM9051A SPI read OK\r\n");
    } else {
        PRINTF_LOG("DM9051A SPI read abnormal, check PA15/PB3/PB5/PB4 wiring and CS polarity\r\n");
    }
}

static void Clock_Log(const char *fmt, ...)
{
    char buffer[96];
    va_list args;
    int i;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    printf("%s", buffer);
}

static void Clock_PrintConfig(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    Clock_Log("Clock mode: HSI/2 + PLL (72MHz target)\r\n");
    Clock_Log("PLL source: HSI/2, PLL mul: x18\r\n");
    Clock_Log("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK: %3.1fMhz\r\n",
        (float)clocks.SYSCLK_Frequency / 1000000,
        (float)clocks.HCLK_Frequency   / 1000000,
        (float)clocks.PCLK_Frequency   / 1000000);
}

static void CLK_Configuration(void)
{
    RCC_DeInit();
    RCC_HSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) { }
    RCC_PLLCmd(DISABLE);
    FLASH_Unlock();
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_Lock();
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_18);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) { }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLKConfig(RCC_HCLK_Div1);
}

static void UART_Configuration(uint32_t baud)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate = baud;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART_TEST, &usart);
    USART_Cmd(USART_TEST, ENABLE);
}

int SER_PutChar(int ch)
{
    while (USART_GetFlagStatus(USART_TEST, USART_FLAG_TC) == RESET) { }
    USART_SendData(USART_TEST, (uint8_t)ch);
    return ch;
}

int fputc(int ch, FILE *f)
{
    (void)f;
    if (ch == '\n') {
        SER_PutChar('\r');
    }
    return SER_PutChar(ch);
}

#endif /* MH2203_UIP_PORT */
