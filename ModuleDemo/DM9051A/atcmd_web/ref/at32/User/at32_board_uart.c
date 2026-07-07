
#include "stdio.h"
#include "includes.h"
#include "at32_board_uart.h"
#include "uip.h"
#include "uip_init.h"
#include "DM9051.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "uip_keepalive.h"

extern struct eeprom_funcation eeprom_show;

volatile uint8_t StringLoop;
uint32_t ideal_len = 0;
uint32_t ideal_len_old = 0;

#if JOSRANDOM
#if 0
  void init_at32_random(void) {
    // Example: Enable RNG clock and initialize (if using RNG)
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    RNG->CR |= RNG_CR_RNGEN;
  }
#endif
#endif

#if JOSRANDOM
uint32_t generate_random_number(void) {
//    uint32_t random_number = 0;

//    // TODO: Implement RNG initialization and number generation
//    // Example:
//    // if ((RNG->SR & RNG_SR_DRDY) == RNG_SR_DRDY) {
//    //     random_number = RNG->DR;
//    // }

//    return random_number;
	
	return ((uint32_t)rand() + eeprom_show.ee_lastport);
}
#endif

/**
  * @brief  initialize UART1   
  * @param  bound: UART BaudRate
  * @retval None
  */
void UART_Print_Init(uint32_t bound)
{
  GPIO_InitType GPIO_InitStructure;
  USART_InitType USART_InitStructure;

  /*Enable the UART Clock*/
#if defined (AT32F421xx)
  RCC_AHBPeriphClockCmd(AT32_PRINT_UARTTX_GPIO_RCC | AT32_PRINT_UARTRX_GPIO_RCC, ENABLE);	
#else
  RCC_APB2PeriphClockCmd(AT32_PRINT_UARTTX_GPIO_RCC | AT32_PRINT_UARTRX_GPIO_RCC, ENABLE);	
#endif
  RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_USART1, ENABLE);
	//RCC_APB1PeriphClockCmd(AT32_PRINT_UART_RCC, ENABLE);

  /* Configure the UART1 TX pin */
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pins = AT32_PRINT_UARTTX_PIN; 
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_2MHz; //GPIO_MaxSpeed_50MHz;
#if !defined (AT32F421xx)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OutType = GPIO_OutType_PP;
  GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;
#endif
  GPIO_Init(AT32_PRINT_UARTTX_GPIO, &GPIO_InitStructure);

  /* Configure the UART1 RX pin */
  GPIO_InitStructure.GPIO_Pins = AT32_PRINT_UARTRX_PIN;//PA10
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_2MHz; //GPIO_MaxSpeed_50MHz;
#if !defined (AT32F421xx)
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_PD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;
#endif
  GPIO_Init(AT32_PRINT_UARTRX_GPIO, &GPIO_InitStructure);

#if defined (AT32F421xx)
  GPIO_PinAFConfig(GPIOA, GPIO_PinsSource9, GPIO_AF_1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinsSource10, GPIO_AF_1);
#endif

  /*Configure UART param*/
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate = bound;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

  USART_Init(AT32_PRINT_UART, &USART_InitStructure); 
  //USART_INTConfig(AT32_PRINT_UART, USART_INT_RDNE, ENABLE);
  USART_Cmd(AT32_PRINT_UART, ENABLE);   
}


