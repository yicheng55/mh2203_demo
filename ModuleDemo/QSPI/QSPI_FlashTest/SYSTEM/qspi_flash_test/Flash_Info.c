#include "Flash_Info.h"
#include "mh22xx.h"


Flash_Info_t Flash_Info[10] = 
{
	{"MXIC",MANUFATOR_MXIC},
	{"Zbit",MANUFATOR_ZBIT},
	{"XMC",MANUFATOR_XMC},
	{"PUYA",MANUFATOR_PUYA},
	{"GD",MANUFATOR_GD},
	{"FM",MANUFATOR_FM},
	{"XTX",MANUFATOR_XTX},
	{"WINBOND",MANUFATOR_WINBOND},
	{"BG",MANUFATOR_BG},
	{"EON",MANUFATOR_EON},
};



Flash_CMD_t Flash_Cmd[FLASH_CMD_MAX] = 
{
	/* Reset Operations */
	{RESET_ENABLE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	{RESET_MEMORY_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Write Operations */
	{WRITE_ENABLE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	{WRITE_DISABLE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Register Operations */
	{READ_STATUS_REG_CMD1, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{WRITE_STATUS_REG_CMD1, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{READ_STATUS_REG_CMD2, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{WRITE_STATUS_REG_CMD2, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{READ_STATUS_REG_CMD3, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{WRITE_STATUS_REG_CMD3, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Identification Operations */
	{READ_ID_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{READ_ID_CMD2, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Low Power Mode */
	{WAKEUP_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 24},
	{DEEP_SLEEP_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	/* 4-byte Address Mode Operations */
	{ENTER_4_BYTE_ADDR_MODE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	{EXIT_4_BYTE_ADDR_MODE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Read Operations */
	{READ_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	{FAST_READ_DTR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Enable, 6},//可32bit地址
	{READ_4_BYTE_ADDR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_32bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},
	{FAST_READ_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 8},//可32bit地址//dummy可配且不同情况不一致
	{FAST_READ_4_BYTE_ADDR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_32bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 8},///dummy可配且不同情况不一致
	{DUAL_OUT_FAST_READ_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_2Line, QSPI_ComConfig_DDRMode_Disable, 8},//可32bit地址//dummy可配且不同情况不一致
	{DUAL_INOUT_FAST_READ_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_2Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_2Line, QSPI_ComConfig_DDRMode_Disable, 4},//可32bit地址//dummy可配且不同情况不一致
	{DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_2Line, QSPI_ComConfig_ADSize_32bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_2Line, QSPI_ComConfig_DDRMode_Disable, 4},//dummy可配且不同情况不一致
	{QUAD_OUT_FAST_READ_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 8},//可32bit地址//dummy可配且不同情况不一致
	{QUAD_INOUT_FAST_READ_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 6},//可32bit地址//dummy可配且不同情况不一致
	{QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_32bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 6},//dummy可配且不同情况不一致
	{QUAD_INOUT_FAST_READ_DTR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Enable, 6},//dummy可配且不同情况不一致
	{QUAD_INOUT_FAST_READ_4_BYTE_ADDR_DDR_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Read, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_32bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Enable, 6},//dummy可配且不同情况不一致
	/* Erase Operations */
	{SECTOR_ERASE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	{BLOCK32_ERASE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	{BLOCK_ERASE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	{CHIP_ERASE_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	/* Program Operations */
	{PAGE_PROG_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_1Line, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	{QUAD_IN_FAST_PROG_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_1Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	{QUAD_PROG_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_4Line, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_4Line, QSPI_ComConfig_DDRMode_Disable, 0},//可32bit地址
	
	/* QPI Operations */
	{MXIC_ENTER_QUAD_CMD, QSPI_ComConfig_IMode_1Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
	{MXIC_EXIT_QUAD_CMD, QSPI_ComConfig_IMode_4Line, QSPI_ComConfig_FMode_Indirect_Write, QSPI_ComConfig_ADMode_NoAddress, QSPI_ComConfig_ADSize_24bit
		,QSPI_ComConfig_ABMode_NoAlternateByte, QSPI_ComConfig_ABSize_8bit, QSPI_ComConfig_DMode_NoData, QSPI_ComConfig_DDRMode_Disable, 0},
};


