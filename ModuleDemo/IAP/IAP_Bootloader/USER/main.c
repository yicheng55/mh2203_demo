#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"
#include "key.h"
#include "iap.h"

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;
#define USART_REC_LEN  			32*1024 //定义最大接收字节数 32K

void UART_Configuration(uint32_t bound);
void NVIC_Configuration(void);
void RCC_ClkConfiguration(void);

uint16_t USART_RX_STA=0;       	//接收状态标记	  
uint16_t USART_RX_CNT=0;			//接收的字节数	 
uint8_t USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));

int main(void)
{	
	uint8_t key;
	uint16_t oldcount=0;				//老的串口接收数据值
	uint16_t applenth=0;				//接收到的app代码长度
	
	RCC_ClocksTypeDef clocks;
	
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration(115200);
	NVIC_Configuration();
	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);
	
	PRINTF_LOG("IAP Bootloader Test.\n");
	PRINTF_LOG("KEY1:Copy To FLASH\n");
	PRINTF_LOG("KEY2:Run FLASH APP\n");
	
	KEY_Init();
	while(1)
	{
	 	if(USART_RX_CNT)
		{
			if(oldcount==USART_RX_CNT)
			{
				applenth=USART_RX_CNT;
				oldcount=0;
				USART_RX_CNT=0;
				PRINTF_LOG("用户程序接收完成!\r\n");
				PRINTF_LOG("代码长度:%dBytes\r\n",applenth);
			}
			else 
			{
				oldcount=USART_RX_CNT;			
			}
		}
		
		Delay_Ms(100);
		
		key=KEY_Scan();
		if(key==KEY1_PRES)
		{		
			if(applenth)
			{
				PRINTF_LOG("开始更新固件...\r\n");	
				PRINTF_LOG("Copying To FLASH...\n");
 				if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
				{	 
					IAP_Write_Appbin(FLASH_APP1_ADDR,USART_RX_BUF,applenth);//更新FLASH代码   
					PRINTF_LOG("Copy APP Successed!!\n");
					PRINTF_LOG("固件更新完成!\r\n");	
				}
				else 
				{
					PRINTF_LOG("Illegal FLASH APP!  \n");	   
					PRINTF_LOG("非FLASH应用程序!\r\n");
				}
 			}
			else 
			{
				PRINTF_LOG("没有可以更新的固件!\r\n");
				PRINTF_LOG("No APP!\n");
			}
		}
		
		if(key==KEY2_PRES)
		{	
			PRINTF_LOG("开始执行FLASH用户代码!!\r\n");
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{	 
				IAP_Load_App(FLASH_APP1_ADDR);//执行FLASH APP代码
			}
			else 
			{
				PRINTF_LOG("非FLASH应用程序,无法执行!\r\n");
				PRINTF_LOG("Illegal FLASH APP!\n");	   
			}			
		}
		
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

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
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
	USART_ITConfig(USART_TEST, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART_TEST, ENABLE);
}

void USART1_IRQHandler(void)
{
	uint8_t  res;	
	if(USART_GetFlagStatus(USART_TEST,USART_FLAG_RXNE))
	{	 
		res=USART_ReceiveData(USART_TEST);
		if(USART_RX_CNT<USART_REC_LEN)
		{
			USART_RX_BUF[USART_RX_CNT]=res;
			USART_RX_CNT++;			 									     
		}
	}
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

