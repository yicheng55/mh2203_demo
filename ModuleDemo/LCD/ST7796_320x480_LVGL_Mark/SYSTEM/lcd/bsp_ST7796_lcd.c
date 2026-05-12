#include "bsp_ST7796_lcd.h"
#include "delay.h"


//参数可选值为0-7
//调用ST7796_GramScan函数设置方向时会自动更改
//LCD刚初始化完成时会使用本默认值
uint8_t LCD_SCAN_MODE = 0;

static void                   ST7796_GPIO_Config         ( void );
static void                   ST7796_FSMC_Config         ( void );
static void                   ST7796_REG_Config          ( void );

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
	ST7796_GPIO_Config ();
	ST7796_FSMC_Config ();
	
	ST7796_Rst ();
	ST7796_REG_Config ();
	
	//设置默认扫描方向，其中 6 模式为大部分液晶例程的默认显示方向  
	ST7796_GramScan(LCD_SCAN_MODE);
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




void ST7796_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usX1, uint16_t usY1 )
{	
	ST7796_Write_Cmd ( CMD_SetCoordinateX ); 				 /* 设置X坐标 */
	ST7796_Write_Data ( usX >> 8  & 0xff);	 /* 先高8位，然后低8位 */
	ST7796_Write_Data ( usX & 0xff  );	 /* 设置起始点和结束点*/
	ST7796_Write_Data ( usX1>> 8  & 0xff);
	ST7796_Write_Data ( usX1 & 0xff);

	ST7796_Write_Cmd ( CMD_SetCoordinateY ); 			     /* 设置Y坐标*/
	ST7796_Write_Data ( usY >> 8  & 0xff);
	ST7796_Write_Data ( usY & 0xff  );
	ST7796_Write_Data ( usY1 >> 8 & 0xff);
	ST7796_Write_Data ( usY1 & 0xff);
}









/*********************end of file*************************/



