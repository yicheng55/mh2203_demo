/*
	说明：
	1、本工程是MH2203的的数字信号处理单元DSP的示例工程。
	   根据DSP指令编写DSP代码能够驱动DSP单元独立计算数据。
	2、在mh22xx_dsp_algorithm_lib.c 中定义了可以使用的DSP函数。并且
	   与DSP代码相对应的给出了C代码做对比验证。
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"
#include "math.h"
#include "mh22xx_dsp_algorithm_lib.h"

#define PRINTF_LOG 	printf

void UART_Configuration(void);
void RCC_ClkConfiguration(void);

float f_dat[256] = {0};
float f_dat2[16] = {0};

int main(void)
{	 
	RCC_ClocksTypeDef clocks;
	uint16_t i = 0;
	uint16_t num = 10;
	
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration();

	RCC_GetClocksFreq(&clocks);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DSP, ENABLE);
	RCC_AHBPeriphResetCmd(RCC_AHBPeriphReset_DSP, ENABLE);
	RCC_AHBPeriphResetCmd(RCC_AHBPeriphReset_DSP, DISABLE);
	
	PRINTF_LOG("\nSYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);	
	PRINTF_LOG("FPU Demo...\n");
	
	DSP->CR2 |= DSP_CR2_S_ADDR_32K;
	DSP_Space_Mode_Config(DSP_SPACE_MODE_32K);
	
	DSP_FUNC_Write(); // DSP指令写入
	
	/* 数组填充 */
	for(i = 0; i<255; i++) {
		f_dat[i] = (i + 1) * 1.1;
	}
	for(i = 0; i<16; i++) {
		f_dat2[i] = (i + 1) * 2.5;
	}	
	
	/* sin()正弦函数 */
	PRINTF_LOG("Software_Sin() = %f\n", Software_Sin(1.047));
	PRINTF_LOG("DSP_SIN() = %f\n\n", DSP_SIN(1.047));		
	
	/* cos()余弦函数 */
	PRINTF_LOG("Software_Cos() = %f\n", Software_Cos(1.047));
	PRINTF_LOG("DSP_COS() = %f\n\n", DSP_COS(1.047));	
							
	/* sum()数组求和函数 */	
	PRINTF_LOG("Software_SUM(f_dat, num) = %f\n", Software_SUM(f_dat, num));
	PRINTF_LOG("DSP_SUM(f_dat, num) = %f\n\n", DSP_SUM(f_dat, num));	
	
	/* sub()数组求差函数 */
	PRINTF_LOG("Software_SUB(f_dat, num) = %f\n", Software_SUB(f_dat, num));
	PRINTF_LOG("DSP_SUB(f_dat, num) = %f\n\n", DSP_SUB(f_dat, num));	

	/* prdct()数组求积函数 */
	PRINTF_LOG("Software_PRDCT(f_dat, num) = %f\n", Software_PRDCT(f_dat, num));
	PRINTF_LOG("DSP_PRDCT(f_dat, num) = %f\n\n", DSP_PRDCT(f_dat, num));
				
	/* SUMSQ()数组平方求和函数 */
	PRINTF_LOG("Software_SUMSQ(f_dat, num) = %f\n", Software_SUMSQ(f_dat, num));
	PRINTF_LOG("DSP_SUMSQ(f_dat, num) = %f\n\n", DSP_SUMSQ(f_dat, num));				
					
	/* DOT()数组点乘求和函数 */	
	PRINTF_LOG("Software_DOT(f_dat, f_dat2, num) = %f\n", Software_DOT(f_dat, f_dat2, num));
	PRINTF_LOG("DSP_DOT(f_dat, f_dat2, num) = %f\n\n", DSP_DOT(f_dat, f_dat2, num));	
								
	/* I2F()整型转浮点型函数 */	
	PRINTF_LOG("Software_I2F(num) = %f\n", Software_I2F(num));
	PRINTF_LOG("DSP_I2F(num) = %f\n\n", DSP_I2F(num));	

	/* F2I()浮点型转整型函数 */	
	PRINTF_LOG("Software_F2I(f_dat[0]) = %d\n", Software_F2I(f_dat[8]));
	PRINTF_LOG("DSP_F2I(f_dat[0]) = %d\n\n", DSP_F2I(f_dat[8]));	
	
	/* CMP_F()浮点数比较函数 */		
	PRINTF_LOG("Software_CMP_F(f_dat[0], f_dat[1]) = %d\n", Software_CMP_F(f_dat[0], f_dat[1]));
	PRINTF_LOG("DSP_CMP_F(f_dat[0], f_dat[1]) = %d\n\n",  DSP_CMP_F(f_dat[0], f_dat[1]));		

	/* DIV_F()浮点数除法函数 */		
	PRINTF_LOG("Software_DIV_F(f_dat[0], f_dat[1]) = %f\n", Software_DIV_F(f_dat[0], f_dat[1]));
	PRINTF_LOG("DSP_DIV_F(f_dat[0], f_dat[1]) = %f\n\n", DSP_DIV_F(f_dat[0], f_dat[1]));	

	/* INVSQRT_F()平方根倒数函数 */		
	PRINTF_LOG("Software_INVSQRT_F(f_dat[0]) = %f\n", Software_INVSQRT_F(f_dat[0]));
	PRINTF_LOG("DSP_INVSQRT_F(f_dat[0]) = %f\n\n", DSP_INVSQRT_F(f_dat[0]));	

	/* MAC_F()乘加函数 */		
	PRINTF_LOG("Software_MAC_F(f_dat[0], f_dat[1], f_dat[2]) = %f\n", Software_MAC_F(f_dat[0], f_dat[1], f_dat[2]));
	PRINTF_LOG("DSP_MAC_F(f_dat[0], f_dat[1], f_dat[2]) = %f\n\n", DSP_MAC_F(f_dat[0], f_dat[1], f_dat[2]));				

	/* TAN()正切函数 */
	PRINTF_LOG("Software_tan(theta) = %f\n",  Software_tan(1.8));
	PRINTF_LOG("DSP_TAN(theta) = %f\n\n", DSP_TAN(1.8));	
		
	/* ASIN()反正弦函数 */		
	PRINTF_LOG("Software_ASIN(result_func) = %f\n", Software_ASIN(0.8660249));
	PRINTF_LOG("DSP_ASIN(result_func) = %f\n\n", DSP_ASIN(0.8660249));			
				
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

