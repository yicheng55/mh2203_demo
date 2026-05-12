#include "bsp_ST7796_lcd.h"
#include "fonts.h"	
#include "delay.h"

//根据液晶扫描方向而变化的XY像素宽度
//调用ST7796_GramScan函数设置方向时会自动更改
uint16_t LCD_X_LENGTH = ST7796_LESS_PIXEL;
uint16_t LCD_Y_LENGTH = ST7796_MORE_PIXEL;

//液晶屏扫描模式，本变量主要用于方便选择触摸屏的计算参数
//参数可选值为0-7
//调用ST7796_GramScan函数设置方向时会自动更改
//LCD刚初始化完成时会使用本默认值
uint8_t LCD_SCAN_MODE = 0;


static sFONT *LCD_Currentfonts = &Font8x16;  //英文字体
static uint16_t CurrentTextColor   = BLACK;//前景色
static uint16_t CurrentBackColor   = WHITE;//背景色

void                 ST7796_Write_Cmd           ( uint8_t usCmd );
void                 ST7796_Write_Data          ( uint8_t usData );
uint8_t              ST7796_Read_Data           ( void );
static void                   ST7796_Delay               ( __IO uint32_t nCount );
static void                   ST7796_GPIO_Config         ( void );
static void                   ST7796_FSMC_Config         ( void );
static void                   ST7796_REG_Config          ( void );
static void                   ST7796_SetCursor           ( uint16_t usX, uint16_t usY );
static uint16_t               ST7796_Read_PixelData      ( void );




/**
  * @brief  向ST7796写入命令
  * @param  usCmd :要写入的命令（表寄存器地址）
  * @retval 无
  */	
void ST7796_Write_Cmd ( uint8_t usCmd )
{
	* ( __IO uint8_t * ) ( FSMC_Addr_ST7796_CMD ) = usCmd;
}


/**
  * @brief  向ST7796写入数据
  * @param  usData :要写入的数据
  * @retval 无
  */	
void ST7796_Write_Data ( uint8_t usData )
{
	* ( __IO uint8_t * ) ( FSMC_Addr_ST7796_DATA ) = usData;
}


/**
  * @brief  从ST7796读取数据
  * @param  无
  * @retval 读取到的数据
  */	
__inline uint8_t ST7796_Read_Data ( void )
{
	return ( * ( __IO uint8_t * ) ( FSMC_Addr_ST7796_DATA ) );
}


/**
  * @brief  用于 ST7796 简单延时函数
  * @param  nCount ：延时计数值
  * @retval 无
  */	
static void ST7796_Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
}


/**
  * @brief  初始化ST7796的IO引脚
  * @param  无
  * @retval 无
  */
