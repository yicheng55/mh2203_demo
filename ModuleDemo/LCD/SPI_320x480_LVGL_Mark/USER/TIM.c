#include "User_Inc.h"

void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef       NVIC_Structure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	  
	NVIC_Structure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_Structure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Structure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_Structure.NVIC_IRQChannelSubPriority=0;
	
	TIM_TimeBaseStructure.TIM_Period = 999;
	TIM_TimeBaseStructure.TIM_Prescaler = 216000000/1000000 - 1; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM2, ENABLE); 
	NVIC_Init(&NVIC_Structure);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2, ENABLE);  

}
void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	lv_tick_inc(1);
}
