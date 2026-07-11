#include "stdio.h"
#include "includes.h"
#include "uip_init.h"
//AT32F413
#include "at32f4xx.h"
#include "at32_board_uart.h"
#include "spi_def.h"
#include "etherbridge.h"
#include "atcommand.h"
#include "DM9051.h"
#include "httpd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
// Default Network Configuration
/*uint8_t ip[4] = {192, 168, 7, 51};
uint8_t ns[4] = {255, 255, 255, 0};
uint8_t gw[4] = {192, 168, 7, 1};*/

/*delay variable*/
static __IO float fac_us;
static __IO float fac_ms;	  
#define STEP_DELAY_MS	500 
void Delay_init(void)
{
  /*Config Systick*/
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
  fac_us=(float)SystemCoreClock/(8 * 1000000);
  fac_ms=fac_us*1000;
}
/**
  * @brief  Inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval None
  */
void Delay_us(u32 nus)
{
  u32 temp;
  SysTick->LOAD = (u32)(nus*fac_us);
  SysTick->VAL = 0x00;
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;
  do
  {
    temp = SysTick->CTRL;
  }while((temp & 0x01) &&! (temp & (1<<16)));

  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  SysTick->VAL = 0X00;
}
	    								  
/**
  * @brief  Inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay_ms(u16 nms)
{
  u32 temp;
  while(nms)
  {
    if(nms > STEP_DELAY_MS)
    {
      SysTick->LOAD = (u32)(STEP_DELAY_MS * fac_ms);
      nms -= STEP_DELAY_MS;
    }
    else
    {
      SysTick->LOAD = (u32)(nms * fac_ms);
      nms = 0;
    }
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
      temp = SysTick->CTRL;
    }while( (temp & 0x01) && !(temp & (1<<16)) );

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  sec: specifies the delay time length, in seconds.
  * @retval None
  */
void Delay_sec(u16 sec)
{
  u16 i;
  for(i=0; i<sec; i++)
  {
    Delay_ms(500);
    Delay_ms(500);
  }
}

void RCC_Configuration(void)
{
  /* Enable peripheral clocks --------------------------------------------------*/
  /* GPIOA, GPIOB and SPI1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA | RCC_APB2PERIPH_GPIOB | RCC_APB2PERIPH_GPIOF |
                         RCC_APB2PERIPH_SPI1, ENABLE);

  /* SPI2 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_SPI2, ENABLE);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);
}

void NVIC_Configuration(void)
{
	NVIC_InitType NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits*/  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	/* Set the Vector Table base location at 0x08000000 */ 
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		
	/* Enable the ATCMD_USART_IRQn Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = AT32_CMD_USART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
#if 1	
#ifdef ATCMD_USART_RX_DMA	
	/*Enable DMA Channel6 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = AT32_CMD_USART_RXDMA_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
#endif //ATCMD_USART_RX_DMA
#endif

#ifdef DMA_INT
/* Enable Rx DMA1 channel2 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif	
}	


void AT32_Base_Init(void)
{
  /* System clocks configuration ---------------------------------------------*/
  /* GPIO configuration ------------------------------------------------------*/
  //int temp;
	RCC_Configuration();
	UART_Print_Init(115200);  /* USART1 configured */
	UART_CMD_CLKPin(); //UART_CMD_Init(115200); /* AT COMD configured */
	DMA_Init_AT();
	Delay_init();	
	DM9051_SPI_Configuration(); 
	NVIC_Configuration();
	//temp = DMA_GetCurrDataCounter(AT32_CMD_USART_DMA_RX_Channel);
}

/**
  * @brief  Configures the ADC.
  * @param  None
  * @retval None
  */
void ADC_Configuration_AT407(void)
{
	#if 1 //[JJComment][at32f4xx_adc.c]
  ADC_InitType ADC_InitStructure;
  
  /* Enable ADC1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_ADC1, ENABLE);

  /* ADC1 Configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrig = ADC_ExternalTrig_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NumOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239_5);

  /* Enable ADC1 */
  ADC_Ctrl(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
	ADC_RstCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCtrl(ADC1, ENABLE);
	#endif
}


/* Private functions ---------------------------------------------------------*/
int main(void)
{	
	uint8_t mflag;
	
	/* System initial */
	AT32_Base_Init();
	
	mflag = Read_AT_DataFlash();
	if(mflag == 2){ // DataFlash is empty
	  atcmd_restore();
	}
	
	print_resp_boot();
	
#ifdef CUS_TEST
	if (!AT_Pass_Custom_Mode())
	  while(1) ;
#endif //CUS_TEST

	/* Show MCU and Ethernet Message & uIP App */
	_printf("*** %s ***\r\n", IPSPP_VERSION);

	UART_CMD_Init_Update(at_type.baudrate, at_type.wordlen ,at_type.parity, at_type.stop); /* AT COMD configured */
	atcmd_resp_boot();	
	atcmd_version();
	arp_need_new(); //init
	dns_tag_new();
	
	tcpip_init();
					
#ifdef HTTP_SERVER_SUPPORT
	httpd_init();
#endif //HTTP_SERVER_SUPPORT
	while(1){			
		/* uart to eth bridge function */
		bridge_init();
		//_printf(" bridge_init() end \r\n");
		
		/* TCP/IP TX/RX process*/
		tcpip_process();
		//_printf("tcpip_process() end\r\n");
		
		/* receive at uart data */
		at_cmdProcess();
		//_printf("at_cmdProcess() end\r\n");
	}
}