static void ST7796_GPIO_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能FSMC对应相应管脚时钟*/
	RCC_APB2PeriphClockCmd ( 	
													/*控制信号*/
													ST7796_CS_CLK|ST7796_DC_CLK|ST7796_WR_CLK|
													ST7796_RD_CLK	|ST7796_BK_CLK|ST7796_RST_CLK|
													/*数据信号*/
													ST7796_D0_CLK|ST7796_D1_CLK|	ST7796_D2_CLK | 
													ST7796_D3_CLK | ST7796_D4_CLK|ST7796_D5_CLK|
													ST7796_D6_CLK | ST7796_D7_CLK, ENABLE );
		
	
	/* 配置FSMC相对应的数据线,FSMC-D0~D7 */	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D0_PIN;
	GPIO_Init ( ST7796_D0_PORT, & GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = ST7796_D1_PIN;
	GPIO_Init ( ST7796_D1_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D2_PIN;
	GPIO_Init ( ST7796_D2_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D3_PIN;
	GPIO_Init ( ST7796_D3_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D4_PIN;
	GPIO_Init ( ST7796_D4_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D5_PIN;
	GPIO_Init ( ST7796_D5_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D6_PIN;
	GPIO_Init ( ST7796_D6_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_D7_PIN;
	GPIO_Init ( ST7796_D7_PORT, & GPIO_InitStructure );
	
	/* 配置FSMC相对应的控制线
	 * FSMC_NOE   :LCD-RD
	 * FSMC_NWE   :LCD-WR
	 * FSMC_NE1   :LCD-CS
	 * FSMC_A16  	:LCD-DC
	 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;
	
	GPIO_InitStructure.GPIO_Pin = ST7796_RD_PIN; 
	GPIO_Init (ST7796_RD_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_WR_PIN; 
	GPIO_Init (ST7796_WR_PORT, & GPIO_InitStructure );
	
	GPIO_InitStructure.GPIO_Pin = ST7796_CS_PIN; 
	GPIO_Init ( ST7796_CS_PORT, & GPIO_InitStructure );  
	
	GPIO_InitStructure.GPIO_Pin = ST7796_DC_PIN; 
	GPIO_Init ( ST7796_DC_PORT, & GPIO_InitStructure );
	

  /* 配置LCD复位RST控制管脚*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = ST7796_RST_PIN; 
	GPIO_Init ( ST7796_RST_PORT, & GPIO_InitStructure );
	
	
	/* 配置LCD背光控制管脚BK*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	
	GPIO_InitStructure.GPIO_Pin = ST7796_BK_PIN; 
	GPIO_Init ( ST7796_BK_PORT, & GPIO_InitStructure );
	GPIO_SetBits(ST7796_BK_PORT,ST7796_BK_PIN);
}


 /**
  * @brief  LCD  FSMC 模式配置
  * @param  无
  * @retval 无
  */
static void ST7796_FSMC_Config ( void )
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 	
	
	/* 使能FSMC时钟*/
	RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_FSMC, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_LCD_AF3,ENABLE);

	//地址建立时间（ADDSET）为1个HCLK 2/72M=28ns
	readWriteTiming.FSMC_AddressSetupTime      = 0x01;	 //地址建立时间
	//数据保持时间（DATAST）+ 1个HCLK = 5/72M=70ns	
	readWriteTiming.FSMC_DataSetupTime         = 0x03;	 //数据建立时间
	
	//选择控制的模式
	//模式B,异步NOR FLASH模式，与ST7796的8080时序匹配
	readWriteTiming.FSMC_AccessMode            = FSMC_AccessMode_B;	
	
	/*以下配置与模式B无关*/
	//地址保持时间（ADDHLD）模式A未用到
	readWriteTiming.FSMC_AddressHoldTime       = 0x00;	 //地址保持时间
	//设置总线转换周期，仅用于复用模式的NOR操作
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	//设置时钟分频，仅用于同步类型的存储器
	readWriteTiming.FSMC_CLKDivision           = 0x00;
	//数据保持时间，仅用于同步型的NOR	
	readWriteTiming.FSMC_DataLatency           = 0x00;	

	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_Bank                  = FSMC_Bank1_NORSRAMx;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType            = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_8b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode              = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &readWriteTiming;
	
	FSMC_NORSRAMInit ( & FSMC_NORSRAMInitStructure ); 

	/* 使能 FSMC_Bank1_NORSRAM4 */
	FSMC_NORSRAMCmd ( FSMC_Bank1_NORSRAMx, ENABLE );  	
}


////*	名称：void LCD_Display_Dir(u8 dir)
////*	功能：设置LCD显示方向
////*	输入：dir:0,竖屏；1,横屏
////*	返回：无
//void LCD_Display_Dir(u8 dir)
//{
//	if(dir==0)			
//	{
//		lcddev.dir=0;
//	
//		lcddev.wramcmd=0X2C;
//		lcddev.setxcmd=0X2A;
//		lcddev.setycmd=0X2B;  	 
//		
//		lcddev.width=320;
//		lcddev.height=480;
//	}
//	else
//	{	  				
//		lcddev.dir=1;
//		lcddev.wramcmd=0X2C;
//		lcddev.setxcmd=0X2A;
//		lcddev.setycmd=0X2B;  	 
//		lcddev.width=480;
//		lcddev.height=320; 			
//	} 
//	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
//}	 

/**
 * @brief  初始化ST7796寄存器
 * @param  无
 * @retval 无
 */
static void ST7796_REG_Config ( void )
{
	ST7796_Write_Cmd(0X11);
	Delay_Ms(120);

	ST7796_Write_Cmd(0Xf0);
	ST7796_Write_Data(0xc3);

	ST7796_Write_Cmd(0Xf0);
	ST7796_Write_Data(0x96);

	ST7796_Write_Cmd(0X36);
	ST7796_Write_Data(0x48);

	ST7796_Write_Cmd(0X3a);
	ST7796_Write_Data(0x05);

	ST7796_Write_Cmd(0XB4);
	ST7796_Write_Data(0x01);

	ST7796_Write_Cmd(0Xe6);
	ST7796_Write_Data(0x0f);
	ST7796_Write_Data(0xf2);
	ST7796_Write_Data(0x3f);
	ST7796_Write_Data(0x4f);
	ST7796_Write_Data(0x4f);
	ST7796_Write_Data(0x28);
	ST7796_Write_Data(0x0e);
	ST7796_Write_Data(0x00);

	ST7796_Write_Cmd(0Xc5);
	ST7796_Write_Data(0x20);

	ST7796_Write_Cmd(0Xe0);
	ST7796_Write_Data(0xf0);
	ST7796_Write_Data(0x03);
	ST7796_Write_Data(0x0a);
	ST7796_Write_Data(0x11);
	ST7796_Write_Data(0x14);
	ST7796_Write_Data(0x1c);
	ST7796_Write_Data(0x3b);
	ST7796_Write_Data(0x55);
	ST7796_Write_Data(0x4a);
	ST7796_Write_Data(0x0a);
	ST7796_Write_Data(0x13);
	ST7796_Write_Data(0x14);
	ST7796_Write_Data(0x1c);
	ST7796_Write_Data(0x1f);

	ST7796_Write_Cmd(0Xe1);
	ST7796_Write_Data(0xf0);
	ST7796_Write_Data(0x03);
	ST7796_Write_Data(0x0a);
	ST7796_Write_Data(0x0c);
	ST7796_Write_Data(0x0c);
	ST7796_Write_Data(0x09);
	ST7796_Write_Data(0x36);
	ST7796_Write_Data(0x54);
	ST7796_Write_Data(0x49);
	ST7796_Write_Data(0x0f);
	ST7796_Write_Data(0x1b);
	ST7796_Write_Data(0x18);
	ST7796_Write_Data(0x1b);
	ST7796_Write_Data(0x1f);

	ST7796_Write_Cmd(0Xf0);
	ST7796_Write_Data(0x3c);

	ST7796_Write_Cmd(0Xf0);
	ST7796_Write_Data(0x69);
	Delay_Ms(120);
	ST7796_Write_Cmd(0X29);
}


/**
 * @brief  ST7796初始化函数，如果要用到lcd，一定要调用这个函数
 * @param  无
 * @retval 无
 */
void ST7796_Init ( void )
{
	  SPI_Configuration();
		BK_H;
		ST7796_Rst();
		ST7796_REG_Config();
		ILI9488_GramScan(0);
}


/**
 * @brief  ST7796背光LED控制
 * @param  enumState ：决定是否使能背光LED
  *   该参数为以下值之一：
  *     @arg ENABLE :使能背光LED
  *     @arg DISABLE :禁用背光LED
 * @retval 无
 */
void ST7796_BackLed_Control ( FunctionalState enumState )
{
	if ( enumState )
		GPIO_ResetBits ( ST7796_BK_PORT, ST7796_BK_PIN );	
	else
		GPIO_SetBits ( ST7796_BK_PORT, ST7796_BK_PIN );
		
}



/**
 * @brief  ST7796 软件复位
 * @param  无
 * @retval 无
 */
void ST7796_Rst ( void )
{			
	GPIO_SetBits ( ST7796_RST_PORT, ST7796_RST_PIN );	
	Delay_Ms(10);
	GPIO_ResetBits ( ST7796_RST_PORT, ST7796_RST_PIN );	 //低电平复位
	Delay_Ms(20);
	GPIO_SetBits ( ST7796_RST_PORT, ST7796_RST_PIN );		 	 
	Delay_Ms(20);
}




/**
 * @brief  设置ST7796的GRAM的扫描方向 
 * @param  ucOption ：选择GRAM的扫描方向 
 *     @arg 0-7 :参数可选值为0-7这八个方向
 *
 *	！！！其中0、3、5、6 模式适合从左至右显示文字，
 *				不推荐使用其它模式显示文字	其它模式显示文字会有镜像效果			
 *		
 *	其中0、2、4、6 模式的X方向像素为240，Y方向像素为320
 *	其中1、3、5、7 模式下X方向像素为320，Y方向像素为240
 *
 *	其中 6 模式为大部分液晶例程的默认显示方向
 *	其中 3 模式为摄像头例程使用的方向
 *	其中 0 模式为BMP图片显示例程使用的方向
 *
 * @retval 无
 * @note  坐标图例：A表示向上，V表示向下，<表示向左，>表示向右
					X表示X轴，Y表示Y轴

------------------------------------------------------------
模式0：				.		模式1：		.	模式2：			.	模式3：					
					A		.					A		.		A					.		A									
					|		.					|		.		|					.		|							
					Y		.					X		.		Y					.		X					
					0		.					1		.		2					.		3					
	<--- X0 o		.	<----Y1	o		.		o 2X--->  .		o 3Y--->	
------------------------------------------------------------	
模式4：				.	模式5：			.	模式6：			.	模式7：					
	<--- X4 o		.	<--- Y5 o		.		o 6X--->  .		o 7Y--->	
					4		.					5		.		6					.		7	
					Y		.					X		.		Y					.		X						
					|		.					|		.		|					.		|							
					V		.					V		.		V					.		V		
---------------------------------------------------------				
											 LCD屏示例
								|-----------------|
								|			Logo		|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|-----------------|

 *******************************************************/
 u8 DFT_SCAN_DIR; //扫描方向变量
_lcd_dev lcddev;

//设置LCD的自动扫描方向
//dir:0~7,代表8个方向(具体定义见lcd.h)   
void LCD_Scan_Dir(u8 dir)
{
	u16 regval=0;
	u8 dirreg=0;
//	u16 temp;  
	switch(dir)//方向转换
	{
		case 0:dir=6;break;
		case 1:dir=7;break;
		case 2:dir=4;break;
		case 3:dir=5;break;
		case 4:dir=1;break;
		case 5:dir=0;break;
		case 6:dir=3;break;
		case 7:dir=2;break;	     
	}
	switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
			regval|=(0<<7)|(0<<6)|(0<<5); 
			break;
		case L2R_D2U://从左到右,从下到上
			regval|=(1<<7)|(0<<6)|(0<<5); 
			break;
		case R2L_U2D://从右到左,从上到下
			regval|=(0<<7)|(1<<6)|(0<<5); 
			break;
		case R2L_D2U://从右到左,从下到上
			regval|=(1<<7)|(1<<6)|(0<<5); 
			break;	 
		case U2D_L2R://从上到下,从左到右
			regval|=(0<<7)|(0<<6)|(1<<5); 
			break;
		case U2D_R2L://从上到下,从右到左
			regval|=(0<<7)|(1<<6)|(1<<5); 
			break;
		case D2U_L2R://从下到上,从左到右
			regval|=(1<<7)|(0<<6)|(1<<5); 
			break;
		case D2U_R2L://从下到上,从右到左
			regval|=(1<<7)|(1<<6)|(1<<5); 
			break;	 
	}
	dirreg=0X36; 
  regval|=0x08;	//0x08 0x00  红蓝反色可以通过这里修改
		ST7796_Write_Cmd(dirreg);
	ST7796_Write_Data(regval);
	
	ST7796_Write_Cmd(lcddev.setxcmd); 
	ST7796_Write_Data(0);ST7796_Write_Data(0);
	ST7796_Write_Data((lcddev.width-1)>>8);ST7796_Write_Data((lcddev.width-1)&0XFF);
	ST7796_Write_Cmd(lcddev.setycmd); 
	ST7796_Write_Data(0);ST7796_Write_Data(0);
	ST7796_Write_Data((lcddev.height-1)>>8);ST7796_Write_Data((lcddev.height-1)&0XFF);  

}      


void ST7796_GramScan ( uint8_t ucOption )
{	
	if(ucOption==0)			 
	{
		lcddev.dir=0;	//竖屏
		lcddev.width=LCD_WIDTH;
		lcddev.height=LCD_HEIGHT;

		lcddev.wramcmd=0X2C;
		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  
    DFT_SCAN_DIR=D2U_R2L;	    //竖显-设定显示方向	

	}else 				  //横屏
	{	  				
		lcddev.dir=1;	 
		lcddev.width=LCD_HEIGHT;
		lcddev.height=LCD_WIDTH;

		lcddev.wramcmd=0X2C;
		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  
    DFT_SCAN_DIR=L2R_D2U;     //横显-设定显示方向		
		
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}


/**
 * @brief  在ST7796显示器上开辟一个窗口
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @retval 无
 */
void ST7796_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{	
	ST7796_Write_Cmd ( CMD_SetCoordinateX ); 				 /* 设置X坐标 */
	ST7796_Write_Data ( usX >> 8  & 0xff);	 /* 先高8位，然后低8位 */
	ST7796_Write_Data ( usX & 0xff  );	 /* 设置起始点和结束点*/
	ST7796_Write_Data ( ( usX + usWidth - 1) >> 8  & 0xff);
	ST7796_Write_Data ( ( usX + usWidth - 1) & 0xff);

	ST7796_Write_Cmd ( CMD_SetCoordinateY ); 			     /* 设置Y坐标*/
	ST7796_Write_Data ( usY >> 8  & 0xff);
	ST7796_Write_Data ( usY & 0xff  );
	ST7796_Write_Data ( ( usY + usHeight - 1) >> 8 & 0xff);
	ST7796_Write_Data ( ( usY + usHeight - 1) & 0xff);
}


/**
 * @brief  设定ST7796的光标坐标
 * @param  usX ：在特定扫描方向下光标的X坐标
 * @param  usY ：在特定扫描方向下光标的Y坐标
 * @retval 无
 */
static void ST7796_SetCursor ( uint16_t usX, uint16_t usY )	
{
	ST7796_OpenWindow ( usX, usY, 1, 1 );
}


/**
 * @brief  在ST7796显示器上以某一颜色填充像素点
 * @param  ulAmout_Point ：要填充颜色的像素点的总数目
 * @param  usColor ：颜色
 * @retval 无
 */
void ST7796_FillColor ( uint32_t ulAmout_Point, uint16_t usColor )
{
	uint32_t i = 0;
	/* memory write */
	ST7796_Write_Cmd ( CMD_SetPixel );	
		
	for ( i = 0; i < ulAmout_Point; i ++ )
	{
		ST7796_Write_Data ( usColor >> 8 & 0xFF);
		ST7796_Write_Data ( usColor & 0xFF);
	}
}


/**
 * @brief  对ST7796显示器的某一窗口以某种颜色进行清屏
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_Clear ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{
	ST7796_OpenWindow ( usX, usY, usWidth, usHeight );

	ST7796_FillColor ( usWidth * usHeight, CurrentBackColor );		
	
}


/**
 * @brief  对ST7796显示器的某一点以某种颜色进行填充
 * @param  usX ：在特定扫描方向下该点的X坐标
 * @param  usY ：在特定扫描方向下该点的Y坐标
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_SetPointPixel ( uint16_t usX, uint16_t usY )	
{	
	if ( ( usX < LCD_X_LENGTH ) && ( usY < LCD_Y_LENGTH ) )
  {
		ST7796_SetCursor ( usX, usY );
		
		ST7796_FillColor ( 1, CurrentTextColor );
	}
	
}


/**
 * @brief  读取ST7796 GRAN 的一个像素数据
 * @param  无
 * @retval 像素数据
 */
static uint16_t ST7796_Read_PixelData ( void )	
{	
	uint16_t usR=0, usG=0, usB=0 ;

	
	ST7796_Write_Cmd ( 0x2E );   /* 读数据 */
	
	usR = ST7796_Read_Data (); 	/*FIRST READ OUT DUMMY DATA*/
	
	usR = ST7796_Read_Data ();  	/*READ OUT RED DATA  */
	usB = ST7796_Read_Data ();  	/*READ OUT BLUE DATA*/
	usG = ST7796_Read_Data ();  	/*READ OUT GREEN DATA*/	
	
  return ( ( ( usR >> 11 ) << 11 ) | ( ( usG >> 10 ) << 5 ) | ( usB >> 11 ) );
	
}


/**
 * @brief  获取 ST7796 显示器上某一个坐标点的像素数据
 * @param  usX ：在特定扫描方向下该点的X坐标
 * @param  usY ：在特定扫描方向下该点的Y坐标
 * @retval 像素数据
 */
uint16_t ST7796_GetPointPixel ( uint16_t usX, uint16_t usY )
{ 
	uint16_t usPixelData;

	
	ST7796_SetCursor ( usX, usY );
	
	usPixelData = ST7796_Read_PixelData ();
	
	return usPixelData;
	
}


/**
 * @brief  在 ST7796 显示器上使用 Bresenham 算法画线段 
 * @param  usX1 ：在特定扫描方向下线段的一个端点X坐标
 * @param  usY1 ：在特定扫描方向下线段的一个端点Y坐标
 * @param  usX2 ：在特定扫描方向下线段的另一个端点X坐标
 * @param  usY2 ：在特定扫描方向下线段的另一个端点Y坐标
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DrawLine ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2 )
{
	uint16_t us; 
	uint16_t usX_Current, usY_Current;
	
	int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int32_t lIncrease_X, lIncrease_Y; 	
	
	
	lDelta_X = usX2 - usX1; //计算坐标增量 
	lDelta_Y = usY2 - usY1; 
	
	usX_Current = usX1; 
	usY_Current = usY1; 
	
	
	if ( lDelta_X > 0 ) 
		lIncrease_X = 1; //设置单步方向 
	
	else if ( lDelta_X == 0 ) 
		lIncrease_X = 0;//垂直线 
	
	else 
  { 
    lIncrease_X = -1;
    lDelta_X = - lDelta_X;
  } 

	
	if ( lDelta_Y > 0 )
		lIncrease_Y = 1; 
	
	else if ( lDelta_Y == 0 )
		lIncrease_Y = 0;//水平线 
	
	else 
  {
    lIncrease_Y = -1;
    lDelta_Y = - lDelta_Y;
  } 

	
	if (  lDelta_X > lDelta_Y )
		lDistance = lDelta_X; //选取基本增量坐标轴 
	
	else 
		lDistance = lDelta_Y; 

	
	for ( us = 0; us <= lDistance + 1; us ++ )//画线输出 
	{  
		ST7796_SetPointPixel ( usX_Current, usY_Current );//画点 
		
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 
		
		if ( lError_X > lDistance ) 
		{ 
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  
		
		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 
		
	}  
	
	
}   


/**
 * @brief  在 ST7796 显示器上画一个矩形
 * @param  usX_Start ：在特定扫描方向下矩形的起始点X坐标
 * @param  usY_Start ：在特定扫描方向下矩形的起始点Y坐标
 * @param  usWidth：矩形的宽度（单位：像素）
 * @param  usHeight：矩形的高度（单位：像素）
 * @param  ucFilled ：选择是否填充该矩形
  *   该参数为以下值之一：
  *     @arg 0 :空心矩形
  *     @arg 1 :实心矩形 
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DrawRectangle ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled )
{
	if ( ucFilled )
	{
		ST7796_OpenWindow ( usX_Start, usY_Start, usWidth, usHeight );
		ST7796_FillColor ( usWidth * usHeight ,CurrentTextColor);	
	}
	else
	{
		ST7796_DrawLine ( usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start );
		ST7796_DrawLine ( usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1 );
		ST7796_DrawLine ( usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1 );
		ST7796_DrawLine ( usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1 );		
	}

}


/**
 * @brief  在 ST7796 显示器上使用 Bresenham 算法画圆
 * @param  usX_Center ：在特定扫描方向下圆心的X坐标
 * @param  usY_Center ：在特定扫描方向下圆心的Y坐标
 * @param  usRadius：圆的半径（单位：像素）
 * @param  ucFilled ：选择是否填充该圆
  *   该参数为以下值之一：
  *     @arg 0 :空心圆
  *     @arg 1 :实心圆
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DrawCircle ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled )
{
	int16_t sCurrentX, sCurrentY;
	int16_t sError;
	
	
	sCurrentX = 0; sCurrentY = usRadius;	  
	
	sError = 3 - ( usRadius << 1 );     //判断下个点位置的标志
	
	
	while ( sCurrentX <= sCurrentY )
	{
		int16_t sCountY;
		
		
		if ( ucFilled ) 			
			for ( sCountY = sCurrentX; sCountY <= sCurrentY; sCountY ++ ) 
			{                      
				ST7796_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCountY );           //1，研究对象 
				ST7796_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCountY );           //2       
				ST7796_SetPointPixel ( usX_Center - sCountY,   usY_Center + sCurrentX );           //3
				ST7796_SetPointPixel ( usX_Center - sCountY,   usY_Center - sCurrentX );           //4
				ST7796_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCountY );           //5    
        ST7796_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCountY );           //6
				ST7796_SetPointPixel ( usX_Center + sCountY,   usY_Center - sCurrentX );           //7 	
        ST7796_SetPointPixel ( usX_Center + sCountY,   usY_Center + sCurrentX );           //0				
			}
		
		else
		{          
			ST7796_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCurrentY );             //1，研究对象
			ST7796_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCurrentY );             //2      
			ST7796_SetPointPixel ( usX_Center - sCurrentY, usY_Center + sCurrentX );             //3
			ST7796_SetPointPixel ( usX_Center - sCurrentY, usY_Center - sCurrentX );             //4
			ST7796_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCurrentY );             //5       
			ST7796_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCurrentY );             //6
			ST7796_SetPointPixel ( usX_Center + sCurrentY, usY_Center - sCurrentX );             //7 
			ST7796_SetPointPixel ( usX_Center + sCurrentY, usY_Center + sCurrentX );             //0
    }			
		
		
		sCurrentX ++;

		
		if ( sError < 0 ) 
			sError += 4 * sCurrentX + 6;	  
		
		else
		{
			sError += 10 + 4 * ( sCurrentX - sCurrentY );   
			sCurrentY --;
		} 	
		
		
	}
	
	
}

/**
 * @brief  在 ST7796 显示器上显示一个英文字符
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下该点的起始Y坐标
 * @param  cChar ：要显示的英文字符
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DispChar_EN ( uint16_t usX, uint16_t usY, const char cChar )
{
	uint8_t  byteCount, bitCount,fontLength;	
	uint16_t ucRelativePositon;
	uint8_t *Pfont;
	
	//对ascii码表偏移（字模表不包含ASCII表的前32个非图形符号）
	ucRelativePositon = cChar - ' ';
	
	//每个字模的字节数
	fontLength = (LCD_Currentfonts->Width*LCD_Currentfonts->Height)/8;
		
	//字模首地址
	/*ascii码表偏移值乘以每个字模的字节数，求出字模的偏移位置*/
	Pfont = (uint8_t *)&LCD_Currentfonts->table[ucRelativePositon * fontLength];
	
	//设置显示窗口
	ST7796_OpenWindow ( usX, usY, LCD_Currentfonts->Width, LCD_Currentfonts->Height);
	
	ST7796_Write_Cmd ( CMD_SetPixel );			

	//按字节读取字模数据
	//由于前面直接设置了显示窗口，显示数据会自动换行
	for ( byteCount = 0; byteCount < fontLength; byteCount++ )
	{
		//一位一位处理要显示的颜色
		for ( bitCount = 0; bitCount < 8; bitCount++ )
		{
				if ( Pfont[byteCount] & (0x80>>bitCount) )
				{
					ST7796_Write_Data ( CurrentTextColor >>8 & 0xFF);	
					ST7796_Write_Data ( CurrentTextColor & 0xFF);	
				}						
				else
				{
					ST7796_Write_Data ( CurrentBackColor >>8 & 0xFF);	
					ST7796_Write_Data ( CurrentBackColor & 0xFF);
				}
		}
	}	
}


/**
 * @brief  在 ST7796 显示器上显示英文字符串
 * @param  line ：在特定扫描方向下字符串的起始Y坐标
  *   本参数可使用宏LINE(0)、LINE(1)等方式指定文字坐标，
  *   宏LINE(x)会根据当前选择的字体来计算Y坐标值。
	*		显示中文且使用LINE宏时，需要把英文字体设置成Font8x16
 * @param  pStr ：要显示的英文字符串的首地址
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DispStringLine_EN (  uint16_t line,  char * pStr )
{
	uint16_t usX = 0;
	
	while ( * pStr != '\0' )
	{
		if ( ( usX - ST7796_DispWindow_X_Star + LCD_Currentfonts->Width ) > LCD_X_LENGTH )
		{
			usX = ST7796_DispWindow_X_Star;
			line += LCD_Currentfonts->Height;
		}
		
		if ( ( line - ST7796_DispWindow_Y_Star + LCD_Currentfonts->Height ) > LCD_Y_LENGTH )
		{
			usX = ST7796_DispWindow_X_Star;
			line = ST7796_DispWindow_Y_Star;
		}
		
		ST7796_DispChar_EN ( usX, line, * pStr);
		
		pStr ++;
		
		usX += LCD_Currentfonts->Width;
		
	}
	
}


/**
 * @brief  在 ST7796 显示器上显示英文字符串
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下字符的起始Y坐标
 * @param  pStr ：要显示的英文字符串的首地址
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DispString_EN ( 	uint16_t usX ,uint16_t usY,  char * pStr )
{
	while ( * pStr != '\0' )
	{
		if ( ( usX - ST7796_DispWindow_X_Star + LCD_Currentfonts->Width ) > LCD_X_LENGTH )
		{
			usX = ST7796_DispWindow_X_Star;
			usY += LCD_Currentfonts->Height;
		}
		
		if ( ( usY - ST7796_DispWindow_Y_Star + LCD_Currentfonts->Height ) > LCD_Y_LENGTH )
		{
			usX = ST7796_DispWindow_X_Star;
			usY = ST7796_DispWindow_Y_Star;
		}
		
		ST7796_DispChar_EN ( usX, usY, * pStr);
		
		pStr ++;
		
		usX += LCD_Currentfonts->Width;
		
	}
	
}


/**
 * @brief  在 ST7796 显示器上显示英文字符串(沿Y轴方向)
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下字符的起始Y坐标
 * @param  pStr ：要显示的英文字符串的首地址
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ST7796_DispString_EN_YDir (	 uint16_t usX,uint16_t usY ,  char * pStr )
{	
	while ( * pStr != '\0' )
	{
		if ( ( usY - ST7796_DispWindow_Y_Star + LCD_Currentfonts->Height ) >LCD_Y_LENGTH  )
		{
			usY = ST7796_DispWindow_Y_Star;
			usX += LCD_Currentfonts->Width;
		}
		
		if ( ( usX - ST7796_DispWindow_X_Star + LCD_Currentfonts->Width ) >  LCD_X_LENGTH)
		{
			usX = ST7796_DispWindow_X_Star;
			usY = ST7796_DispWindow_Y_Star;
		}
		
		ST7796_DispChar_EN ( usX, usY, * pStr);
		
		pStr ++;
		
		usY += LCD_Currentfonts->Height;		
	}	
}


/**
  * @brief  设置英文字体类型
  * @param  fonts: 指定要选择的字体
	*		参数为以下值之一
  * 	@arg：Font24x32;
  * 	@arg：Font16x24;
  * 	@arg：Font8x16;
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
  * @brief  获取当前字体类型
  * @param  None.
  * @retval 返回当前字体类型
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}


/**
  * @brief  设置LCD的前景(字体)及背景颜色,RGB565
  * @param  TextColor: 指定前景(字体)颜色
  * @param  BackColor: 指定背景颜色
  * @retval None
  */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor) 
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief  获取LCD的前景(字体)及背景颜色,RGB565
  * @param  TextColor: 用来存储前景(字体)颜色的指针变量
  * @param  BackColor: 用来存储背景颜色的指针变量
  * @retval None
  */
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  设置LCD的前景(字体)颜色,RGB565
  * @param  Color: 指定前景(字体)颜色 
  * @retval None
  */
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  设置LCD的背景颜色,RGB565
  * @param  Color: 指定背景颜色 
  * @retval None
  */
void LCD_SetBackColor(uint16_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  清除某行文字
  * @param  Line: 指定要删除的行
  *   本参数可使用宏LINE(0)、LINE(1)等方式指定要删除的行，
  *   宏LINE(x)会根据当前选择的字体来计算Y坐标值，并删除当前字体高度的第x行。
  * @retval None
  */
void LCD_ClearLine(uint16_t Line)
{
  ST7796_Clear(0,Line,LCD_X_LENGTH,((sFONT *)LCD_GetFont())->Height);	/* 清屏，显示全黑 */

}
/*********************end of file*************************/



