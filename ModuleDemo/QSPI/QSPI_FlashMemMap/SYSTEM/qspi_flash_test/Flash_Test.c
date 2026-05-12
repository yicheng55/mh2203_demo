#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Flash_Test.h"

#define PRINTF_LOG 	printf

void QSPI_DMA_Send(uint8_t *buf,uint16_t len);
void QSPI_DMA_Recv(uint8_t *buf,uint16_t len);

struct
{
	uint8_t state;
	uint8_t *buf;
	uint32_t buf_pos;
	uint32_t buf_len;
}qspi_trans;

uint32_t Flash_Capcity = 0;//Flash 容量
uint32_t Manufactor = 0;
QSPI_ComConfig_InitTypeDef  QSPI_ComConfig_InitStructure;
volatile uint32_t QSPI_SR_FLAG;

void Flash_Indirect_Operation(const Flash_CMD_t *cmd, uint32_t addr, uint8_t *buf, uint32_t len,uint8_t Trans_Type)
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

void Flash_Auto_Polling_Operation(const Flash_CMD_t *cmd, uint32_t QSPI_Match, uint32_t QSPI_Mask , uint32_t QSPI_Match_Mode, uint8_t AutoStop,uint32_t len,uint32_t addr)
{
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DDRMode     = cmd->DoubleDataRateMode;//DDR模式使能
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = cmd->Instruction_Mode;//指令模式 无/单线/双线/四线传输指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = cmd->Instruction ;//具体指令
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = cmd->Address_Mode;//地址模式 无/单线/双线/四线传输地址
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = cmd->Address_Size;//地址长度 8/16/24/32
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = cmd->Alternate_Bytes_Mode;//交替字节模式 无/单线/双线/四线传输交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = cmd->Alternate_Bytes_Size;//交替字节长度 8/16/24/32 位交替字节
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = cmd->Data_Mode;//数据模式 无/单线/双线/四线传输数据
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Auto_Polling;//功能模式 间接写/间接读/自动轮询/内存映射
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = cmd->Dummy_Cycles;//空指令周期数 0~31
	
	if(len == 0)
		len = 1;
	
	while(QSPI_GetFlagStatus(QSPI_FLAG_BUSY) == SET);
	if(cmd->Data_Mode != QSPI_ComConfig_DMode_NoData)
		QSPI_SetDataLength(len - 1);
	if(cmd->Address_Mode != QSPI_ComConfig_ADMode_NoAddress)
		QSPI_SetAddress(addr);
	QSPI_AutoPollingMode_Config(QSPI_Match,QSPI_Mask,QSPI_Match_Mode);
	if(AutoStop)
		QSPI_AutoPollingModeStopCmd(ENABLE);
	else
		QSPI_AutoPollingModeStopCmd(DISABLE);
	QSPI_ComConfig_Init(&QSPI_ComConfig_InitStructure);

	while(QSPI_GetFlagStatus(QSPI_FLAG_SM) != SET);
	if(!AutoStop)
		QSPI_AbortRequest();
    while (QSPI_GetFlagStatus(QSPI_FLAG_BUSY) == SET)
        ;
	
}

void Flash_Config(uint8_t Manufactor)
{
	uint8_t State[3];

	switch(Manufactor)
	{
		case MANUFATOR_MXIC:
		{
			Flash_Cmd[FLASH_CMD_ENTER_QUAD_CMD].Instruction = MXIC_ENTER_QUAD_CMD;
			Flash_Cmd[FLASH_CMD_EXIT_QUAD_CMD].Instruction = MXIC_EXIT_QUAD_CMD;
			Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ_DTR].Dummy_Cycles = 6;
			
			Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS1],0,State,1,0);
			if((State[0] & 0x40) == 0)
			{
				State[0] |= 0x40;
				Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_ENABLE],0,NULL,0,0);
				Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_STATUS1],0,State,1,0);
				Flash_Auto_Polling_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS1],0x00000000,0x00000001,QSPI_PMM_AND,ENABLE,1,0);
			}
			break;
		}
		case MANUFATOR_ZBIT:
		{
			Flash_Cmd[FLASH_CMD_ENTER_QUAD_CMD].Instruction = ZBIT_ENTER_QUAD_CMD;
			Flash_Cmd[FLASH_CMD_EXIT_QUAD_CMD].Instruction = ZBIT_EXIT_QUAD_CMD;
			Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ_DTR].Dummy_Cycles = 8;
			
			Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS2],0,State,1,0);
			if((State[0] & 0x02) == 0)
			{
				State[0] |= 0x02;
				Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_ENABLE],0,NULL,0,0);
				Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_STATUS2],0,State,1,0);
				Flash_Auto_Polling_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS1],0x00000000,0x00000001,QSPI_PMM_AND,ENABLE,1,0);
			}
			break;
		}
		default:
			break;
	}
}