void DMA_Init_AT(void)
{
	DMA_InitType  DMA_InitStructure;
	
#ifdef ATCMD_USART_RX_DMA		
	//Interrupt configure  
	//USART_ITConfig(AT32_CMD_USART, USART_IT_TC, DISABLE);  
	//USART_ITConfig(AT32_CMD_USART, USART_IT_RXNE, DISABLE);  
	//USART_ITConfig(AT32_CMD_USART, USART_IT_IDLE, ENABLE);
  USART_INTConfig(AT32_CMD_USART, USART_INT_TRAC, DISABLE);  
	USART_INTConfig(AT32_CMD_USART, USART_INT_RDNE, DISABLE);
	//USART_INTConfig(AT32_CMD_USART, USART_INT_RDNE, ENABLE);
  USART_INTConfig(AT32_CMD_USART, USART_INT_IDLEF, ENABLE);
	//USART_INTConfig(AT32_CMD_USART, USART_INT_IDLEF, DISABLE);
	
	//ATCMD_USART_Rx_DMA_Channel configuration ---------------------------------
//#ifdef UPDATE_USART2_RX_DMA
	//DMA_DeInit(ATCMD_USART_DMA_RX_Channel);
	//DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ATCMD_USART_DMA_RDR_BaseAddr;
	//DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_u8RecData;
	//DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Reset(AT32_CMD_USART_DMA_RX_Channel); //..
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404; //(uint32_t)ATCMD_USART_DMA_RDR_BaseAddr;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_u8RecData;
	DMA_InitStructure.DMA_Direction /*DMA_DIR*/ = DMA_DIR_PERIPHERALSRC; //DMA_DIR_PeripheralSRC;
#ifndef UIP_USE_BIGDATA_SIZE	
	DMA_InitStructure.DMA_BufferSize = RecData_Size;		//RX length
#else
	DMA_InitStructure.DMA_BufferSize = 1460;		//RX length	
#endif //UIP_USE_BIGDATA_SIZE
	DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE; //DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE; //DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataWidth /*DMA_PeripheralDataSize*/ = DMA_PERIPHERALDATAWIDTH_BYTE; //DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataWidth /*DMA_MemoryDataSize*/ = DMA_MEMORYDATAWIDTH_BYTE; //DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode =  DMA_MODE_NORMAL; //DMA_MODE_CIRCULAR;  
	DMA_InitStructure.DMA_Priority = DMA_PRIORITY_VERYHIGH; //DMA_Priority_High;
	DMA_InitStructure.DMA_MTOM /*DMA_M2M*/ = DMA_MEMTOMEM_DISABLE; //DMA_M2M_Disable;
	DMA_Init(AT32_CMD_USART_DMA_RX_Channel, &DMA_InitStructure);
//#endif //UPDATE_USART2_RX_DMA

	DMA_INTConfig(AT32_CMD_USART_DMA_RX_Channel, DMA_INT_TC/*DMA_IT_TC*/, ENABLE);	//EnableATCMD_USART_DMA_RX_Channel Interrupt
	USART_DMACmd(AT32_CMD_USART, USART_DMAReq_Rx /*USART_DMAReq_Rx*/, ENABLE);//Enable DMA RX 
	//DMA_Cmd(ATCMD_USART_DMA_RX_Channel, ENABLE);
	DMA_ChannelEnable(DMA1_Channel6, ENABLE);

#else
	USART_INTConfig(AT32_CMD_USART, USART_INT_RDNE, ENABLE);
	//USART_ITConfig(ATCMD_USART, USART_IT_TXE, ENABLE);
	//USART_ReceiverTimeOutCmd(ATCMD_USART,ENABLE);
	//USART_SetReceiverTimeOut(ATCMD_USART,80);
	//ATCMD_USART->CR3 = 0x00001000;
#endif //ATCMD_USART_RX_DMA

}	

void UART_CMD_CLKPin(void) {
  GPIO_InitType GPIO_InitStructure;
//  USART_InitType USART_InitStructure;

  /*Enable the UART Clock*/
#if defined (AT32F421xx)
  RCC_AHBPeriphClockCmd(AT32_CMD_UARTTX_GPIO_RCC | AT32_PRINT_UARTRX_GPIO_RCC, ENABLE);	
#else
  RCC_APB2PeriphClockCmd(AT32_CMD_UARTTX_GPIO_RCC | AT32_PRINT_UARTRX_GPIO_RCC, ENABLE);	
#endif
  //RCC_APB2PeriphClockCmd(AT32_CMD_UART_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(AT32_CMD_UART_RCC, ENABLE);

  /* Configure the UART1 TX pin */
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pins = AT32_CMD_UARTTX_PIN; 
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz; //GPIO_MaxSpeed_2MHz; 
#if !defined (AT32F421xx)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OutType = GPIO_OutType_PP;
  GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;
#endif
  GPIO_Init(AT32_CMD_UARTTX_GPIO, &GPIO_InitStructure);

  /* Configure the UART1 RX pin */
  GPIO_InitStructure.GPIO_Pins = AT32_CMD_UARTRX_PIN;//PA10
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz; //GPIO_MaxSpeed_2MHz; 
#if !defined (AT32F421xx)
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_PD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;
#endif
  GPIO_Init(AT32_CMD_UARTRX_GPIO, &GPIO_InitStructure);

#if defined (AT32F421xx)
  GPIO_PinAFConfig(GPIOA, GPIO_PinsSource9, GPIO_AF_1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinsSource10, GPIO_AF_1);
#endif
}

