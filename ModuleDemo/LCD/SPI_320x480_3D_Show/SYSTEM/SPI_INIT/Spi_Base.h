#ifndef _USER_INC_H
#define _USER_INC_H
#include 	"mh22xx.h"
#include "fonts.h"
#include "delay.h"
#define BGR 0x08
#define COLORORDER BGR


#define  BK_H  GPIOC->BSRR|=1<<14;  
#define  BK_L  GPIOC->BRR|=1<<14;

#define  RST_H  GPIOC->BSRR|=1<<15;
#define  RST_L  GPIOC->BRR|=1<<15;

#define  CS_H  GPIOB->BSRR|=1<<12;
#define  CS_L  GPIOB->BRR|=1<<12;

#define  DC_H  GPIOA->BSRR|=1<<6;
#define  DC_L  GPIOA->BRR|=1<<6;


#define LCD_WIDTH  320
#define LCD_HEIGHT 480   //更换屏幕对应修改分辨率 

#define OLED_COLUMN_OFFSET 0

#define RED 	0XF800
#define GREEN  	0X07E0
#define BLUE 	0X001F
#define BLACK 	0X0000
#define WHITE 	0XFFFF

#define      		ST7796_DispWindow_X_Star		    				0     //起始点的X坐标
#define      		ST7796_DispWindow_Y_Star		    				0     //起始点的Y坐标

#define 			ST7796_LESS_PIXEL	  								320			//液晶屏较短方向的像素宽度
#define 			ST7796_MORE_PIXEL	 							  	480			//液晶屏较长方向的像素宽度

//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				  //LCD ID
	u8  dir;			  //横屏还是竖屏控制：0，竖屏；1，横屏。	
	u8	wramcmd;		//开始写gram指令
	u8  setxcmd;		//设置x坐标指令
	u8  setycmd;		//设置y坐标指令	 
}_lcd_dev;
void LCD_ClearLine(uint16_t Line);
void LCD_SetTextColor(uint16_t Color);
void ST7796_Clear ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );

void ST7796_DispString_EN ( 	uint16_t usX ,uint16_t usY,  char * pStr );
void ST7796_DispString_EN_YDir (	 uint16_t usX,uint16_t usY ,  char * pStr );
void ST7796_DispStringLine_EN (  uint16_t line,  char * pStr );
void LCD_SetFont(sFONT *fonts);
sFONT *LCD_GetFont(void);
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor);
void LCD_Scan_Dir(u8 dir);
void ST7796_SetCursor ( uint16_t usX, uint16_t usY );
void ST7796_Write_Data ( uint8_t usCmd );
void ST7796_Write_Cmd ( uint8_t usCmd );
void ST7796_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);
void ST7796_Rst (void);
void ST7796_REG_Config ( void );
void ST7796_Init(void);
void TIM_Configuration(void);
void SPI_Configuration(void);
#endif /*_USER_INC_H*/