char *Get_Flash_Manufature(uint8_t id)
{
	for(uint8_t i = 0; i < sizeof(Flash_Info)/sizeof(Flash_Info_t);i++)
	{
		if(Flash_Info[i].id == id)
			return (char *)Flash_Info[i].manufacture;
	}
	return "Unknown";
}

void Flash_Init(void)
{
	uint8_t Flash_ID[4];
	uint8_t Temp_Buf[4];
	
	Flash_CMD_t Temp_Flash_Cmd;

	memcpy(&Temp_Flash_Cmd,&Flash_Cmd[FLASH_CMD_EXIT_QUAD_CMD],sizeof(Flash_CMD_t));

	Temp_Flash_Cmd.Instruction = MXIC_EXIT_QUAD_CMD;
	Flash_Indirect_Operation(&Temp_Flash_Cmd,0,NULL,0,0);
	Temp_Flash_Cmd.Instruction = ZBIT_EXIT_QUAD_CMD;
	Flash_Indirect_Operation(&Temp_Flash_Cmd,0,NULL,0,0);

	Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS3],0,Temp_Buf,1,0);
	if(Temp_Buf[0] != 0x70)
	{
		Temp_Buf[0] = 0x70;
		Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_ENABLE],0,NULL,0,0);
		Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_STATUS3],0,Temp_Buf,1,0);
		Flash_Auto_Polling_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS1],0x00000000,0x00000001,QSPI_PMM_AND,ENABLE,1,0);
	}
	Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_READ_ID],0,Flash_ID,2,0);	
	PRINTF_LOG("ID : %02X %02X\r\n",Flash_ID[0], Flash_ID[1]);
	Flash_Config(Flash_ID[0]);
	Flash_Capcity = 2 << Flash_ID[1];
	while(QSPI_GetFlagStatus(QSPI_FLAG_BUSY) == SET);
	if(Flash_Capcity != 0)
		QUADSPI->DCR = (QUADSPI->DCR & ~QUADSPI_DCR_FSIZE) | ((Flash_ID[1]) << 16);
	PRINTF_LOG("Manufacture : %s\r\nCapcity : %d%s\r\n",Get_Flash_Manufature(Flash_ID[0])
		,Flash_Capcity >= 0x100000?(Flash_Capcity/0x100000):(Flash_Capcity/0x400),Flash_Capcity >= 0x100000?"MB":"KB");
	Manufactor = Flash_ID[0];
}

