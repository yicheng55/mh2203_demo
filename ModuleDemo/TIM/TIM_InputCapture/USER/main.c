#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void TIM_Configuration(void);
void RCC_ClkConfiguration(void);
RCC_ClocksTypeDef clocks;

uint8_t  TIM3_CAPTURE_STA=0;	//输入捕获状态		    				
uint16_t TIM3_CAPTURE_VAL;		//输入捕获值
int main(void)
{	
	uint32_t temp=0; 
	RCC_ClkConfiguration();
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000);

	PRINTF_LOG("TIM Input Capture Test.\n");
	
	TIM_Configuration();
	
	while(1)
	{
 		if(TIM3_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
		{
			temp=TIM3_CAPTURE_STA&0X3F;
			temp*=65536;//溢出时间总和
			temp+=TIM3_CAPTURE_VAL;//得到总的高电平时间
			PRINTF_LOG("HIGH:%d us\r\n",temp);//打印总的高点平时间
			TIM3_CAPTURE_STA=0;//开启下一次捕获
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


TIM_ICInitTypeDef  TIM_ICInitStructure;
void TIM_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	

   //设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = 0XFFFF; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency*2/1000000 - 1; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2; 
  	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
   	TIM_Cmd(TIM3,ENABLE);
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

 
//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 
 	if((TIM3_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{
			if(TIM3_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM3_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM3_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM3_CAPTURE_VAL=0XFFFF;
				}else TIM3_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)//捕获1发生捕获事件
		{	
			if(TIM3_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM3_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM3_CAPTURE_VAL=TIM_GetCapture2(TIM3);
				TIM_OC2PolarityConfig(TIM3,TIM_ICPolarity_Rising); //设置为上升沿捕获
			}else  							//还未开始,第一次捕获上升沿
			{
				TIM3_CAPTURE_STA=0;			//清空
				TIM3_CAPTURE_VAL=0;
				TIM_SetCounter(TIM3,0);
				TIM3_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
				TIM_OC2PolarityConfig(TIM3,TIM_ICPolarity_Falling);	//设置为下降沿捕获
			}
		}
 	}
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC2|TIM_IT_Update); //清除中断标志位
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

