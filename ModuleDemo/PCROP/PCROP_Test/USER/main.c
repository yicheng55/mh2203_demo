#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void RCC_ClkConfiguration(void);
uint8_t GetCmd(void);
void TestList(void);

typedef struct
{
	uint8_t(*MH_PCROP_Config)(uint8_t block,uint8_t state,uint32_t saddr,uint32_t eaddr);
}MH_FuncTypeDef;

#define MH_INTERLIB		((MH_FuncTypeDef*)(0x1FFFD00C))

#define TEST_CODE_NUMBER	(1024)

#pragma arm section code=".ARM.__at_0x8020000"
uint32_t FuncTestAdd(uint32_t a, uint32_t b)
{
	return (a+b);
}
#pragma arm section

#pragma arm section code=".ARM.__at_0x8020100"
uint32_t FuncTestSub(uint32_t a, uint32_t b)
{
	return (a-b);
}
#pragma arm section

int main(void)
{	
	uint8_t state;
	uint8_t cmd = 0;
	uint32_t i;
	RCC_ClocksTypeDef clocks;
	
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);
	
	PRINTF_LOG("PCROP Test. Protect Test Addr 0x8020000,Size 1KByte\n");
	TestList();

	while(1)
	{
		cmd = GetCmd();
		switch(cmd)
		{
			case '1':
			{
				for(i = 0; i < TEST_CODE_NUMBER/4; i++)
				{
					PRINTF_LOG("Addr: 0x%x, Value: 0x%x\n",(0x8020000+i*4),*(uint32_t *)(0x8020000+i*4));
				}
				break;
			}
			
			case '2':
			{
				PRINTF_LOG("FuncTestAdd ！！> %d\n",FuncTestAdd(500,100));
				PRINTF_LOG("FuncTestSub ！！> %d\n",FuncTestSub(400,100));
				break;
			}
			
			case '3':
			{
				PRINTF_LOG("Enable PCROP,Read Addr 0x8020000 ~ 0x8020400\n");
				state = MH_INTERLIB->MH_PCROP_Config(1,0xA5,0x8020000,0x8020400);
				if(state != 0)
				{
					PRINTF_LOG("state = 0x%x\n",state);
					while(1);
				}
				break;
			}
			
			case '4':
			{
				PRINTF_LOG("Disable PCROP,Read Addr 0x8020000 ~ 0x8020400\n");
				state = MH_INTERLIB->MH_PCROP_Config(1,0x00,0x8020000,0x8020400);
				if(state != 0)
				{
					PRINTF_LOG("state = 0x%x\n",state);
					while(1);
				}
				break;
			}
			
			case 'r':
			{
				TestList();
				break;
			}
		}
	}
}

void TestList(void)
{
	PRINTF_LOG("/*********************************************************/\n");
	PRINTF_LOG("1: Read Addr 0x8020000(1KByte)\n");
	PRINTF_LOG("2: Execute Addr 0x8020000 ~ 0x8020400 Function\n");
	PRINTF_LOG("3: Enable PCROP(Addr 0x8020000 ~ 0x8020400)\n");
	PRINTF_LOG("4: Disable PCROP(Addr 0x8020000 ~ 0x8020400)\n");
	PRINTF_LOG("r: Show List\n");
	PRINTF_LOG("/*********************************************************/\n");
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

uint8_t GetCmd(void)
{
	uint8_t tmp = 0;

	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE))
	{
		tmp = USART_ReceiveData(USART1);
	}
	return tmp;
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