/**
  * @brief  initialize CMD UART   
  * @param  bound: UART BaudRate
  * @retval None
  */
#if 0
void UART_CMD_Init(uint32_t bound)
{
	UART_CMD_CLKPin();

#if 0
//  GPIO_InitType GPIO_InitStructure;
  USART_InitType USART_InitStructure;
//	DMA_InitType  DMA_InitStructure;
  /*Configure UART param*/
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate = bound;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(AT32_CMD_USART, &USART_InitStructure); 
  //USART_INTConfig(AT32_CMD_USART, USART_INT_RDNE, ENABLE);
  USART_Cmd(AT32_CMD_USART, ENABLE);
#endif

#if 0

//#ifdef ATCMD_USART_RX_DMA		
//	//Interrupt configure  
//	//USART_ITConfig(AT32_CMD_USART, USART_IT_TC, DISABLE);  
//	//USART_ITConfig(AT32_CMD_USART, USART_IT_RXNE, DISABLE);  
//	//USART_ITConfig(AT32_CMD_USART, USART_IT_IDLE, ENABLE);
//  USART_INTConfig(AT32_CMD_USART, USART_INT_TRAC, DISABLE);  
//	USART_INTConfig(AT32_CMD_USART, USART_INT_RDNE, DISABLE);
//  USART_INTConfig(AT32_CMD_USART, USART_INT_IDLEF, ENABLE);	
//	
//	//ATCMD_USART_Rx_DMA_Channel configuration ---------------------------------
//	//DMA_DeInit(ATCMD_USART_DMA_RX_Channel);
//	//DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ATCMD_USART_DMA_RDR_BaseAddr;
//	//DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_u8RecData;
//	//DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//	DMA_Reset(ATCMD_USART_DMA_RX_Channel);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404; //(uint32_t)ATCMD_USART_DMA_RDR_BaseAddr;
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_u8RecData;
//	DMA_InitStructure.DMA_Direction /*DMA_DIR*/ = DMA_DIR_PERIPHERALSRC; //DMA_DIR_PeripheralSRC;
//#ifndef UIP_USE_BIGDATA_SIZE	
//	DMA_InitStructure.DMA_BufferSize = RecData_Size;		//RX length
//#else
//	DMA_InitStructure.DMA_BufferSize = 1460;		//RX length	
//#endif //UIP_USE_BIGDATA_SIZE
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE; //DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE; //DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataWidth /*DMA_PeripheralDataSize*/ = DMA_PERIPHERALDATAWIDTH_BYTE; //DMA_PeripheralDataSize_Byte;
//	DMA_InitStructure.DMA_MemoryDataWidth /*DMA_MemoryDataSize*/ = DMA_MEMORYDATAWIDTH_BYTE; //DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_Mode = DMA_MODE_NORMAL; //DMA_Mode_Circular;
//	DMA_InitStructure.DMA_Priority = DMA_PRIORITY_VERYHIGH; //DMA_Priority_High;
//	DMA_InitStructure.DMA_MTOM /*DMA_M2M*/ = DMA_MEMTOMEM_DISABLE; //DMA_M2M_Disable;
//	DMA_Init(ATCMD_USART_DMA_RX_Channel, &DMA_InitStructure);

//	//DMA_INTConfig(ATCMD_USART_DMA_RX_Channel, DMA_INT_TC/*DMA_IT_TC*/, ENABLE);	//EnableATCMD_USART_DMA_RX_Channel Interrupt
//	USART_DMACmd(AT32_CMD_USART, USART_DMAReq_Rx /*USART_DMAReq_Rx*/, ENABLE);//Enable DMA RX 
//	//DMA_Cmd(ATCMD_USART_DMA_RX_Channel, ENABLE);
//	DMA_ChannelEnable(DMA1_Channel6, ENABLE);

//#else
//	USART_INTConfig(AT32_CMD_USART, USART_INT_RDNE, ENABLE);
//	//USART_ITConfig(ATCMD_USART, USART_IT_TXE, ENABLE);
//	//USART_ReceiverTimeOutCmd(ATCMD_USART,ENABLE);
//	//USART_SetReceiverTimeOut(ATCMD_USART,80);
//	//ATCMD_USART->CR3 = 0x00001000;
//#endif //ATCMD_USART_RX_DMA
#endif

 
}
#endif

