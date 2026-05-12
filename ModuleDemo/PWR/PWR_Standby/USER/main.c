#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
uint8_t GetCmd(void);
void RCC_ClkConfiguration(void);
void WAKEUP_Configuration(void);

int main(void)
{	
	RCC_ClocksTypeDef clocks;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration(115200);

	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);
	
	PRINTF_LOG("PWR Standby Test.\n");
	
	PRINTF_LOG("1:Enable WakeUp1 Pin - PA0\n");
	PRINTF_LOG("2:Enable WakeUp2 Pin - PC13\n");
	PRINTF_LOG("3:Enable WakeUp3 Pin - PB4\n");
	PRINTF_LOG("4:Enable WakeUp4 Pin - PB5\n");
	PRINTF_LOG("5:Enable WakeUp5 Pin - PB6\n");
	PRINTF_LOG("6:Enable WakeUp6 Pin - PC8\n");
	PRINTF_LOG("7:Enable WakeUp7 Pin - PC9\n");
	PRINTF_LOG("8:Enable WakeUp8 Pin - PC11\n");
	PRINTF_LOG("9:Enable WakeUp9 Pin - PC12\n");
	PRINTF_LOG("a:Enable WakeUp10 Pin - PD2\n");
	
	WAKEUP_Configuration();
	
	PRINTF_LOG("Please Input 's', Come Standby Mode\n");
	while(GetCmd() != 's');
	GPIO_Configuration();
	PWR_EnterSTANDBYMode();

	while(1);
}

void WAKEUP_Configuration(void)
{
	uint8_t cmd;

	PWR_WakeUpPinCmd(PWR_WakeUpPin_1,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_2,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_3,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_4,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_5,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_6,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_7,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_8,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_9,DISABLE);
	PWR_WakeUpPinCmd(PWR_WakeUpPin_10,DISABLE);
	while(1)
	{
		cmd = GetCmd();
		
		switch (cmd)
		{
			case '1':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_1,ENABLE);
				PRINTF_LOG("Enable WakeUp1 Success\n");
				return;
			}
			
			case '2':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_2,ENABLE);
				PRINTF_LOG("Enable WakeUp2 Success\n");
				return;
			}
			
			case '3':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_3,ENABLE);
				PRINTF_LOG("Enable WakeUp3 Success\n");
				return;
			}
			
			case '4':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_4,ENABLE);
				PRINTF_LOG("Enable WakeUp4 Success\n");
				return;
			}

			case '5':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_5,ENABLE);
				PRINTF_LOG("Enable WakeUp5 Success\n");
				return;
			}
			
			case '6':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_6,ENABLE);
				PRINTF_LOG("Enable WakeUp6 Success\n");
				return;
			}

			case '7':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_7,ENABLE);
				PRINTF_LOG("Enable WakeUp7 Success\n");
				return;
			}
			
			case '8':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_8,ENABLE);
				PRINTF_LOG("Enable WakeUp8 Success\n");
				return;
			}
			
			case '9':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_9,ENABLE);
				PRINTF_LOG("Enable WakeUp9 Success\n");
				return;
			}

			case 'a':
			{
				PWR_WakeUpPinCmd(PWR_WakeUpPin_10,ENABLE);
				PRINTF_LOG("Enable WakeUp10 Success\n");
				return;
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

void GPIO_Configuration(void)
{
	uint8_t i;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|\
							RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	for(i = 0; i < (GPIOG_BASE-GPIOA_BASE)/0x400; i++)
	{
		GPIO_Init((GPIO_TypeDef *)((APB2PERIPH_BASE + (i+3)*0x0400)), &GPIO_InitStructure);
		
	}
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
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

uint8_t GetCmd(void)
{
	uint8_t tmp = 0;

	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE))
	{
		tmp = USART_ReceiveData(USART1);
	}
	return tmp;
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

