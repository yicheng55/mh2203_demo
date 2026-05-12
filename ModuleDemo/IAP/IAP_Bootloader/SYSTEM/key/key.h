#ifndef __KEY_H
#define __KEY_H	 
	
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"

#define KEY1_PIN	GPIO_Pin_1
#define KEY1_GROUP	GPIOB
#define KEY1_PERIPH	RCC_APB2Periph_GPIOB

#define KEY2_PIN	GPIO_Pin_0
#define KEY2_GROUP	GPIOB
#define KEY2_PERIPH	RCC_APB2Periph_GPIOB

#define KEY1  	GPIO_ReadInputDataBit(KEY1_GROUP,KEY1_PIN)
#define KEY2   	GPIO_ReadInputDataBit(KEY2_GROUP,KEY2_PIN)

#define KEY1_PRES	1	//KEY1按下
#define KEY2_PRES	2	//KEY2按下

void KEY_Init(void);
uint8_t KEY_Scan(void);

#endif