#define USART_WORD_LENGTH_BIT(wordlen) (wordlen == 9) ? USART_WordLength_9b : USART_WordLength_8b
//#define USART_STOPBITS_BIT(stop) (stop == 3) ? USART_StopBits_2 : (stop == 2) ? USART_StopBits_1_5 : USART_StopBits_1
#define USART_PARITY_BIT(parity) (parity == 2) ? USART_Parity_Even : (parity == 1) ? USART_Parity_Odd : USART_Parity_No

#define SPRINT_PARITY_STR(parity) (parity == 2) ? "even" : (parity == 1) ? "odd" : "none"	 
//#define SPRINT_STOPBITS_BIT(stop) (stop == 3) ? "2bit(3)" : (stop == 2) ? "1.5bit(2)" : (stop == 1) ? "1bit(1)" : "?"

#define USART_STOPBITS_BIT(stop) (stop == 2) ? USART_StopBits_2 : USART_StopBits_1
#define SPRINT_STOPBITS_BIT(stop) (stop == 2) ? "2bit(2)" : (stop == 1) ? "1bit(1)" : "?"

#if 1
void UART_CMD_Init_Update_print(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
  uint16_t USART_WordLength, USART_Parity, USART_StopBits;

  USART_WordLength = USART_WORD_LENGTH_BIT(wordlen); //USART_WordLength_8b;/USART_WordLength_9b;
  
	if (parity != 0)
		USART_WordLength = USART_WORD_LENGTH_BIT(wordlen+1);
  
  USART_StopBits = USART_STOPBITS_BIT(stop); //USART_StopBits_1;
  USART_Parity = USART_PARITY_BIT(parity); //USART_Parity_No;

	printf(" at-cmd user..... %u %u %s(%u) %s\r\n", baud, wordlen, SPRINT_PARITY_STR(parity), parity,
			SPRINT_STOPBITS_BIT(stop));
	printf("   -mcu config... %u %u %s(%u) %s\r\n", baud, (parity != 0) ? wordlen + 1 : wordlen, SPRINT_PARITY_STR(parity), parity,
			SPRINT_STOPBITS_BIT(stop));
	printf("   -mcu cnf reg.. %04x %04x %04x\r\n",
			USART_WordLength,
			USART_Parity,
			USART_StopBits);
}
void UART_CMD_Init_Update_log(void) {
	uint8_t paramTB[10][3] = {
		{7,1,1,},
		{7,2,1,},
		{7,1,2,},
		{7,2,2,},
		{8,0,1,},
		{8,1,1,},
		{8,2,1,},
		{8,0,2,},
		{8,1,2,},
		{8,2,2,},
	};
	int i;
	printf("   -BAUD.. TESTING\r\n");
	for (i = 0; i< 10; i++)
		UART_CMD_Init_Update_print(115200, paramTB[i][0], paramTB[i][1], paramTB[i][2]);
}
#endif

