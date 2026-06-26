
#include "uip.h"
#include "timer.h" //"timer.h" or "uip_timer.h"
#include "web_led.h"
#include "includes.h"

char sCurrLED[4]= {'o', 'n', 0}; 
char sCurrP05[6]= {'o', 'f', 'f', 0};

int CurrTmp = 0;
int CurrHum = 0;

uint32_t LED_timer_flag;

void Delay(uint32_t times)
{
	while(times--)
	{
		uint32_t i;
		for (i=0; i<0xffff; i++);
	}
}

void Set_LED_mode(char lkkcode)
{
	//int i;
	
	if(lkkcode == ('0'))
	{
		//PB1 = 0;
		
		GPIO_SetBits(GPIOC,GPIO_Pins_13);
		GPIO_SetBits(GPIOC,GPIO_Pins_14);
		GPIO_SetBits(GPIOC,GPIO_Pins_15);
	}else if (lkkcode == '1'){
		//PB1 = 1;
		
		GPIO_ResetBits(GPIOC,GPIO_Pins_13);
		GPIO_ResetBits(GPIOC,GPIO_Pins_14);
		GPIO_ResetBits(GPIOC,GPIO_Pins_15);
	}else if(lkkcode == '2')
	{
		printf("hello2\r\n");
		//for(i = 0 ; i< 30 ; ++i)
		{
			//PB1 = 1;
			Delay(25);
			//PB1 = 0;
			Delay(25);
		}
	}
}
