#ifndef      __BSP_ST7796_LCD_H
#define	      __BSP_ST7796_LCD_H

#include "mh22xx.h"
#include "fonts.h"


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

#define LCD_WIDTH  320
#define LCD_HEIGHT 480   //更换屏幕对应修改分辨率 
//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 


/***************************************************************************************
2^26 =0X0400 0000 = 64MB,每个 BANK 有4*64MB = 256MB
64MB:FSMC_Bank1_NORSRAM1:0X6000 0000 ~ 0X63FF FFFF
64MB:FSMC_Bank1_NORSRAM2:0X6400 0000 ~ 0X67FF FFFF
64MB:FSMC_Bank1_NORSRAM3:0X6800 0000 ~ 0X6BFF FFFF
64MB:FSMC_Bank1_NORSRAM4:0X6C00 0000 ~ 0X6FFF FFFF

选择BANK1-BORSRAM4 连接 TFT，地址范围为0X6C00 0000 ~ 0X6FFF FFFF
FSMC_A23 接LCD的DC(寄存器/数据选择)脚
寄存器基地址 = 0X6C00 0000
RAM基地址 = 0X6D00 0000 = 0X6C00 0000+2^23*2 = 0X6C00 0000 + 0X100 0000 = 0X6D00 0000
当选择不同的地址线时，地址要重新计算  
****************************************************************************************/

//FSMC_Bank1_NORSRAM用于LCD命令操作的地址
#define      FSMC_Addr_ST7796_CMD         ( ( uint32_t ) 0x6C000000 )

//FSMC_Bank1_NORSRAM用于LCD数据操作的地址      
#define      FSMC_Addr_ST7796_DATA        ( ( uint32_t ) 0x6C000001 )

//由片选引脚决定的NOR/SRAM块
#define      FSMC_Bank1_NORSRAMx           FSMC_Bank1_NORSRAM4

/******************************* ST7796 显示屏8080通讯引脚定义 ***************************/
/******控制信号线******/
//片选，选择NOR/SRAM块
#define      ST7796_CS_CLK                RCC_APB2Periph_GPIOA   	//PA7 NE4
#define      ST7796_CS_PORT               GPIOA
#define      ST7796_CS_PIN                GPIO_Pin_7

//DC引脚，使用FSMC的地址信号控制，本引脚决定了访问LCD时使用的地址
//PB13为FSMC_A0
#define      ST7796_DC_CLK                RCC_APB2Periph_GPIOB   //PB13 A0
#define      ST7796_DC_PORT               GPIOB
#define      ST7796_DC_PIN                GPIO_Pin_13

//写使能
#define      ST7796_WR_CLK                RCC_APB2Periph_GPIOA   //NWE PA6
#define      ST7796_WR_PORT               GPIOA
#define      ST7796_WR_PIN                GPIO_Pin_6//NWE

//读使能
#define      ST7796_RD_CLK                RCC_APB2Periph_GPIOA   //NOE PA8
#define      ST7796_RD_PORT               GPIOA
#define      ST7796_RD_PIN                GPIO_Pin_8//NOE

//复位引脚
#define      ST7796_RST_CLK               RCC_APB2Periph_GPIOC   //PB11
#define      ST7796_RST_PORT              GPIOC
#define      ST7796_RST_PIN               GPIO_Pin_15

//背光引脚
#define      ST7796_BK_CLK                RCC_APB2Periph_GPIOC  //PB1
#define      ST7796_BK_PORT               GPIOC
#define      ST7796_BK_PIN                GPIO_Pin_14

/********数据信号线***************/
#define      ST7796_D0_CLK                RCC_APB2Periph_GPIOB
#define      ST7796_D0_PORT               GPIOB
#define      ST7796_D0_PIN                GPIO_Pin_14

#define      ST7796_D1_CLK                RCC_APB2Periph_GPIOB   
#define      ST7796_D1_PORT               GPIOB
#define      ST7796_D1_PIN                GPIO_Pin_15

#define      ST7796_D2_CLK                RCC_APB2Periph_GPIOA
#define      ST7796_D2_PORT               GPIOA
#define      ST7796_D2_PIN                GPIO_Pin_0

#define      ST7796_D3_CLK                RCC_APB2Periph_GPIOA
#define      ST7796_D3_PORT               GPIOA
#define      ST7796_D3_PIN                GPIO_Pin_1

#define      ST7796_D4_CLK                RCC_APB2Periph_GPIOA   
#define      ST7796_D4_PORT               GPIOA
#define      ST7796_D4_PIN                GPIO_Pin_2

