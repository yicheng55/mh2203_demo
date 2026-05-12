#ifndef      __BSP_ST7796_LCD_H
#define	     __BSP_ST7796_LCD_H

#include "mh22xx.h"
#include "fonts.h"


//0,不支持ucos
//1,支持ucos
#define SYSTEM_SUPPORT_OS		0		//定义系统文件夹是否支持UCOS
																	    
	 
//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

//以下为汇编函数
void WFI_SET(void);		//执行WFI指令
void INTX_DISABLE(void);//关闭所有中断
void INTX_ENABLE(void);	//开启所有中断
void MSR_MSP(u32 addr);	//设置堆栈地址

//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	wramcmd;		//开始写gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令 
}_lcd_dev; 	  


//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色	   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------LCD端口定义---------------- 
#define	LCD_LED PBout(0) //LCD背光  PB0 	    
//LCD地址结构体
typedef struct
{
	vu16 LCD_REG;
	vu16 LCD_RAM;
} LCD_TypeDef;
//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A10作为数据命令区分线 
//注意设置时STM32内部会右移一位对其! 			    
#define LCD_BASE        ((u32)(0x6C000000 | 0x000007FE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 



//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
	    															  
void LCD_Init(void);													   	//初始化
void LCD_DisplayOn(void);													//开显示
void LCD_DisplayOff(void);													//关显示
void LCD_Clear(u16 Color);	 												//清屏
void LCD_SetCursor(u16 Xpos, u16 Ypos);										//设置光标
void LCD_DrawPoint(u16 x,u16 y);											//画点
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color);								//快速画点
u16  LCD_ReadPoint(u16 x,u16 y); 											//读点 
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r);						 			//画圆
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);							//画线
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   				//画矩形
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);		   				//填充单色
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);				//填充指定颜色
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);						//显示一个字符
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);  						//显示一个数字
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode);				//显示 数字
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);		//显示一个字符串,12/16字体

void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);
u16 LCD_ReadReg(u16 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
void LCD_SSD_BackLightSet(u8 pwm);							//SSD1963 背光控制
void LCD_Scan_Dir(u8 dir);									//设置屏扫描方向
void LCD_Display_Dir(u8 dir);								//设置屏幕显示方向
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height);	//设置窗口					   						   																			 
//LCD分辨率设置
#define SSD_HOR_RESOLUTION		800		//LCD水平分辨率
#define SSD_VER_RESOLUTION		480		//LCD垂直分辨率
//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直前廊
//如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
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
#define      ST7796_RST_PIN               GPIO_Pin_14

//背光引脚
#define      ST7796_BK_CLK                RCC_APB2Periph_GPIOC  //PB1
#define      ST7796_BK_PORT               GPIOC
#define      ST7796_BK_PIN                GPIO_Pin_15

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
void 					 ST7796_DispString_EN_YDir 		(   uint16_t usX,uint16_t usY ,  char * pStr );
uint16_t ST7796_ReadID(uint8_t cmd);
uint16_t ST7796_ReadPoint(u16 x,u16 y);
void ST7796_FillColor ( uint32_t ulAmout_Point, uint16_t usColor );
void ST7796_Fast_DrawPoint(u16 x,u16 y,u16 color);
void ST7796_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
void 											LCD_SetFont											(sFONT *fonts);
sFONT 										*LCD_GetFont											(void);
void 											LCD_ClearLine										(uint16_t Line);
void 											LCD_SetBackColor								(uint16_t Color);
void 											LCD_SetTextColor								(uint16_t Color)	;
void 											LCD_SetColors										(uint16_t TextColor, uint16_t BackColor);
void 											LCD_GetColors										(uint16_t *TextColor, uint16_t *BackColor);


#endif /* __BSP_ST7796_ST7796_H */


