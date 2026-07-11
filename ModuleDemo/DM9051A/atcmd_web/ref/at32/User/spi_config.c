#include "stdio.h"
//#include "nbsdk.h" 
//#include "bsp.h"
//#include "project/board.h"

//#include "project/lwip_dm_includes.h"
//#include "lwip_dm_includes.h"

//#include "stm32f10x.h"
#include "at32f4xx.h"	 
#include "DM9051.h"

//[AT32 redefine]
#define GPIO_Pin_2	GPIO_Pins_2
#define GPIO_Pin_4	GPIO_Pins_4
#define GPIO_Pin_5	GPIO_Pins_5
#define GPIO_Pin_6	GPIO_Pins_6
#define GPIO_Pin_7	GPIO_Pins_7
#define GPIO_Pin_10	GPIO_Pins_10

#define	PROC_SPI_NONE		0
#define PROC_SPI_YES		1
//[#define PROC_SPI_MAIN		1]
//[#define	PROC_SPI_DM9051	2]

//[spi interface gpio]
static void SPI_Interface_GPIO_Configuration(void)
{
  GPIO_InitType GPIO_InitStructure;

  /* Configure SPI1 pins: SCK, MISO and MOSI ---------------------------------*/
  /* Confugure SCK and MOSI pins as Alternate Function Push Pull */
  GPIO_InitStructure.GPIO_Pins = GPIO_Pins_5 | GPIO_Pins_6 | GPIO_Pins_7;
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz; //GPIO_MaxSpeed_2MHz; //GPIO_MaxSpeed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure SPI2 pins: SCK, MISO and MOSI ---------------------------------*/
  /* Confugure SCK and MOSI pins as Input Floating */
	#if 0
  GPIO_InitStructure.GPIO_Pins = GPIO_Pins_13 | GPIO_Pins_15;
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz; //GPIO_MaxSpeed_2MHz; //GPIO_MaxSpeed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pins = GPIO_Pins_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	#endif
}

//[cs]
/*void SPI_cs_gpio_Config(void)
{
  GPIO_InitType GPIO_InitStructure;
 //GPIO_InitStructure.GPIO_Pins = GPIO_Pin_DM9051_CS;
 GPIO_InitStructure.GPIO_Pins = GPIO_Pins_9; // | GPIO_Pins_11
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
 GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz;
 GPIO_Init(GPIOA, &GPIO_InitStructure); //GPIOA	
	#if 0
 GPIO_InitStructure.GPIO_Pins = GPIO_Pins_12; // | GPIO_Pins_11
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
 GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz;
 GPIO_Init(GPIOB, &GPIO_InitStructure); //GPIOB	
	#endif
}*/

//[cs]
static void SPI_cs_gpio_Config(void)
{
  GPIO_InitType GPIO_InitStructure;
 //GPIO_InitStructure.GPIO_Pins = GPIO_Pin_DM9051_CS;
 //GPIO_InitStructure.GPIO_Pins = GPIO_Pins_12 /*GPIO_Pin_DM9051_CS*/; // | GPIO_Pins_11
 GPIO_InitStructure.GPIO_Pins = GPIO_Pins_4;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
 GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz; //GPIO_MaxSpeed_2MHz; //GPIO_MaxSpeed_10MHz;
 //GPIO_Init(GPIOB /*GPIO_DM9051_CS*/, &GPIO_InitStructure); //GPIOB	
 GPIO_Init(GPIOA /*GPIO_DM9051_CS*/, &GPIO_InitStructure); //GPIOA		
}

