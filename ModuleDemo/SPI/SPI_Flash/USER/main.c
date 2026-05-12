#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"
#include "w25qxx.h"	 

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
void RCC_ClkConfiguration(void);

//要写入到FLASH的字符串数组
const u8 TEXT_Buffer[]={"MEGAHUNT MH2103 SPI TEST"};
#define SIZE sizeof(TEXT_Buffer)
	
int main(void)
{	
	RCC_ClocksTypeDef clocks;
	uint32_t FLASH_SIZE=128*1024*1024;	//FLASH 大小为16M字节
	uint8_t datatemp[SIZE];
	
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration(115200);

	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);
	PRINTF_LOG("SPI FLASH Test.\n");
	
	W25QXX_Init();	
	while(W25QXX_ReadID()!=W25Q64)								//检测不到
	{
		PRINTF_LOG("W25Q64 Check Failed!\n");
		PRINTF_LOG("Read ID: 0x%x\n",W25QXX_ReadID());
		Delay_Ms(2000);
	}
	
	PRINTF_LOG("W25Q64 Ready!\n");    
	
	while(1)
	{
		PRINTF_LOG("Start Write W25Q64....\n"); 
		W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);			//从倒数第100个地址处开始,写入SIZE长度的数据
		PRINTF_LOG("W25Q128 Write Finished!\n");	//提示传送完成
		Delay_Ms(2000);
		PRINTF_LOG("Start Read W25Q64.... \n");
		W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//从倒数第100个地址处开始,读出SIZE个字节
		PRINTF_LOG("The Data Readed Is:  \n");	//提示传送完成
		PRINTF_LOG("%s\n",datatemp);
		Delay_Ms(2000);
	}
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