void UART_CMD_Init_Update(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
//  GPIO_InitType GPIO_InitStructure;
  USART_InitType USART_InitStructure;
//	DMA_InitType  DMA_InitStructure;

	UART_CMD_CLKPin();

  /*Configure UART param*/
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate = baud; //baud
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

#if 0
  USART_InitStructure.USART_WordLength = USART_WORD_LENGTH_BIT(wordlen); //USART_WordLength_8b;/USART_WordLength_9b;
  USART_InitStructure.USART_StopBits = USART_STOPBITS_BIT(stop); //USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_PARITY_BIT(parity); //USART_Parity_No;
	printf(" at-cmd update.. %u %u %s(%u) %s\n", baud, wordlen, SPRINT_PARITY_STR(parity), parity,
			SPRINT_STOPBITS_BIT(stop));
	//[temp avoid crash!]
//	if (wordlen == 8 && parity == 1 && stop == 2) {
//	} else if (wordlen == 8 && parity == 2 && stop == 2) { }
	if (wordlen == 8 && (parity == 1 || parity == 2) && stop == 2) {
	  printf(" Cast.. parity %s to(%s)\n", SPRINT_PARITY_STR(parity), SPRINT_PARITY_STR(0));
	  printf(" Cast.. stopbt %s to(%s)\n", SPRINT_STOPBITS_BIT(stop), SPRINT_STOPBITS_BIT(1));
	  USART_InitStructure.USART_Parity = USART_PARITY_BIT(0);
	  USART_InitStructure.USART_StopBits = USART_STOPBITS_BIT(1);
	}
	else if (stop == 2) {
	  printf(" Cast.. stopbt %s to(%s)\n", SPRINT_STOPBITS_BIT(stop), SPRINT_STOPBITS_BIT(1));
	  USART_InitStructure.USART_StopBits = USART_STOPBITS_BIT(1);
	}
	else if (wordlen == 8 && (parity == 1 || parity == 2)) {
	  printf(" Cast.. parity %s to(%s)\n", SPRINT_PARITY_STR(parity), SPRINT_PARITY_STR(0));
	  USART_InitStructure.USART_Parity = USART_PARITY_BIT(0);
	}
//	else if (wordlen == 8 && parity == 1) {
//	} else if (wordlen == 8 && parity == 2) { } 
//	else {

//	  printf("  Be OK\n");
//	}
#else
  USART_InitStructure.USART_WordLength = USART_WORD_LENGTH_BIT(wordlen); //USART_WordLength_8b;/USART_WordLength_9b;
  
  if (parity != 0)
	USART_InitStructure.USART_WordLength = USART_WORD_LENGTH_BIT(wordlen+1);
  
  USART_InitStructure.USART_Parity = USART_PARITY_BIT(parity); //USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_STOPBITS_BIT(stop); //USART_StopBits_1;

	printf(" at-cmd user..... %u %u %s(%u) %s\r\n", baud, wordlen, SPRINT_PARITY_STR(parity), parity,
			SPRINT_STOPBITS_BIT(stop));
	printf("   -mcu config... %u %u %s(%u) %s\r\n", baud, (parity != 0) ? wordlen + 1 : wordlen, SPRINT_PARITY_STR(parity), parity,
			SPRINT_STOPBITS_BIT(stop));
	printf("   -mcu cnf reg.. %04x %04x %04x\r\n",
			USART_InitStructure.USART_WordLength,
			USART_InitStructure.USART_Parity,
			USART_InitStructure.USART_StopBits);
#endif

  USART_Init(AT32_CMD_USART, &USART_InitStructure);
  USART_Cmd(AT32_CMD_USART, ENABLE);
  
#if 0
//#ifdef ATCMD_USART_RX_DMA
//#else
//#endif //ATCMD_USART_RX_DMA
#endif
}


#if defined(ATCMD_UART_RX_DOUB_BUF) || defined(UIP_USE_BIGDATA_SIZE) || defined(UIP_USE_BIGDATA_SIZE) //org
void ATCMD_isr(void)
{
	if(on_trans_mode())
	{
#ifndef ATCMD_USART_RX_DMA
		g_u8RecData[g_u32comRbytes++] = USART_ReceiveData(ATCMD_USART);
		
		if(g_u32comRbytes >= RecData_Size){
			g_u32comRbytes = RecData_Size;
		}
#endif //ATCMD_USART_RX_DMA
	
		if(!strcasecmp(g_u8RecData, "+++")){
			recv_cmd_auto_disconnect(ROLE_UDP_SCLIENT);// reveive +++ clear connect table
		}
		
		atcmd_flag = TRUE; //set for !+++
	}else{
#ifndef ATCMD_USART_RX_DMA
		StringLoop = USART_ReceiveData(ATCMD_USART);
		g_u8RecData[g_u32comRbytes++] = StringLoop;
#endif //ATCMD_USART_RX_DMA
    		
		if((StringLoop == '\b') || (StringLoop == '\177')){
			g_u32comRbytes -= 2;
			printf("\b \b");
			
			if(g_u32comRbytes <= 0){
				g_u32comRbytes = 0;
			}
		}

		if((StringLoop == '\r') || (StringLoop == '\n')){
			atcmd_flag = TRUE; //set for \r\n
			//_printf("no connected ATCMD_isr _atcmd_flag %x \r\n", _atcmd_flag);
		}
	}
}