//[SPI_Config]
static void SPI_interface_pin_Config(void)
{	
  /* SPI1 configuration ------------------------------------------------------*/
	SPI_InitType  SPI_InitStructure;
#if 1
//[SPI CRC]
  SPI_DefaultInitParaConfig(&SPI_InitStructure);  
	
  SPI_InitStructure.SPI_TransMode = SPI_TRANSMODE_FULLDUPLEX;
  SPI_InitStructure.SPI_Mode = SPI_MODE_MASTER;
	//#if 1
  SPI_InitStructure.SPI_FrameSize = SPI_FRAMESIZE_8BIT;
	//#else
  //SPI_InitStructure.SPI_FrameSize = SPI_FRAMESIZE_16BIT;
	//#endif
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1EDGE /*SPI_CPHA_2EDGE*/;
  SPI_InitStructure.SPI_NSSSEL = SPI_NSSSEL_SOFT;
  SPI_InitStructure.SPI_MCLKP = /*SPI_MCLKP_8;*/ /*SPI_MCLKP_4*/ SPI_MCLKP_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB;
  SPI_InitStructure.SPI_CPOLY = 7;
  SPI_Init(SPI1, &SPI_InitStructure); //SPI_InitStructure.SPI_Mode = SPI_MODE_MASTER (SPI_MODE_SLAVE;)
  /* SPI2 configuration ------------------------------------------------------*/
#endif

  /* Enable SPI1 CRC calculation */
  //SPI_CRCEN(SPI1, ENABLE);
  /* Enable SPI2 CRC calculation */

  /* Enable SPI1 */
  SPI_Enable(SPI1, ENABLE);
  /* Enable SPI2 */
}

