#include "Spi_Base.h"
#include "delay.h"

extern _lcd_dev lcddev;
u8 DFT_SCAN_DIR; //扫描方向变量

SPI_TypeDef* SPI_TEST = SPI2;
uint16_t Send_Data[2048];

static sFONT *LCD_Currentfonts = &Font8x16;  //英文字体
static uint16_t CurrentTextColor   = BLACK;//前景色
static uint16_t CurrentBackColor   = WHITE;//背景色

uint16_t LCD_X_LENGTH = ST7796_LESS_PIXEL;
uint16_t LCD_Y_LENGTH = ST7796_MORE_PIXEL;

//根据液晶扫描方向而变化的XY像素宽度
//调用ST7796_GramScan函数设置方向时会自动更改

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	
void SPI_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15; //14-RST  15-BK
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_11;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    BK_H
    RST_H
    CS_H
    DC_L
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14| GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大1小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;		//定义波特率预分频的值:波特率预分频值为256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
    SPI_Init(SPI_TEST, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI_TEST, ENABLE); //使能SPI外设

}
static void ST7796_SetCursor ( uint16_t usX, uint16_t usY )	
{
	ST7796_OpenWindow ( usX, usY, 1, 1 );
}
void ST7796_Fast_DrawPoint(u16 x,u16 y,u16 color)
{	 
	ST7796_SetCursor ( x, y );
	ST7796_FillColor ( 1, color );
}

void ST7796_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{   
	ST7796_OpenWindow (sx,sy,ex,ey);
	ST7796_FillColor( (ex-sx)*(ey-sy) ,color);
}
uint16_t ST7796_ReadPoint(u16 x,u16 y)
{
	return 0;
}
/*************SPI配置函数*******************
SSD1306，SCL空闲时低电平，第一个上升沿采样
模拟SPI
******************************************/

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
  * @brief  向ST7796写入命令
  * @param  usCmd :要写入的命令（表寄存器地址）
  * @retval 无
  */
void ST7796_Write_Cmd ( uint8_t usCmd )
{
    DC_L
    CS_L
    SPI_I2S_SendData(SPI_TEST,usCmd);
    while(SPI_I2S_GetFlagStatus(SPI_TEST,SPI_I2S_FLAG_TXE)==0);
    while(SPI_I2S_GetFlagStatus(SPI_TEST,SPI_I2S_FLAG_RXNE)==0);
    SPI_TEST->DR;
    CS_H
}
/**
  * @brief  向ST7796写入数据
  * @param  usData :要写入的数据
  * @retval 无
  */
void ST7796_Write_Data ( uint8_t usCmd )
{
    DC_H
    CS_L
    SPI_I2S_SendData(SPI_TEST,usCmd);
    while(SPI_I2S_GetFlagStatus(SPI_TEST,SPI_I2S_FLAG_TXE)==0);
    while(SPI_I2S_GetFlagStatus(SPI_TEST,SPI_I2S_FLAG_RXNE)==0);
    SPI_TEST->DR;
    CS_H
}


void ILI9488_GramScan ( uint8_t ucOption )
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

    } else 				  //横屏
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
 * @brief  在ST7796显示器上以某一颜色填充像素点
 * @param  ulAmout_Point ：要填充颜色的像素点的总数目
 * @param  usColor ：颜色18BIT RGB666
 * @retval 无
 */
void ST7796_FillColor ( uint32_t ulAmout_Point, uint16_t usColor )
{
    uint32_t i = 0;
    /* memory write */
    ST7796_Write_Cmd ( 0x2C );

    for ( i = 0; i < ulAmout_Point; i ++ )
    {
        ST7796_Write_Data((usColor>>8)&0x00ff);
        ST7796_Write_Data(usColor&0x00ff);
    }
}
/**
 * @brief  初始化ST7796寄存器
 * @param  无
 * @retval 无
 */