void USART2_IRQ_Routine(void)
{
#ifdef ATCMD_USART_RX_DMA
	uint8_t i;
	uint32_t clr_flag = 0;
	volatile int g_u32comRbytes_temp = 0;
#endif //	ATCMD_USART_RX_DMA
	
#ifdef ATCMD_USART_RX_DMA	
	//if(USART_GetITStatus(AT32_CMD_USART, USART_INT_IDLEF) != RESET) // Received characters modify string
	//if(USART_GetITStatus(AT32_CMD_USART, USART_INT_RDNE) != RESET)
	{
		//DMA_Cmd(ATCMD_USART_DMA_RX_Channel, DISABLE);
		
		clr_flag = AT32_CMD_USART->STS;  //?MUSART_IT_IDLE??Ӡ 
		clr_flag = AT32_CMD_USART->DT; 
		//USART_ReceiveData(ATCMD_USART);//????յ?`?N?G??y????n?A?_????࿲M???????Ӧ졃
		
		g_u32comRbytes_temp = DMA_GetCurrDataCounter(AT32_CMD_USART_DMA_RX_Channel);
		//_printf("1. ATCMD_USART_IRQHandler g_u32comRbytes_temp %x \r\n", g_u32comRbytes_temp);
		if(on_trans_mode()){
#ifdef ATCMD_UART_RX_DOUB_BUF	
			if(Free_Buf_Now == BUF_NO0) //?p?G BUF1 ?ſ?A? DMA ?????յ??ȿ BUF1 
			{	
				ATCMD_USART_DMA_RX_Channel->CMAR = (uint32_t)g_u8RecData1;
#ifdef UIP_USE_BIGDATA_SIZE
				if((at_type.role == 0) || (at_type.role == 1) || (at_type.role == 2)){
					if(g_u32comRbytes_temp != 2920){ //idle = uart "g_u8RecData" have data 
						
						g_u32comRbytes = 2920 - g_u32comRbytes_temp; 
						//printf("A0 = %d\r\n", g_u32comRbytes);
						Free_Buf_Now = BUF_NO1;
					}
				}else{
					if(g_u32comRbytes_temp != 1460){ //idle = uart "g_u8RecData" have data 
						g_u32comRbytes = 1460 - g_u32comRbytes_temp; 
						//printf("A0 = %d\r\n", g_u32comRbytes);
						Free_Buf_Now = BUF_NO1;
					}
				}
#else
				if(g_u32comRbytes_temp != RecData_Size){ //idle = uart "g_u8RecData" have data 
					ATCMD_USART_DMA_RX_Channel->CMAR = (uint32_t)g_u8RecData1;
					g_u32comRbytes = RecData_Size - g_u32comRbytes_temp; 
					//printf("A0 = %d\r\n", g_u32comRbytes);
					Free_Buf_Now = BUF_NO1;
				}/*else{ 
					//printf("A0.1\r\n");
				}*/
#endif //UIP_USE_BIGDATA_SIZE				
			}else{
				ATCMD_USART_DMA_RX_Channel->CMAR = (uint32_t)g_u8RecData;
#ifdef UIP_USE_BIGDATA_SIZE
				if((at_type.role == 0) || (at_type.role == 1) || (at_type.role == 2)){
					if(g_u32comRbytes_temp != 2920){ //idle = uart "g_u8RecData" have data
						g_u32comRbytes1 = 2920 - g_u32comRbytes_temp; 
						//printf("A1 = %d\r\n", g_u32comRbytes1); 
						Free_Buf_Now = BUF_NO0;
					}
				}else{
					if(g_u32comRbytes_temp != 1460){ //idle = uart "g_u8RecData" have data
						g_u32comRbytes1 = 1460 - g_u32comRbytes_temp; 
						//printf("A1 = %d\r\n", g_u32comRbytes1); 
						Free_Buf_Now = BUF_NO0;
					}
				}
#else
				if(g_u32comRbytes_temp != RecData_Size){ //idle = uart "g_u8RecData" have data
					//ATCMD_USART_DMA_RX_Channel->CMAR = (uint32_t)g_u8RecData;
					g_u32comRbytes1 = RecData_Size - g_u32comRbytes_temp; 
					//printf("A1 = %d\r\n", g_u32comRbytes1); 
					Free_Buf_Now = BUF_NO0;
				}/*else{ 
					//printf("A1.1\r\n");
				}*/
#endif //UIP_USE_BIGDATA_SIZE
			}
			Buf_Ok = TRUE; 
#else
			g_u32comRbytes_temp = DMA_GetCurrDataCounter(AT32_CMD_USART_DMA_RX_Channel); //...
				
			if(g_u32comRbytes_temp != RecData_Size){ //idle = uart "g_u8RecData" have data 
				g_u32comRbytes = RecData_Size - g_u32comRbytes_temp; 
				//_printf("2. connected g_u32comRbytes %x \r\n", g_u32comRbytes);
				atcmd_flag = TRUE; //set on rxdma
			}
#endif //ATCMD_UART_RX_DOUB_BUF	
			recv_cmd_auto_disconnect(ROLE_UDP_SCLIENT);// reveive +++ clear connect table
			}else{
#ifdef UIP_USE_BIGDATA_SIZE
			if((at_type.role == 0) || (at_type.role == 1) || (at_type.role == 2)){
				g_u32comRbytes = 2920 - g_u32comRbytes_temp;
			}else{
				g_u32comRbytes = 1460 - g_u32comRbytes_temp;
			}
#else
			g_u32comRbytes_temp = DMA_GetCurrDataCounter(AT32_CMD_USART_DMA_RX_Channel); //...
			g_u32comRbytes = RecData_Size - g_u32comRbytes_temp;
			//_printf("3. no connected g_u32comRbytes %x \r\n", g_u32comRbytes);
#endif //UIP_USE_BIGDATA_SIZE
		 if (g_u32comRbytes != 0)	{ //...
			//_printf("3. no connected g_u32comRbytes %x \r\n", g_u32comRbytes); 
		 if (g_u32comRbytes <= 20){ 
      for (i = 0; i < g_u32comRbytes; i++){ 
				StringLoop = g_u8RecData[i];
				ATCMD_isr(); //start resolv at command 
			}
			//_printf("\r\n======== no connected for loop end ===========\r\n");
		 }
	   }
		 atcmd_flag = TRUE; //set onrxdma
		}
   
		USART_ClearITPendingBit(AT32_CMD_USART, USART_INT_IDLEF);
#ifdef UIP_USE_BIGDATA_SIZE
			if((at_type.role == 0) || (at_type.role == 1) || (at_type.role == 2)){
				ATCMD_USART_DMA_RX_Channel->CNDTR = 2920;//reset DMA lengh
			}else{
				ATCMD_USART_DMA_RX_Channel->CNDTR = 1460;//reset DMA lengh
			}
#else
		//AT32_CMD_USART_DMA_RX_Channel->CNDTR = RecData_Size;//reset DMA lengh
		//if (g_u32comRbytes != 0)		
		// ATCMD_USART_DMA_RX_Channel->TCNT = RecData_Size;//reset DMA lengh	
#endif //UIP_USE_BIGDATA_SIZE
		DMA_ClearFlag(AT32_CMD_USART_DMA_RX_TC_FLAG);  //Clear DMA Flag
			
#if 1			
		DMA_Init_AT();
		DMA_Cmd(AT32_CMD_USART_DMA_RX_Channel, ENABLE); //Enable DMA
#endif		
			
	}
#else
	if(USART_GetITStatus(ATCMD_USART, USART_INT_RDNE) != RESET){ // Received characters modify string	
		ATCMD_isr();
	}else{
		USART_ClearITPendingBit(ATCMD_USART, USART_INT_RDNE);
		USART_ClearITPendingBit(ATCMD_USART, USART_INT_ORERR);
	}
#endif //ATCMD_USART_RX_DMA

}
#endif //defined(ATCMD_UART_RX_DOUB_BUF) || defined(UIP_USE_BIGDATA_SIZE) || defined(UIP_USE_BIGDATA_SIZE)