#define      ST7796_D5_CLK                RCC_APB2Periph_GPIOA
#define      ST7796_D5_PORT               GPIOA
#define      ST7796_D5_PIN                GPIO_Pin_3

#define      ST7796_D6_CLK                RCC_APB2Periph_GPIOA
#define      ST7796_D6_PORT               GPIOA
#define      ST7796_D6_PIN                GPIO_Pin_4

#define      ST7796_D7_CLK                RCC_APB2Periph_GPIOA
#define      ST7796_D7_PORT               GPIOA
#define      ST7796_D7_PIN                GPIO_Pin_5

/*************************************** 调试预用 ******************************************/
#define      DEBUG_DELAY()                

/***************************** ST7796 显示区域的起始坐标和总行列数 ***************************/
#define      		ST7796_DispWindow_X_Star		    				0     //起始点的X坐标
#define      		ST7796_DispWindow_Y_Star		    				0     //起始点的Y坐标

#define 			ST7796_LESS_PIXEL	  								320			//液晶屏较短方向的像素宽度
#define 			ST7796_MORE_PIXEL	 								480			//液晶屏较长方向的像素宽度

//根据液晶扫描方向而变化的XY像素宽度
//调用ST7796_GramScan函数设置方向时会自动更改
extern uint16_t LCD_X_LENGTH,LCD_Y_LENGTH; 

//液晶屏扫描模式
//参数可选值为0-7
extern uint8_t LCD_SCAN_MODE;

/******************************* 定义 ST7796 显示屏常用颜色 ********************************/
#define      BACKGROUND		                BLACK   //默认背景颜色

#define      WHITE		 		           0xFFFF	   //白色
#define      BLACK                         0x0000	   //黑色 
#define      GREY                          0xF7DE	   //灰色 
#define      BLUE                          0x001F	   //蓝色 
#define      BLUE2                         0x051F	   //浅蓝色 
#define      RED                           0xF800	   //红色 
#define      MAGENTA                       0xF81F	   //红紫色，洋红色 
#define      GREEN                         0x07E0	   //绿色 
#define      CYAN                          0x7FFF	   //蓝绿色，青色 
#define      YELLOW                        0xFFE0	   //黄色 
#define      BRED                          0xF81F
#define      GRED                          0xFFE0
#define      GBLUE                         0x07FF



/******************************* 定义 ST7796 常用命令 ********************************/
#define      CMD_SetCoordinateX		 		    0x2A	     //设置X坐标
#define      CMD_SetCoordinateY		 		    0x2B	     //设置Y坐标
#define      CMD_SetPixel		 		        0x2C	     //填充像素

/********************************** 声明 ST7796 函数 ***************************************/
void ST7796_FillColor ( uint32_t ulAmout_Point, uint16_t usColor );
void                     ST7796_Init                    ( void );
void                     ST7796_Rst                     ( void );
void                     ST7796_BackLed_Control         ( FunctionalState enumState );
void                     ST7796_GramScan                ( uint8_t ucOtion );
void                     ST7796_OpenWindow              ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );
void                     ST7796_Clear                   ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );
void                     ST7796_SetPointPixel           ( uint16_t usX, uint16_t usY );
uint16_t                 ST7796_GetPointPixel           ( uint16_t usX , uint16_t usY );
void                     ST7796_DrawLine                ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2 );
void                     ST7796_DrawRectangle           ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight,uint8_t ucFilled );
void                     ST7796_DrawCircle              ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled );
void                     ST7796_DispChar_EN             ( uint16_t usX, uint16_t usY, const char cChar );
void                     ST7796_DispStringLine_EN      ( uint16_t line, char * pStr );
void                     ST7796_DispString_EN      			( uint16_t usX, uint16_t usY, char * pStr );
void 											ST7796_DispString_EN_YDir 		(   uint16_t usX,uint16_t usY ,  char * pStr );

void 											LCD_SetFont											(sFONT *fonts);
sFONT 										*LCD_GetFont											(void);
void 											LCD_ClearLine										(uint16_t Line);
void 											LCD_SetBackColor								(uint16_t Color);
void 											LCD_SetTextColor								(uint16_t Color)	;
void 											LCD_SetColors										(uint16_t TextColor, uint16_t BackColor);
void 											LCD_GetColors										(uint16_t *TextColor, uint16_t *BackColor);


#endif /* __BSP_ST7796_ST7796_H */