void FLASH_QPI_Mode_Config(uint8_t State)
{
	if(State)
	{
		Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_ENTER_QUAD_CMD],0,NULL,0,0);
		Flash_Cmd[FLASH_CMD_RESET_ENABLE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_RESET_MEMORY].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_WRITE_ENABLE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_WRITE_DISABLE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		
		Flash_Cmd[FLASH_CMD_READ_STATUS1].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_READ_STATUS1].Data_Mode = QSPI_ComConfig_DMode_4Line;
		
		Flash_Cmd[FLASH_CMD_WRITE_STATUS1].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_WRITE_STATUS1].Data_Mode = QSPI_ComConfig_DMode_4Line;
		
		Flash_Cmd[FLASH_CMD_READ_STATUS2].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_READ_STATUS2].Data_Mode = QSPI_ComConfig_DMode_4Line;
		
		Flash_Cmd[FLASH_CMD_WRITE_STATUS2].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_WRITE_STATUS2].Data_Mode = QSPI_ComConfig_DMode_4Line;
		
		Flash_Cmd[FLASH_CMD_READ_STATUS3].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_READ_STATUS3].Data_Mode = QSPI_ComConfig_DMode_4Line;
		
		Flash_Cmd[FLASH_CMD_WRITE_STATUS3].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_WRITE_STATUS3].Data_Mode = QSPI_ComConfig_DMode_4Line;
		
		Flash_Cmd[FLASH_CMD_WAKEUP].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_WAKEUP].Data_Mode = QSPI_ComConfig_DMode_4Line;
		Flash_Cmd[FLASH_CMD_WAKEUP].Dummy_Cycles = 6;
		
		Flash_Cmd[FLASH_CMD_DEEP_SLEEP].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_ENTER_4_BYTE_ADDR].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_EXIT_4_BYTE_ADDR].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		if(Manufactor == MANUFATOR_ZBIT)
			Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ].Dummy_Cycles = 4;
		Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ_DTR].Instruction_Mode = QSPI_ComConfig_IMode_4Line;		
		Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_DDR].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Address_Mode = QSPI_ComConfig_ADMode_4Line;
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Data_Mode = QSPI_ComConfig_DMode_4Line;
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Dummy_Cycles = 8;
		
		Flash_Cmd[FLASH_CMD_SECTOR_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_SECTOR_ERASE].Address_Mode = QSPI_ComConfig_ADMode_4Line;
		
		Flash_Cmd[FLASH_CMD_BLOCK32_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_BLOCK32_ERASE].Address_Mode = QSPI_ComConfig_ADMode_4Line;
		
		Flash_Cmd[FLASH_CMD_BLOCK_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_BLOCK_ERASE].Address_Mode = QSPI_ComConfig_ADMode_4Line;
		
		Flash_Cmd[FLASH_CMD_CHIP_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		
		Flash_Cmd[FLASH_CMD_PAGE_PROG].Instruction_Mode = QSPI_ComConfig_IMode_4Line;
		Flash_Cmd[FLASH_CMD_PAGE_PROG].Address_Mode = QSPI_ComConfig_ADMode_4Line;
		Flash_Cmd[FLASH_CMD_PAGE_PROG].Data_Mode = QSPI_ComConfig_DMode_4Line;
	}
	else
	{
		Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_EXIT_QUAD_CMD],0,NULL,0,0);
		Flash_Cmd[FLASH_CMD_RESET_ENABLE].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_RESET_MEMORY].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_WRITE_ENABLE].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		
		Flash_Cmd[FLASH_CMD_READ_STATUS1].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_READ_STATUS1].Data_Mode = QSPI_ComConfig_DMode_1Line;
		
		Flash_Cmd[FLASH_CMD_WRITE_STATUS1].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_WRITE_STATUS1].Data_Mode = QSPI_ComConfig_DMode_1Line;
		
		Flash_Cmd[FLASH_CMD_READ_STATUS2].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_READ_STATUS2].Data_Mode = QSPI_ComConfig_DMode_1Line;
		
		Flash_Cmd[FLASH_CMD_WRITE_STATUS2].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_WRITE_STATUS2].Data_Mode = QSPI_ComConfig_DMode_1Line;
		
		Flash_Cmd[FLASH_CMD_READ_STATUS3].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_READ_STATUS3].Data_Mode = QSPI_ComConfig_DMode_1Line;
		
		Flash_Cmd[FLASH_CMD_WRITE_STATUS3].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_WRITE_STATUS3].Data_Mode = QSPI_ComConfig_DMode_1Line;
		
		Flash_Cmd[FLASH_CMD_WAKEUP].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_WAKEUP].Data_Mode = QSPI_ComConfig_DMode_1Line;
		Flash_Cmd[FLASH_CMD_WAKEUP].Dummy_Cycles = 24;
		
		Flash_Cmd[FLASH_CMD_DEEP_SLEEP].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_ENTER_4_BYTE_ADDR].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_EXIT_4_BYTE_ADDR].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		if(Manufactor == MANUFATOR_ZBIT)
			Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ].Dummy_Cycles = 6;
		Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ_DTR].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_DDR].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Address_Mode = QSPI_ComConfig_ADMode_1Line;
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Data_Mode = QSPI_ComConfig_DMode_1Line;
		Flash_Cmd[FLASH_CMD_FAST_READ_DTR].Dummy_Cycles = 6;
		
		Flash_Cmd[FLASH_CMD_SECTOR_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_SECTOR_ERASE].Address_Mode = QSPI_ComConfig_ADMode_1Line;
		
		Flash_Cmd[FLASH_CMD_BLOCK32_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_BLOCK32_ERASE].Address_Mode = QSPI_ComConfig_ADMode_1Line;
		
		Flash_Cmd[FLASH_CMD_BLOCK_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_BLOCK_ERASE].Address_Mode = QSPI_ComConfig_ADMode_1Line;
		
		Flash_Cmd[FLASH_CMD_CHIP_ERASE].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		
		Flash_Cmd[FLASH_CMD_PAGE_PROG].Instruction_Mode = QSPI_ComConfig_IMode_1Line;
		Flash_Cmd[FLASH_CMD_PAGE_PROG].Address_Mode = QSPI_ComConfig_ADMode_1Line;
		Flash_Cmd[FLASH_CMD_PAGE_PROG].Data_Mode = QSPI_ComConfig_DMode_1Line;
	}
}