static void SPI_RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA | RCC_APB2PERIPH_GPIOB | RCC_APB2PERIPH_GPIOF |
                         RCC_APB2PERIPH_SPI1, ENABLE);
  /* SPI2 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_SPI2, ENABLE);
}
	
/*   SPI Initialization  */
void DM9051_SPI_Configuration(void)
{
	static char Process_SPI_CONFIG = PROC_SPI_NONE;
  /* SPI Config.s -------------------------------------------------------------*/
	//SPI_InitType   SPI_InitStructure;
  /* SPI Config.e -------------------------------------------------------------*/
  /* GPIO of SPI Config.s -------------------------------------------------------------*/
  //GPIO_InitType GPIO_InitStructure;
  /* GPIO of SPI Config.e -------------------------------------------------------------*/
	
	if (Process_SPI_CONFIG == PROC_SPI_YES)
	{
//		printf("............._SPI_Configuration had been done (already).............\r\n");
		return;
	}
	if (Process_SPI_CONFIG == PROC_SPI_NONE)
	{
//		printf("............._SPI_Configuration had been done (firstly).............\r\n");
		Process_SPI_CONFIG = PROC_SPI_YES;
	}
	
	//[RCC]
	/* RCC Cfg.s -------------------------------------------------------------*/
	/*RCC_APB2PeriphClockCmd()
  * @param  RCC_APB2Periph: specifies the APB2 peripheral to gates its clock.
  *   This parameter can be any combination of the following values:
  *     @arg RCC_APB2PERIPH_AFIO,   'RCC_APB2PERIPH_GPIOA',   'RCC_APB2PERIPH_GPIOB',
  *          RCC_APB2PERIPH_GPIOC,  RCC_APB2PERIPH_GPIOD,   RCC_APB2PERIPH_GPIOE,
  *          RCC_APB2PERIPH_GPIOF,  RCC_APB2PERIPH_GPIOG,   RCC_APB2PERIPH_ADC1,
  *          RCC_APB2PERIPH_ADC2,   RCC_APB2PERIPH_TMR1,    RCC_APB2PERIPH_SPI1,
  *          RCC_APB2PERIPH_TMR8,   'RCC_APB2PERIPH_USART1',  RCC_APB2PERIPH_ADC3,
  *          RCC_APB2PERIPH_TMR15,  RCC_APB2PERIPH_TMR9,    RCC_APB2PERIPH_TMR10,
  *          RCC_APB2PERIPH_TMR11 */
	/*RCC_APB1PeriphClockCmd() -- NOT USED HERE!
	* @param  RCC_APB1Periph: specifies the APB1 peripheral to gates its clock.
  *   This parameter can be any combination of the following values:
  *     @arg RCC_APB1PERIPH_TMR2,   RCC_APB1PERIPH_TMR3,   RCC_APB1PERIPH_TMR4,
  *          RCC_APB1PERIPH_TMR5,   RCC_APB1PERIPH_TMR6,   RCC_APB1PERIPH_TMR7,
  *          RCC_APB1PERIPH_WWDG,   RCC_APB1PERIPH_SPI2,   RCC_APB1PERIPH_SPI3,
  *          RCC_APB1PERIPH_SPI4,   RCC_APB1PERIPH_USART2, RCC_APB1PERIPH_USART3,
  *          RCC_APB1Periph_USART4, RCC_APB1Periph_USART5, RCC_APB1PERIPH_I2C1,
  *          RCC_APB1PERIPH_I2C2,   RCC_APB1PERIPH_I2C3,   RCC_APB1PERIPH_USB,
  *          RCC_APB1PERIPH_CAN1,   RCC_APB1PERIPH_BKP,    RCC_APB1PERIPH_PWR,
  *          RCC_APB1PERIPH_DAC,    RCC_APB1PERIPH_TMR12,  RCC_APB1PERIPH_TMR13,
  *          RCC_APB1PERIPH_TMR14*/
	//[RCC_Configuration()]
#if 1	
	SPI_RCC_Configuration();
	/*RCC_APB2PeriphClockCmd(SPIz_GPIO_CLK, ENABLE); // Enable GPIO clock for SPIz 
  RCC_APB1PeriphClockCmd(SPIz_CLK, ENABLE); // Enable SPIz Periph clock 
	*/
#endif	
	/* RCC Cfg.e -------------------------------------------------------------*/
	
	//[SPI Mode Master][10 MHz]
  /* GPIO of SPIz Config.s -------------------------------------------------------------*/
	/* Configure SPIy pins: SCK, MISO and MOSI ---------------------------------*/
	//[SPI_Interface_GPIO_Configuration]
#if 1
  SPI_Interface_GPIO_Configuration();
	/*GPIO_InitStructure.GPIO_Pins = SPIz_PIN_SCK | SPIz_PIN_MOSI;
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz; //GPIO_MaxSpeed_50MHz; //GPIO_MaxSpeed_2MHz; //GPIO_MaxSpeed_10MHz; // 10 MHz................
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPIz_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pins = SPIz_PIN_MISO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPIz_GPIO, &GPIO_InitStructure);*/
#endif

	SPI_cs_gpio_Config();	
	/*GPIO_InitStructure.GPIO_Pins = GPIO_Pin_DM9051_CS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz;
	GPIO_Init(GPIO_DM9051_CS, &GPIO_InitStructure); //GPIOB
	*/
  /* GPIO of SPIz Config.e -------------------------------------------------------------*/
	
	//[SPI2]
  /* SPI2 Config.s -------------------------------------------------------------*/
	//[SPI_interface_pin_Config]
#if 1
	SPI_interface_pin_Config();
  /*SPI_DefaultInitParaConfig(&SPI_InitStructure);
  //--------------- Reset SPI init structure parameters values -----------------
  SPI_InitStructure.SPI_TransMode = SPI_TRANSMODE_FULLDUPLEX;
  SPI_InitStructure.SPI_Mode = SPI_MODE_MASTER; //SPI_MODE_SLAVE; //SPI_MODE_MASTER;
  SPI_InitStructure.SPI_FrameSize = SPI_FRAMESIZE_8BIT;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1EDGE; //SPI_CPHA_2EDGE;
  SPI_InitStructure.SPI_NSSSEL = SPI_NSSSEL_SOFT;
  SPI_InitStructure.SPI_MCLKP = //SPI_MCLKP_2; //SPI_MCLKP_4; //SPI_MCLKP_8;
																SPI_MCLKP_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB; //SPI_FIRSTBIT_LSB;
  SPI_InitStructure.SPI_CPOLY = 7;
  SPI_Init(SPIz, &SPI_InitStructure);
	
	//[SPI Enable]
	//.SPI_CRCEN(SPI2, ENABLE); // Enable SPI2 CRC calculation
	SPI_Enable(SPIz, ENABLE); //== SPI_Cmd(SPIz, ENABLE)
	*/
#endif	
  /* SPI2 Config.e -------------------------------------------------------------*/

	/* SPI1 configuration */
	/*
	//SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	//SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	//SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	//SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	//SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	//SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	*/
	/* Enable SPI1  */
	//SPI_Cmd(SPI1, ENABLE);
}