#ifdef ATCMD_UART_RX_DOUB_BUF	
void RXDMA_IRQ_Routine(void) {
	if(DMA_GetITStatus(AT32_CMD_USART_DMA_RX_TC_FLAG) != RESET){
		//DMA_Cmd(ATCMD_USART_DMA_RX_Channel, DISABLE);   // Diable DMA Channel
		DMA_ClearITPendingBit(AT32_CMD_USART_DMA_RX_GL_FLAG);
		
#ifdef ATCMD_UART_RX_DOUB_BUF		
		if(Free_Buf_Now == BUF_NO0){ //?? BUF1 ??,? DMA ??????? BUF1 
			ATCMD_USART_DMA_RX_Channel->CMAR = (uint32_t)g_u8RecData1;
			//g_u32comRbytes = sizeof(g_u8RecData);
			g_u32comRbytes = ATCMD_USART_DMA_RX_Channel->CNDTR;
			//printf("BUF_NO0. %d\r\n", g_u32comRbytes); //Stone add
			Full_Buf_Now = BUF_NO0;
			Free_Buf_Now = BUF_NO1; 
		}else{
			ATCMD_USART_DMA_RX_Channel->CMAR = (uint32_t)g_u8RecData;
			//g_u32comRbytes1 = sizeof(g_u8RecData1);
			//printf("D1 = %d\r\n", g_u32comRbytes1);
			g_u32comRbytes1 = ATCMD_USART_DMA_RX_Channel->CNDTR;
			//printf("BUF_NO1. %d\r\n", g_u32comRbytes1);   //Stone add
			Full_Buf_Now = BUF_NO1;
			Free_Buf_Now = BUF_NO0; 
		}
		Buf_Ok = TRUE;
#else
		g_u32comRbytes = sizeof(g_u8RecData);
#endif //ATCMD_UART_RX_DOUB_BUF	
	}
	atcmd_flag = TRUE; //set on rxdma irqhandler
//#if 0	
//  _printf("RXDMA_IRQHandler \r\n");
//#endif	
	DMA_ClearFlag(AT32_CMD_USART_DMA_RX_TC_FLAG);  //Clear DMA Flag
	//DMA_SetCurrDataCounter(ATCMD_USART_DMA_RX_Channel, RecData_Size); //??????????
	//DMA_Cmd(ATCMD_USART_DMA_RX_Channel, ENABLE); //Enable DMA Channel
	
#if 1			
	DMA_Init_AT();
	DMA_Cmd(AT32_CMD_USART_DMA_RX_Channel, ENABLE); //Enable DMA
#endif		
}
#endif //ATCMD_UART_RX_DOUB_BUF

