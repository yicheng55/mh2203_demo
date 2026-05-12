#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"
#include "bsp_ST7796_lcd.h"
#include "fonts.h"
#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void RCC_ClkConfiguration(void);
void LCD_Test(void);	
void Delay ( __IO uint32_t nCount );

	
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
	
	PRINTF_LOG("ST7796 320x480 4Inch Test.\n");
	
	ST7796_Init();


	while ( 1 )
	{
		LCD_Test();
	}
}

/*用于测试各种液晶的函数*/
/*用于测试各种液晶的函数*/
void LCD_Test(void)
{
	/*演示显示变量*/
	static uint8_t testCNT = 0;	
	char dispBuff[100];
	Dma_Init();
	testCNT++;	

	LCD_SetFont(&Font8x16);
	LCD_SetColors(RED,BLACK);

	ST7796_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
	
	/********显示字符串示例*******/

	ST7796_DispStringLine_EN(LINE(0),"1.54 TFT SPI");
	ST7796_DispStringLine_EN(LINE(1),"LCD_W:320");
	ST7796_DispStringLine_EN(LINE(2),"LCD_H:480");

	Delay_Ms(1000);
	ST7796_Clear(0,16*5,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);	/* 清屏，显示全黑 */
	/* 画直线 */



	LCD_SetTextColor(RED);
	ST7796_DrawLine(37,45+100,157,200+100);  
	ST7796_DrawLine(37,30+100,157,175+100);

	LCD_SetTextColor(GREEN);
	ST7796_DrawLine(75,85+100,220,165+100);  
	ST7796_DrawLine(50,100+100,165,210+100);

	LCD_SetTextColor(BLUE);
	ST7796_DrawLine(62,85+100,182,165+100);  
	ST7796_DrawLine(37,10+100,165,220+100);

	Delay_Ms(1000);
	ST7796_Clear(0,16*5,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);	/* 清屏，显示全黑 */
	
	/*画矩形*/


	LCD_SetTextColor(WHITE);
	ST7796_DrawRectangle(50,200,120,120,0);
	ST7796_DrawRectangle(50,200,170,170,0);
	ST7796_DrawRectangle(50,200,220,220,0);


	Delay_Ms(1000);
	ST7796_Clear(0,16*5,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);	/* 清屏，显示全黑 */

	/* 画圆 */

	LCD_SetTextColor(RED);
	ST7796_DrawCircle(120,120+100,25,1);
	ST7796_DrawCircle(120,120+100,50,0);
	ST7796_DrawCircle(120,120+100,75,0);
	ST7796_DrawCircle(120,120+100,100,0);


	Delay_Ms(1000);
	ST7796_Clear(0,16*5,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);	/* 清屏，显示全黑 */
	while(1);
}

/**
  * @brief  简单延时函数
  * @param  nCount ：延时计数值
  * @retval 无
  */	
static void Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
	
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

