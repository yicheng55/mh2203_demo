/*
	说明：
	1、本工程是MH2203的的浮点运算单元FPU的示例工程。主要使用了
	   __aeabi_xx()接口来调用MCU所支持的FPU。在mh22xx_dsp.c
	   文件中定义了可以使用的__aeabi_xx()接口函数。
	2、本工程Software_Sin()函数以泰勒展开的形式计算了正弦函数sin()。
	   在该函数中包含了浮点数的加减乘操作，在函数被执行时将会调用
	   FPU的__aeabi_xx()接口。
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"
#include "math.h"


#define PRINTF_LOG 	printf

void UART_Configuration(void);
void RCC_ClkConfiguration(void);

/*
	绝对值函数；
*/
double myabs(double num1)
{
    return((num1 > 0) ? num1 : -num1);
}
/*
	软件计算sin()的泰勒展开；
*/
float Software_Sin(float num2)
{
	float dat =  fmodf(num2, 6.2831852);		
    int i = 1, negation = 1; // 取反
    float sum;
    float index = dat;   // 指数
    float Factorial = 1;  // 阶乘
    float TaylorExpansion = dat; // 泰勒展开式求和
	
    do
    {
        Factorial = Factorial * (i + 1) * (i + 2);//求阶乘
        index *= dat * dat;  //求num2的次方
        negation = -negation;  //每次循环取反
        sum = index / Factorial * negation;
        TaylorExpansion += sum;
        i += 2;
    } while (myabs(sum) > 1e-8);
    return(TaylorExpansion);
}

int main(void)
{	 
	RCC_ClocksTypeDef clocks = {0};
	
	RCC_ClkConfiguration();
	UART_Configuration();

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DSP, ENABLE);
	RCC_AHBPeriphResetCmd(RCC_AHBPeriphReset_DSP, ENABLE);
	RCC_AHBPeriphResetCmd(RCC_AHBPeriphReset_DSP, DISABLE);
	
	RCC_GetClocksFreq(&clocks);
	PRINTF_LOG("\nSYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);	
	PRINTF_LOG("FPU Demo...\n");
	
	PRINTF_LOG("Software_Sin() = %f\n", Software_Sin(1.0472));  // 计算1.0472弧度的正弦值，并打印结果
	while(1) {;}
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

void UART_Configuration(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	USART_Init(USART1, &USART_InitStructure); 	
	USART_Cmd(USART1, ENABLE);      
}

//Retarget Printf
int SER_PutChar (int ch)
{
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TC));
	USART_SendData(USART1, (uint8_t) ch);

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