/**
* @brief  This function handles USART2 interrupt request.
* @param  None
* @retval None
*/
void AT32_CMD_USART_IRQ_Handler(void)  
{
	/* Disable All Interrupt */
	//_printf("0. ATCMD_USART_IRQHandler g_u32comRbytes_temp \r\n");
	NVIC_DisableIRQ(AT32_CMD_USART_IRQn);
	
#if defined(ATCMD_UART_RX_DOUB_BUF) || defined(UIP_USE_BIGDATA_SIZE) || defined(UIP_USE_BIGDATA_SIZE)
	USART2_IRQ_Routine();
#else
//#ifdef UPDATE_USART2_RX_DMA
	USART2_IRQ_Routine1();
//#endif //UPDATE_USART2_RX_DMA
#endif //defined(ATCMD_UART_RX_DOUB_BUF) || defined(UIP_USE_BIGDATA_SIZE) || defined(UIP_USE_BIGDATA_SIZE)
	
	/* Enable All Interrupt */
	//_printf("4. ATCMD_USART_IRQHandler end \r\n");
	NVIC_EnableIRQ(AT32_CMD_USART_IRQn);
}

#ifdef ATCMD_USART_RX_DMA
void AT32_CMD_USART_RXDMA_IRQHandler(void)
{
	NVIC_DisableIRQ(AT32_CMD_USART_RXDMA_IRQn);

#ifdef ATCMD_UART_RX_DOUB_BUF	
	RXDMA_IRQ_Routine();
#else
//#ifdef UPDATE_USART2_RX_DMA
	RXDMA_IRQ_Routine1();
//#endif //UPDATE_USART2_RX_DMA
#endif

	NVIC_EnableIRQ(AT32_CMD_USART_RXDMA_IRQn);
}
#endif //ATCMD_USART_RX_DMA


#if 0
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(AT32_PRINT_UART, (uint8_t) ch);
	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(AT32_PRINT_UART, USART_FLAG_TRAC) == RESET);
	return ch;
}
#endif
