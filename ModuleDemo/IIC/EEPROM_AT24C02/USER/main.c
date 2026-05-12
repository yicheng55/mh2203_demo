#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"
#include "bsp_i2c_ee.h"

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void RCC_ClkConfiguration(void);
uint8_t I2C_Test(void);

uint8_t I2C_BUF_WRITE[0xFF];
uint8_t I2C_BUF_READ[0xFF];

int main(void)
{	
	RCC_ClocksTypeDef clocks;
	
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);
	
	PRINTF_LOG("EEPROM AT24C02 Test.\n");
	
	I2C_EE_Init();
	if(I2C_Test() ==1)
	{
		PRINTF_LOG("EEPROM AT24C02 Test OK\n");
	}
	else
	{
		PRINTF_LOG("EEPROM AT24C02 Test FAIL\n");
	}
	
	while(1)
	{

	}
}

uint8_t I2C_Test(void)
{
	uint32_t i = 0;
	
	PRINTF_LOG("写入的数据:\n\r");
	for(i = 0; i < 0xFF; i++)
	{
		I2C_BUF_WRITE[i] = i;
		PRINTF_LOG("0x%02X ", I2C_BUF_WRITE[i]);
		if(i%16 == 15)    
		PRINTF_LOG("\n\r");  
	}
	
	//将I2C_BUF_WRITE中顺序递增的数据写入EERPOM中 
	I2C_EE_BufferWrite( I2C_BUF_WRITE, 0x00, 256);

	PRINTF_LOG("\n\r写成功\n\r");

	PRINTF_LOG("\n\r读出的数据\n\r");
	//将EEPROM读出数据顺序保持到I2C_BUF_READ中
	I2C_EE_BufferRead(I2C_BUF_READ, 0x00, 256); 

	//将I2c_Buf_Read中的数据通过串口打印
	for (i=0; i<0xFF; i++)
	{	
		if(I2C_BUF_READ[i] != I2C_BUF_WRITE[i])
		{
			PRINTF_LOG("0x%02X ", I2C_BUF_READ[i]);
			PRINTF_LOG("错误:I2C EEPROM写入与读出的数据不一致\n\r");
			return 0;
		}
		PRINTF_LOG("0x%02X ", I2C_BUF_READ[i]);
		if(i%16 == 15)    
		PRINTF_LOG("\n\r");

	}
	PRINTF_LOG("\nI2C(AT24C02)读写测试成功\n\r");	
	
	return 1;
}

void RCC_ClkConfiguration(void)
{
	RCC_DeInit();

	RCC_HSEConfig(RCC_HSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
	
	RCC_PLLCmd(DISABLE);
	
	FLASH_SetLatency(FLASH_Latency_2);
	
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_27);
	
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
	
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	
	RCC_LSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
	RCC_HSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
}

void UART_Configuration(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART_TEST, &USART_InitStructure);
	USART_Cmd(USART_TEST, ENABLE);
}

int SER_PutChar (int ch)
{
	while(!USART_GetFlagStatus(USART_TEST,USART_FLAG_TC));
	USART_SendData(USART_TEST, (uint8_t) ch);

	return ch;
}

int fputc(int c, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	if (c == '\n')
	{
		SER_PutChar('\r');
	}
	return (SER_PutChar(c));
}

