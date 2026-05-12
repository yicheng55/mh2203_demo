#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "psram.h"

QSPI_CMD_t Psram_Cmd[QSPI_CMD_MAX] = 
{
	/* Reset Operations */
	{RESET_ENABLE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	{RESET_MEMORY_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Identification Operations */
	{READ_ID_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Read Operations */
	{QUAD_INOUT_FAST_READ_CMD, QSPI_ComConfig_IMode_4Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 6},
	/* Program Operations */
	{QUAD_PROG_CMD, QSPI_ComConfig_IMode_4Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	
	{QUAD_ENTER_QUAD_MODE, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	
	{QUAD_EXIT_QUAD_MODE, QSPI_ComConfig_IMode_4Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},		
};


struct
{
	uint8_t state;
	uint8_t *buf;
	uint32_t buf_pos;
	uint32_t buf_len;
}qspi_trans;

QSPI_ComConfig_InitTypeDef  QSPI_ComConfig_InitStructure;

void QSPI_DMA_Send(uint8_t *buf,uint16_t len)
{
	DMA_Cmd(QSPI_DMA_CHANNEL,DISABLE);
	DMA_SetCurrDataCounter(QSPI_DMA_CHANNEL,len);
	QSPI_DMA_CHANNEL->CCR = (QSPI_DMA_CHANNEL->CCR & ~DMA_CCR1_DIR) | DMA_DIR_PeripheralDST;
	QSPI_DMA_CHANNEL->CMAR = (uint32_t)buf;
	DMA_Cmd(QSPI_DMA_CHANNEL,ENABLE);
	QSPI_DMACmd(ENABLE);
}

void QSPI_DMA_Recv(uint8_t *buf,uint16_t len)
{
	DMA_Cmd(QSPI_DMA_CHANNEL,DISABLE);
	DMA_SetCurrDataCounter(QSPI_DMA_CHANNEL,len);
	QSPI_DMA_CHANNEL->CCR = (QSPI_DMA_CHANNEL->CCR & ~DMA_CCR1_DIR) | DMA_DIR_PeripheralSRC;
	QSPI_DMA_CHANNEL->CMAR = (uint32_t)buf;
	DMA_Cmd(QSPI_DMA_CHANNEL,ENABLE);
	QSPI_DMACmd(ENABLE);
}

void QSPI_Indirect_Operation(const QSPI_CMD_t *cmd, uint32_t addr, uint8_t *buf, uint32_t len,uint8_t Trans_Type)
{
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DDRMode     = cmd->DoubleDataRateMode;//DDR模式使能
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = cmd->Instruction_Mode;//指令模式 无/单线/双线/四线传输指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = cmd->Instruction ;//具体指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = cmd->Address_Mode;//地址模式 无/单线/双线/四线传输地址
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = cmd->Address_Size;//地址长度 8/16/24/32
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = cmd->Alternate_Bytes_Mode;//交替字节模式 无/单线/双线/四线传输交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = cmd->Alternate_Bytes_Size;//交替字节长度 8/16/24/32 位交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = cmd->Data_Mode;//数据模式 无/单线/双线/四线传输数据
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = cmd->Functional_Mode;//功能模式 间接写/间接读/自动轮询/内存映射
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = cmd->Dummy_Cycles;//空指令周期数 0~31
	
	while(QSPI_GetFlagStatus(QSPI_FLAG_BUSY) == SET);

	if(len == 0)
		len = 1;
	
	if(cmd->Data_Mode != QSPI_ComConfig_DMode_NoData)
		QSPI_SetDataLength(len - 1);
	QSPI_ComConfig_Init(&QSPI_ComConfig_InitStructure);
	if(cmd->Address_Mode != QSPI_ComConfig_ADMode_NoAddress)
		QSPI_SetAddress(addr);
	if(cmd->Data_Mode != QSPI_ComConfig_DMode_NoData)
	{
		if(Trans_Type == 1) //DMA
		{
			if(cmd->Functional_Mode == QSPI_ComConfig_FMode_Indirect_Write)
				QSPI_DMA_Send(buf,len);
			else
				QSPI_DMA_Recv(buf,len);
			
			while(DMA_GetFlagStatus(QSPI_DMA_FLAG_TC) == RESET);
			DMA_ClearFlag(QSPI_DMA_FLAG_TC);
			DMA_Cmd(QSPI_DMA_CHANNEL,DISABLE);
			QSPI_DMACmd(DISABLE);
		}
		else if(Trans_Type == 2)
		{
			qspi_trans.buf = buf;
			qspi_trans.buf_pos = 0;
			qspi_trans.buf_len = len;
			if(cmd->Functional_Mode == QSPI_ComConfig_FMode_Indirect_Write)
				qspi_trans.state = 1;
			else
				qspi_trans.state = 2;
				QSPI_ITConfig(QSPI_IT_FT, ENABLE);
		}
		else
		{
			if(cmd->Functional_Mode == QSPI_ComConfig_FMode_Indirect_Write)
			{
				for(uint16_t i = 0; i < len; )
				{
					while(QSPI_GetFlagStatus(QSPI_FLAG_FT) == RESET);
					QSPI_SendData32(*(uint32_t *)&buf[i]);
					i += 4;
				}
			}
			else
			{
				for(uint16_t i = 0; i < len;)
				{
					while(QSPI_GetFlagStatus(QSPI_FLAG_FT) == RESET);
					*(uint32_t *)&buf[i] = QSPI_ReceiveData32();
					i += 4;
				}
			}
		}
	}
}

void QSPI_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(QSPIx_CLK_GPIO_CLK | QSPIx_BANK1_CS_GPIO_CLK | QSPIx_BANK1_D0_GPIO_CLK | \
		QSPIx_BANK1_D1_GPIO_CLK | QSPIx_BANK1_D2_GPIO_CLK | QSPIx_BANK1_D3_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_QSPI_AF2, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = QSPIx_CLK_PIN;
	GPIO_Init(QSPIx_CLK_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = QSPIx_BANK1_D0_PIN;
	GPIO_Init(QSPIx_BANK1_D0_GPIO_PORT, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin = QSPIx_BANK1_D1_PIN;
	GPIO_Init(QSPIx_BANK1_D1_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = QSPIx_BANK1_D2_PIN;
	GPIO_Init(QSPIx_BANK1_D2_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = QSPIx_BANK1_D3_PIN;
	GPIO_Init(QSPIx_BANK1_D3_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = QSPIx_BANK1_CS_PIN;
	GPIO_Init(QSPIx_BANK1_CS_GPIO_PORT, &GPIO_InitStructure);
}

void QSPI_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t) & QUADSPI->DR ;
	DMA_InitStructure.DMA_MemoryBaseAddr		= 0;	
	DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_BufferSize			= 0;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
	DMA_Init(QSPI_DMA_CHANNEL, &DMA_InitStructure);
}


void QSPI_M2M_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_StructInit(&DMA_InitStructure);
	DMA_DeInit(QSPI_M2M_DMA_CHANNEL);	
	
	DMA_InitStructure.DMA_PeripheralBaseAddr	= 0;
	DMA_InitStructure.DMA_MemoryBaseAddr		= 0;	
	DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_BufferSize			= 0;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M                   = DMA_M2M_Enable;
	DMA_Init(QSPI_M2M_DMA_CHANNEL, &DMA_InitStructure);
}

void QSPI_Configuration(void)
{
	QSPI_InitTypeDef  QSPI_InitStructure;
	
	PRINTF_LOG("QSPI Init\r\n");
	QSPI_GPIO_Config();
	QSPI_DMA_Config();
	QSPI_M2M_DMA_Config();
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_QSPI,ENABLE);
	
	QSPI_DeInit();

	/* Initialize QuadSPI ------------------------------------------------------*/
	QSPI_StructInit(&QSPI_InitStructure);
	QSPI_InitStructure.QSPI_SShift    = QSPI_SShift_NoShift;//采样偏移
	QSPI_InitStructure.QSPI_Prescaler = 2;			           		//QSPI 预分频器值
	QSPI_InitStructure.QSPI_FSize     = 20;  /*2M Byte*/         	//闪存大小
	QSPI_InitStructure.QSPI_CKMode    = QSPI_CKMode_Mode0;         //时钟模式
	QSPI_InitStructure.QSPI_CSHTime   = QSPI_CSHTime_2Cycle;       //芯片选择高时间
	QSPI_Init(&QSPI_InitStructure);
	
	QSPI_SetFIFOThreshold(0);
	
	QSPI_Cmd(ENABLE);
	
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DHHC        = QSPI_ComConfig_DHHC_Enable;//DDR模式下数据输出延时使能
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;//仅发送一次指令模块使能
}


void QUADSPI_IRQHandler(void)
{
	if(QSPI_GetITStatus(QSPI_IT_TO))
	{
		QSPI_ClearITPendingBit(QSPI_IT_TO);
	}
	if(QSPI_GetITStatus(QSPI_IT_SM))
	{
		QSPI_ClearITPendingBit(QSPI_IT_SM);
	}
	if(QSPI_GetITStatus(QSPI_IT_FT))
	{
		if(qspi_trans.state == 1)
		{
			QSPI_SendData32(*(uint32_t *)&qspi_trans.buf[qspi_trans.buf_pos]);
			qspi_trans.buf_pos += 4;
		}
		else if(qspi_trans.state == 2)
		{
			*(uint32_t *)&(qspi_trans.buf[qspi_trans.buf_pos]) = QSPI_ReceiveData32();
			qspi_trans.buf_pos += 4;
		}
		if(qspi_trans.buf_pos >= qspi_trans.buf_len)
		{
			QSPI_ITConfig(QSPI_IT_FT, DISABLE);
		}
	}
	if(QSPI_GetITStatus(QSPI_IT_TC))
	{
		QSPI_ClearITPendingBit(QSPI_IT_TC);
	}
	if(QSPI_GetITStatus(QSPI_IT_TE))
	{
		QSPI_ClearITPendingBit(QSPI_IT_TE);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PSRANM_Configuration(void)
{
	uint8_t TEST_ID[8];
	uint8_t i;

	QSPI_Indirect_Operation(&Psram_Cmd[QSPI_CMD_EXIT_QUAD_MODE],0,NULL,0,0);
	QSPI_Indirect_Operation(&Psram_Cmd[QSPI_CMD_RESET_ENABLE],0,NULL,0,0);
	QSPI_Indirect_Operation(&Psram_Cmd[QSPI_CMD_RESET_MEMORY],0,NULL,0,0);
	QSPI_Indirect_Operation(&Psram_Cmd[QSPI_CMD_READ_ID],0,TEST_ID,8,0);
	QSPI_Indirect_Operation(&Psram_Cmd[QSPI_CMD_ENTER_QUAD_MODE],0,NULL,0,0);
	
	for(i = 0; i < sizeof(TEST_ID); i++)
	{
		PRINTF_LOG("ID[%d]:0x%x\n",i,TEST_ID[i]);
	}

	QSPI_MMW_Cmd(ENABLE);
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DDRMode     = Psram_Cmd[QSPI_CMD_QUAD_PROG].DoubleDataRateMode;//DDR模式使能
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = Psram_Cmd[QSPI_CMD_QUAD_PROG].Instruction_Mode;//指令模式 无/单线/双线/四线传输指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = Psram_Cmd[QSPI_CMD_QUAD_PROG].Instruction ;//具体指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = Psram_Cmd[QSPI_CMD_QUAD_PROG].Address_Mode;//地址模式 无/单线/双线/四线传输地址
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = Psram_Cmd[QSPI_CMD_QUAD_PROG].Address_Size;//地址长度 8/16/24/32
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = Psram_Cmd[QSPI_CMD_QUAD_PROG].Alternate_Bytes_Mode;//交替字节模式 无/单线/双线/四线传输交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = Psram_Cmd[QSPI_CMD_QUAD_PROG].Alternate_Bytes_Size;//交替字节长度 8/16/24/32 位交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = Psram_Cmd[QSPI_CMD_QUAD_PROG].Data_Mode;//数据模式 无/单线/双线/四线传输数据
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = Psram_Cmd[QSPI_CMD_QUAD_PROG].Dummy_Cycles;
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Memory_Mapped;//功能模式 间接写/间接读/自动轮询/内存映射
	QSPI_WriteComConfig_Init(&QSPI_ComConfig_InitStructure);

	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DDRMode     = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].DoubleDataRateMode;//DDR模式使能
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Instruction_Mode;//指令模式 无/单线/双线/四线传输指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Instruction ;//具体指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Address_Mode;//地址模式 无/单线/双线/四线传输地址
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Address_Size;//地址长度 8/16/24/32
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Alternate_Bytes_Mode;//交替字节模式 无/单线/双线/四线传输交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Alternate_Bytes_Size;//交替字节长度 8/16/24/32 位交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Data_Mode;//数据模式 无/单线/双线/四线传输数据
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = Psram_Cmd[QSPI_CMD_QUAD_INOUT_FAST_READ].Dummy_Cycles;
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Memory_Mapped;//功能模式 间接写/间接读/自动轮询/内存映射
	QSPI_ComConfig_Init(&QSPI_ComConfig_InitStructure);	
}

uint8_t Test_Send_Buf[0x4000];
uint8_t Test_Recv_Buf[0x4000];
void PSRANM_Test(void)
{
	{
		uint8_t *Buf;
		PRINTF_LOG("\r\n***PSRAM Test***\r\n");
	  
		for(uint16_t i = 0; i < sizeof(Test_Send_Buf); i++)
			Test_Send_Buf[i] = i;
		
		Buf = (uint8_t *)Test_Send_Buf;
		for(uint32_t Test_Addr = 0xB1000000; Test_Addr < 0xB1000000 + sizeof(Test_Send_Buf); Test_Addr += 1)
		{
			*(volatile uint8_t *)Test_Addr = *Buf++;
		}
		PRINTF_LOG("CPU Write(Byte) 16K Byte OK\n");

		Buf = (uint8_t *)Test_Recv_Buf;
		__disable_irq();  
		for(uint32_t Test_Addr = 0xB1000000; Test_Addr < 0xB1000000 + sizeof(Test_Send_Buf); Test_Addr += 1)
		{
		   *Buf++ = *(volatile uint8_t *)Test_Addr;
			QUADSPI->CR |= BIT(1);
			while((QUADSPI->CR & BIT(1)) != 0x00);
		}
		__enable_irq();
		PRINTF_LOG("CPU Read(Byte) 16K Byte OK\n");
		
		 for(uint32_t i = 0; i < sizeof(Test_Recv_Buf); i++)
		{
			if(Test_Send_Buf[i] != Test_Recv_Buf[i])
			{
				PRINTF_LOG("Test_Send_Buf[%d] = 0x%x\n",i,Test_Send_Buf[i]);
				PRINTF_LOG("Test_Recv_Buf[%d] = 0x%x\n",i,Test_Recv_Buf[i]);
				PRINTF_LOG("Data Check Fault\r\n");
				while(1);
			}
			else
			{
				__NOP();
			}
		}
		 PRINTF_LOG("Data Check OK\r\n");
		
	}
	
	{
		uint16_t *Buf;
		PRINTF_LOG("\r\n***PSRAM Test***\r\n");
	  
		for(uint16_t i = 0; i < sizeof(Test_Send_Buf); i++)
			Test_Send_Buf[i] = i;
		
		Buf = (uint16_t *)Test_Send_Buf;
		for(uint32_t Test_Addr = 0xB1000000; Test_Addr < 0xB1000000 + sizeof(Test_Send_Buf); Test_Addr += 2)
		{
			*(volatile uint16_t *)Test_Addr = *Buf++;		
		}
		PRINTF_LOG("CPU Write(Halfword) 16K Byte OK\n");
		
		Buf = (uint16_t *)Test_Recv_Buf;
		__disable_irq();  
		for(uint32_t Test_Addr = 0xB1000000; Test_Addr < 0xB1000000 + sizeof(Test_Send_Buf); Test_Addr += 2)
		{
		   *Buf++ = *(volatile uint16_t *)Test_Addr;
			QUADSPI->CR |= BIT(1);
			while((QUADSPI->CR & BIT(1)) != 0x00);			
		}
		__enable_irq();
		PRINTF_LOG("CPU Read(Halfword) 16K Byte OK\n");
		
		 for(uint32_t i = 0; i < sizeof(Test_Recv_Buf); i++)
		{
			if(Test_Send_Buf[i] != Test_Recv_Buf[i])
			{
				PRINTF_LOG("Test_Send_Buf[%d] = 0x%x\n",i,Test_Send_Buf[i]);
				PRINTF_LOG("Test_Recv_Buf[%d] = 0x%x\n",i,Test_Recv_Buf[i]);
				PRINTF_LOG("Data Check Fault\r\n");
				while(1);
			}
			else
			{
				__NOP();
			}
		}
		 PRINTF_LOG("Data Check OK\r\n");
		
	}
	
	{
		uint32_t *Buf;
		PRINTF_LOG("\r\n***PSRAM Test***\r\n");
		
		for(uint16_t i = 0; i < sizeof(Test_Send_Buf); i++)
			Test_Send_Buf[i] = i;
		
		Buf = (uint32_t *)Test_Send_Buf;
		for(uint32_t Test_Addr = 0xB1000000; Test_Addr < 0xB1000000 + sizeof(Test_Send_Buf); Test_Addr += 4)
		{
			*(volatile uint32_t *)Test_Addr = *Buf++;
		}
		SysTick->CTRL = 0;
		PRINTF_LOG("CPU Write(Word) 16K Byte OK\n");
		
		Buf = (uint32_t *)Test_Recv_Buf;
		__disable_irq();  
		for(uint32_t Test_Addr = 0xB1000000; Test_Addr < 0xB1000000 + sizeof(Test_Send_Buf); Test_Addr += 4)
		{
		   *Buf++ = *(volatile uint32_t *)Test_Addr;		
				QUADSPI->CR |= BIT(1);
				while((QUADSPI->CR & BIT(1)) != 0x00);
		}
		__enable_irq();
		PRINTF_LOG("CPU Read(Word) 16K Byte OK\n");
		
		 for(uint32_t i = 0; i < sizeof(Test_Recv_Buf); i++)
		{
			if(Test_Send_Buf[i] != Test_Recv_Buf[i])
			{
				PRINTF_LOG("Test_Send_Buf[%d] = 0x%x\n",i,Test_Send_Buf[i]);
				PRINTF_LOG("Test_Recv_Buf[%d] = 0x%x\n",i,Test_Recv_Buf[i]);
				PRINTF_LOG("Data Check Fault\r\n");
				while(1);
			}
			else
			{
				__NOP();
			}
		}
		 PRINTF_LOG("Data Check OK\r\n");
	}
}