void Flash_Indirect_Cmd_Set(const Flash_CMD_t *cmd)
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
	
	QSPI_ComConfig_Init(&QSPI_ComConfig_InitStructure);
}

void QSPI_M2M_DMA_Read(uint32_t Addr,uint8_t *buf,uint16_t len)
{
	DMA_Cmd(QSPI_M2M_DMA_CHANNEL,DISABLE);
	DMA_SetCurrDataCounter(QSPI_M2M_DMA_CHANNEL,len);
	QSPI_M2M_DMA_CHANNEL->CPAR = Addr;
	QSPI_M2M_DMA_CHANNEL->CMAR = (uint32_t)buf;
	DMA_Cmd(QSPI_M2M_DMA_CHANNEL,ENABLE);
}

uint8_t Test_Recv_Buf[0x1000];
uint8_t Test_Send_Buf[0x1000];
#define Test_Addr	0x1000
void Flash_MemMap_Test(void)
{
	uint32_t Test_Len = sizeof(Test_Recv_Buf);
	uint32_t Temp = 0, data1, data2;
	Flash_CMD_t Temp_Flash_Cmd;
	
	uint32_t Prog_Cmd[] = 
	{
		FLASH_CMD_READ,
		FLASH_CMD_DUAL_OUT_FAST_READ,
		FLASH_CMD_QUAD_INOUT_FAST_READ,
		FLASH_CMD_FAST_READ_DTR,
		FLASH_CMD_QUAD_INOUT_FAST_READ_DTR,
		FLASH_CMD_QUAD_INOUT_FAST_READ,
		FLASH_CMD_FAST_READ_DTR,
		FLASH_CMD_QUAD_INOUT_FAST_READ_DTR,
	};
	PRINTF_LOG("\r\n***qspi mem map test***\r\n");
	
	PRINTF_LOG("Test Addr = %08X, Test Len = %08X\r\n",Test_Addr, Test_Len);
	PRINTF_LOG("Earse : ");
	Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_ENABLE],0,NULL,0,0);
	Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_SECTOR_ERASE],Test_Addr,NULL,0,0);
	Flash_Auto_Polling_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS1],0x00000000,0x00000101,QSPI_PMM_AND,ENABLE,1,0);
	PRINTF_LOG("OK\r\n");
	
	PRINTF_LOG("Check Erase : ");
	Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_READ],Test_Addr,Test_Recv_Buf,Test_Len,0);
	for(uint16_t i = 0; i < Test_Len; i++)
	{
		if(Test_Recv_Buf[i] != 0xFF)
		{
			PRINTF_LOG("Erase Fail\n");
			PRINTF_LOG("Test_Recv_Buf[%d] = 0x%x\n",i,Test_Recv_Buf[i]);
			while(1);
		}
	}
	PRINTF_LOG("OK\r\n");
	
	PRINTF_LOG("Program : ");
	memset(Test_Send_Buf,0xA5,sizeof(Test_Send_Buf));
	for(uint32_t i = 0; i < Test_Len;i += Temp)
	{
		Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_WRITE_ENABLE],0,NULL,0,0);
		Temp = (Test_Len - i) > 0x100 - (Test_Addr & 0xFF)?0x100 - (Test_Addr & 0xFF):Test_Len - i;
		Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_PAGE_PROG],Test_Addr + i,&Test_Send_Buf[i],Temp,0);
		Flash_Auto_Polling_Operation(&Flash_Cmd[FLASH_CMD_READ_STATUS1],0x00000000,0x00000101,QSPI_PMM_AND,ENABLE,1,0);
	}
	PRINTF_LOG("OK\r\n");
	
	PRINTF_LOG("Program Check : ");
	Flash_Indirect_Operation(&Flash_Cmd[FLASH_CMD_READ],Test_Addr,Test_Recv_Buf,Test_Len,0);
	
	for(uint16_t i = 0; i < Test_Len; i++)
	{
		if(Test_Recv_Buf[i] != Test_Send_Buf[i])
		{
			PRINTF_LOG("Program Fail\n");
			PRINTF_LOG("Test_Recv_Buf[%d] = 0x%x\n",i,Test_Recv_Buf[i]);
			PRINTF_LOG("Test_Send_Buf[%d] = 0x%x\n",i,Test_Send_Buf[i]);
			while(1);
		}
	}
	PRINTF_LOG("OK\r\n");
	
	for(uint32_t j = 0; j < sizeof(Prog_Cmd)/sizeof(uint32_t);j++)
	{
		if(j == 0)
			PRINTF_LOG("\n\n单线\n");
		else if(j == 1)
			PRINTF_LOG("\n\n两线\r\n");
		else if(j == 2)
			PRINTF_LOG("\n\n四线\n");
		else if(j == 3)
			PRINTF_LOG("\n\nSPI DDR\n");
		else if(j == 4)
			PRINTF_LOG("\n\nQSPI DDR\n");
		else if(j == 5)
		{
			FLASH_QPI_Mode_Config(ENABLE);
			PRINTF_LOG("\n\nQPI\n");
		}
		else if(j == 6)
			PRINTF_LOG("\n\nSPI DDR\n");
		else if(j == 7)
			PRINTF_LOG("\n\nQPI DDR\n");
		
		PRINTF_LOG("Core Read Data : ");
		memcpy(&Temp_Flash_Cmd,&Flash_Cmd[Prog_Cmd[j]],sizeof(Flash_CMD_t));
		Temp_Flash_Cmd.Functional_Mode = QSPI_ComConfig_FMode_Memory_Mapped;
		Flash_Indirect_Cmd_Set(&Temp_Flash_Cmd);
		
		for(uint16_t i = 0; i < Test_Len; i++)
		{
			data1 = *(uint8_t *)(QSPI_XIP_ADDR + Test_Addr + i);
			data2 = Test_Send_Buf[i];
			if(data1 != data2)
			{
				PRINTF_LOG("%08X : %02X != %02X\r\n",Test_Addr + i,data1,data2);
				PRINTF_LOG("%08X : %02X != %02X\r\n",Test_Addr + i,*(uint8_t *)(QSPI_XIP_ADDR + Test_Addr + i),Test_Send_Buf[i]);
				while(1);
			}
		}
		
		for(uint16_t i = 0; i < Test_Len;)
		{
			if(Test_Len - i >=2)
			{
				data1 = *(uint16_t *)(QSPI_XIP_ADDR + Test_Addr + i);
				data2 = *(uint16_t *)&Test_Send_Buf[i];
				if(data1 != data2)
				{
					PRINTF_LOG("%08X : %04X != %04X\r\n",Test_Addr + i,data1,data2);
					PRINTF_LOG("%08X : %04X != %04X\r\n",Test_Addr + i,*(uint16_t *)(QSPI_XIP_ADDR + Test_Addr + i),*(uint16_t *)&Test_Send_Buf[i]);
					while(1);
				}
				i+=2;
			}
			else
			{
				data1 = *(uint8_t *)(QSPI_XIP_ADDR + Test_Addr + i);
				data2 = Test_Send_Buf[i];
				if(data1 != data2)
				{
					PRINTF_LOG("%08X : %02X != %02X\r\n",Test_Addr + i,data1,data2);
					PRINTF_LOG("%08X : %02X != %02X\r\n",Test_Addr + i,*(uint8_t *)(QSPI_XIP_ADDR + Test_Addr + i),Test_Send_Buf[i]);
					while(1);
				}
				i+=1;
			}
		}
		
		for(uint16_t i = 0; i < Test_Len; )
		{
			if(Test_Len - i >=4)
			{
				data1 = *(uint32_t *)(QSPI_XIP_ADDR + Test_Addr + i);
				data2 = *(uint32_t *)&Test_Send_Buf[i];
				if(data1 != data2)
				{
					PRINTF_LOG("%08X : %08X != %08X\r\n",Test_Addr + i,data1,data2);
					PRINTF_LOG("%08X : %08X != %08X\r\n",Test_Addr + i,*(uint32_t *)(QSPI_XIP_ADDR + Test_Addr + i),*(uint32_t *)&Test_Send_Buf[i]);
					while(1);
				}
				i+=4;
			}
			else if(Test_Len - i >=2)
			{
				data1 = *(uint16_t *)(QSPI_XIP_ADDR + Test_Addr + i);
				data2 = *(uint16_t *)&Test_Send_Buf[i];
				if(data1 != data2)
				{
					PRINTF_LOG("%08X : %04X != %04X\r\n",Test_Addr + i,data1,data2);
					PRINTF_LOG("%08X : %04X != %04X\r\n",Test_Addr + i,*(uint16_t *)(QSPI_XIP_ADDR + Test_Addr + i),*(uint16_t *)&Test_Send_Buf[i]);
					while(1);
				}
				i+=2;
			}
			else
			{
				data1 = *(uint8_t *)(QSPI_XIP_ADDR + Test_Addr + i);
				data2 = Test_Send_Buf[i];
				if(data1 != data2)
				{
					PRINTF_LOG("%08X : %02X != %02X\r\n",Test_Addr + i,data1,data2);
					PRINTF_LOG("%08X : %02X != %02X\r\n",Test_Addr + i,*(uint8_t *)(QSPI_XIP_ADDR + Test_Addr + i),Test_Send_Buf[i]);
					while(1);
				}
				i+=1;
			}
		}

		PRINTF_LOG("OK\r\nDMA Read Data: ");
		memset(Test_Recv_Buf,0,sizeof(Test_Recv_Buf));
		QSPI_M2M_DMA_Read(QSPI_XIP_ADDR + Test_Addr,Test_Recv_Buf,Test_Len);
		while(DMA_GetFlagStatus(QSPI_M2M_DMA_FLAG_TC) == RESET);
		DMA_ClearFlag(QSPI_M2M_DMA_FLAG_TC);
		DMA_Cmd(QSPI_M2M_DMA_CHANNEL,DISABLE);
		for(uint16_t i = 0; i < Test_Len; i++)
		{
			if(Test_Recv_Buf[i] != Test_Send_Buf[i])
			{
				PRINTF_LOG("%d : %02X != %02X\r\n",i,Test_Recv_Buf[i],Test_Send_Buf[i]);
				while(1);
			}
		}

		QSPI_AbortRequest();
		if(j == 7)
			FLASH_QPI_Mode_Config(DISABLE);
		PRINTF_LOG("OK\r\n");
	}
}