void ST7796_REG_Config ( void )
{
    ST7796_Write_Cmd(0x11); 			//Sleep Out
    Delay_Ms(120);               //DELAY120ms
    //--------------------------------ST7796 Frame rate setting----------------------------------//
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


void LCD_Scan_Dir(u8 dir)
{
    u16 regval=0;
    u8 dirreg=0;
//	u16 temp;
    switch(dir)//方向转换
    {
    case 0:
        dir=6;
        break;
    case 1:
        dir=7;
        break;
    case 2:
        dir=4;
        break;
    case 3:
        dir=5;
        break;
    case 4:
        dir=1;
        break;
    case 5:
        dir=0;
        break;
    case 6:
        dir=3;
        break;
    case 7:
        dir=2;
        break;
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
    regval|=COLORORDER;	//0x08 0x00  红蓝反色可以通过这里修改
    ST7796_Write_Cmd(dirreg);
    ST7796_Write_Data(regval);

    ST7796_Write_Cmd(lcddev.setxcmd);
    ST7796_Write_Data(0);
    ST7796_Write_Data(0);
    ST7796_Write_Data((lcddev.width-1)>>8);
    ST7796_Write_Data((lcddev.width-1)&0XFF);
    ST7796_Write_Cmd(lcddev.setycmd);
    ST7796_Write_Data(0);
    ST7796_Write_Data(0);
    ST7796_Write_Data((lcddev.height-1)>>8);
    ST7796_Write_Data((lcddev.height-1)&0XFF);


}
//设置LCD显示方向（6804不支持横屏显示）
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
    if(dir==0)
    {
        lcddev.dir=0;	//竖屏
        lcddev.width=LCD_WIDTH;
        lcddev.height=LCD_HEIGHT;

        lcddev.wramcmd=0X2C;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;
        DFT_SCAN_DIR=L2R_U2D;	    //竖显-设定显示方向

    } else 				  //横屏
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

//设置窗口
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{
    width=sx+width-1;
    height=sy+height-1;

    ST7796_Write_Cmd(lcddev.setxcmd);
    ST7796_Write_Data(sx>>8);
    ST7796_Write_Data(sx&0XFF);
    ST7796_Write_Data(width>>8);
    ST7796_Write_Data(width&0XFF);
    ST7796_Write_Cmd(lcddev.setycmd);
    ST7796_Write_Data(sy>>8);
    ST7796_Write_Data(sy&0XFF);
    ST7796_Write_Data(height>>8);
    ST7796_Write_Data(height&0XFF);
}

//清屏函数
//color:要清屏的填充色
void LCD_Clear(u16 color)
{
    u32 index=0;
    u32 totalpoint=lcddev.width;
    totalpoint*=lcddev.height; 	//得到总点数
    LCD_Set_Window(0,0,lcddev.width,lcddev.height);
    ST7796_Write_Cmd(lcddev.wramcmd);
    DC_H
    CS_L
    for(index=0; index<totalpoint; index++)
    {
        ST7796_Write_Data(color>>8);
        ST7796_Write_Data(color);
    }
    CS_H

}
/**
 * @brief  ST7796初始化函数，如果要用到lcd，一定要调用这个函数
 * @param  无
 * @retval 无
 */
void ST7796_Init(void)
{
    SPI_Configuration();
    BK_H;
    ST7796_Rst();
    ST7796_REG_Config();
    ILI9488_GramScan(0);
}
/**
 * @brief  ST7796 软件复位
 * @param  无
 * @retval 无
 */
void ST7796_Rst (void)
{
    RST_H;
    Delay_Ms(10);
    RST_L;
    Delay_Ms(20);
    RST_H;
    Delay_Ms(20);
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
    ST7796_Write_Cmd ( 0x2a ); 				 /* 设置X坐标 */
    ST7796_Write_Data ( usX >> 8  & 0xff);	 /* 先高8位，然后低8位 */
    ST7796_Write_Data ( usX & 0xff  );	 /* 设置起始点和结束点*/
    ST7796_Write_Data ( ( usX + usWidth - 1) >> 8  & 0xff);
    ST7796_Write_Data ( ( usX + usWidth - 1) & 0xff);

    ST7796_Write_Cmd ( 0x2b ); 			     /* 设置Y坐标*/
    ST7796_Write_Data ( usY >> 8  & 0xff);
    ST7796_Write_Data ( usY & 0xff  );
    ST7796_Write_Data ( ( usY + usHeight - 1) >> 8 & 0xff);
    ST7796_Write_Data ( ( usY + usHeight - 1) & 0xff);
}



void Dma_Send_Data(uint32_t data,uint32_t Data_Len)
{
    DMA_Cmd(DMA1_Channel5, DISABLE);
    SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);
    DMA1_Channel5->CMAR=(uint32_t)data;
    DMA1_Channel5->CPAR=(uint32_t)&(SPI_TEST->DR);
    DMA1_Channel5->CNDTR=Data_Len;
    CS_L
    DC_H
    DMA_Cmd(DMA1_Channel5, ENABLE);

    while (DMA_GetFlagStatus(DMA1_FLAG_TC5) == RESET);//等待转运完成
    while (SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE) == RESET);//等待转运完成
    SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,DISABLE);
    CS_H
    DMA_ClearFlag(DMA1_FLAG_TC5);//清除标志位
}
void Dma_Init(void)
{

    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;//外设站点的基地址
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//数据宽度
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//是否自增
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;//存储器站点的基地址
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//传输方向
    DMA_InitStructure.DMA_BufferSize = 0;//传输次数
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//是否自动重装
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//软件触发
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//转运优先级
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);



}

/*********************end of file*************************/