/******************************************************************************************************************/
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
	NVIC_InitTypeDef NVIC_InitStruct;
	
	PRINTF_LOG("QSPI Init\r\n");
	QSPI_GPIO_Config();
	QSPI_DMA_Config();
	QSPI_M2M_DMA_Config();
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_QSPI,ENABLE);
	
	QSPI_DeInit();

	/* Initialize QuadSPI ------------------------------------------------------*/
	QSPI_StructInit(&QSPI_InitStructure);
	QSPI_InitStructure.QSPI_SShift    = QSPI_SShift_NoShift;//采样偏移
	QSPI_InitStructure.QSPI_Prescaler = 3;			           		//QSPI 预分频器值
	QSPI_InitStructure.QSPI_FSize     = 0x18;  /*1M Byte*/         	//闪存大小
	QSPI_InitStructure.QSPI_CKMode    = QSPI_CKMode_Mode0;         //时钟模式
	QSPI_InitStructure.QSPI_CSHTime   = QSPI_CSHTime_2Cycle;       //芯片选择高时间
	QSPI_Init(&QSPI_InitStructure);
	
	QSPI_ITConfig(QSPI_IT_TO| QSPI_IT_TC | QSPI_IT_TE,ENABLE);

	NVIC_InitStruct.NVIC_IRQChannel = QUADSPI_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);

	QSPI_SetFIFOThreshold(0);
	
	QSPI_Cmd(ENABLE);

	PRINTF_LOG("Flash Init\r\n");
	
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_DHHC        = QSPI_ComConfig_DHHC_Enable;//DDR模式下数据输出延时使能
	QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;//仅发送一次指令模块使能
}

void QUADSPI_IRQHandler(void)
{
	if(QSPI_GetITStatus(QSPI_IT_TO))
	{
		QSPI_ClearITPendingBit(QSPI_IT_TO);
		QSPI_SR_FLAG |= QUADSPI_SR_TOF;
	}
	if(QSPI_GetITStatus(QSPI_IT_SM))
	{
		QSPI_ClearITPendingBit(QSPI_IT_SM);
		QSPI_SR_FLAG |= QUADSPI_SR_SMF;
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
		QSPI_SR_FLAG |= QUADSPI_SR_TCF;
	}
	if(QSPI_GetITStatus(QSPI_IT_TE))
	{
		QSPI_ClearITPendingBit(QSPI_IT_TE);
		QSPI_SR_FLAG |= QUADSPI_SR_TEF;
	}
}
