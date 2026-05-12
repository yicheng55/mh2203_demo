#ifndef __FLASH_INFO_H_
#define __FLASH_INFO_H_

#include <stdint.h>

typedef struct flash_info
{
	const char *manufacture;
	const uint32_t id;
}Flash_Info_t;

typedef struct flash_cmd
{
	uint8_t Instruction;
	uint32_t Instruction_Mode;
	uint32_t Functional_Mode;
	uint32_t Address_Mode;
	uint32_t Address_Size;
	uint32_t Alternate_Bytes_Mode;
	uint32_t Alternate_Bytes_Size;
	uint32_t Data_Mode;
	uint32_t DoubleDataRateMode;
	uint32_t Dummy_Cycles;
}Flash_CMD_t;


typedef enum
{
	/* Reset Operations */
	FLASH_CMD_RESET_ENABLE = 0,
	FLASH_CMD_RESET_MEMORY,
	/* Write Operations */
	FLASH_CMD_WRITE_ENABLE,
	FLASH_CMD_WRITE_DISABLE,
	/* Register Operations */
	FLASH_CMD_READ_STATUS1,
	FLASH_CMD_WRITE_STATUS1,
	FLASH_CMD_READ_STATUS2,
	FLASH_CMD_WRITE_STATUS2,
	FLASH_CMD_READ_STATUS3,
	FLASH_CMD_WRITE_STATUS3,
	/* Identification Operations */
	FLASH_CMD_READ_ID,
	FLASH_CMD_READ_ID2,
	/* Low Power Mode */
	FLASH_CMD_WAKEUP,
	FLASH_CMD_DEEP_SLEEP,
	/* 4-byte Address Mode Operations */
	FLASH_CMD_ENTER_4_BYTE_ADDR,
	FLASH_CMD_EXIT_4_BYTE_ADDR,
	/* Read Operations */
	FLASH_CMD_READ,
	FLASH_CMD_FAST_READ_DTR,
	FLASH_CMD_READ_4_BYTE_ADDR,
	FLASH_CMD_FAST_READ,
	FLASH_CMD_FAST_READ_4_BYTE_ADDR,
	FLASH_CMD_DUAL_OUT_FAST_READ,
	FLASH_CMD_DUAL_INOUT_FAST_READ,
	FLASH_CMD_DUAL_INOUT_FAST_READ_4_BYTE_ADDR,
	FLASH_CMD_QUAD_OUT_FAST_READ,
	FLASH_CMD_QUAD_INOUT_FAST_READ,
	FLASH_CMD_QUAD_INOUT_FAST_READ_4_BYTE_ADDR,
	FLASH_CMD_QUAD_INOUT_FAST_READ_DTR,
	FLASH_CMD_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_DDR,
	/* Erase Operations */
	FLASH_CMD_SECTOR_ERASE,
	FLASH_CMD_BLOCK32_ERASE,
	FLASH_CMD_BLOCK_ERASE,
	FLASH_CMD_CHIP_ERASE,
	/* Program Operations */
	FLASH_CMD_PAGE_PROG,
	FLASH_CMD_QUAD_IN_FAST_PROG,
	FLASH_CMD_QUAD_PROG,
	/* QPI Operations */
	FLASH_CMD_ENTER_QUAD_CMD,
	FLASH_CMD_EXIT_QUAD_CMD,
	FLASH_CMD_MAX,
	
}Flash_CMD_e;



#define MANUFATOR_MXIC								0xC2
#define MANUFATOR_ZBIT								0x5E
#define MANUFATOR_XMC								0x20
#define MANUFATOR_PUYA								0x85
#define MANUFATOR_GD								0xC8
#define MANUFATOR_FM								0xA1
#define MANUFATOR_XTX								0x0B
#define MANUFATOR_WINBOND							0xEF
#define MANUFATOR_BG								0xE0
#define MANUFATOR_EON								0x1C


/***************************FLASH CMD**************************************/
/* Reset Operations */
#define RESET_ENABLE_CMD							0x66
#define RESET_MEMORY_CMD							0x99

/* Identification Operations */
#define READ_ID_CMD									0x90
#define READ_ID_CMD2								0x9F
#define MULTIPLE_IO_READ_ID_CMD						0xAF
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD			0x5A

/* Low Power Mode */
#define WAKEUP_CMD									0xAB
#define DEEP_SLEEP_CMD								0xB9

/* Read Operations */
#define READ_CMD									0x03
#define READ_4_BYTE_ADDR_CMD						0x13

#define FAST_READ_CMD								0x0B
#define FAST_READ_DTR_CMD							0x0D
#define FAST_READ_4_BYTE_ADDR_CMD					0x0C

#define DUAL_OUT_FAST_READ_CMD						0x3B
#define DUAL_OUT_FAST_READ_DTR_CMD					0x3D
#define DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD			0x3C

#define DUAL_INOUT_FAST_READ_CMD					0xBB
#define DUAL_INOUT_FAST_READ_DTR_CMD				0xBD
#define DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD		0xBC

#define QUAD_OUT_FAST_READ_CMD						0x6B
#define QUAD_OUT_FAST_READ_DTR_CMD					0x6D
#define QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD			0x6C

#define QUAD_INOUT_FAST_READ_CMD					0xEB
#define QUAD_INOUT_FAST_READ_DTR_CMD				0xED
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD		0xEC
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_DDR_CMD	0xEE

/* Write Operations */
#define WRITE_ENABLE_CMD							0x06
#define WRITE_DISABLE_CMD							0x04

/* Register Operations */
#define READ_STATUS_REG_CMD1						0x05
#define WRITE_STATUS_REG_CMD1						0x01

#define READ_STATUS_REG_CMD2						0x35
#define WRITE_STATUS_REG_CMD2						0x31

#define READ_STATUS_REG_CMD3						0x15
#define WRITE_STATUS_REG_CMD3						0x11

#define READ_LOCK_REG_CMD							0xE8
#define WRITE_LOCK_REG_CMD							0xE5

#define READ_FLAG_STATUS_REG_CMD					0x70
#define CLEAR_FLAG_STATUS_REG_CMD					0x50

#define READ_NONVOL_CFG_REG_CMD						0xB5
#define WRITE_NONVOL_CFG_REG_CMD					0xB1

#define READ_VOL_CFG_REG_CMD						0x85
#define WRITE_VOL_CFG_REG_CMD						0x81

#define READ_ENHANCED_VOL_CFG_REG_CMD				0x65
#define WRITE_ENHANCED_VOL_CFG_REG_CMD				0x61

#define READ_EXT_ADDR_REG_CMD						0xC8
#define WRITE_EXT_ADDR_REG_CMD						0xC5

/* Program Operations */
#define PAGE_PROG_CMD								0x02
#define PAGE_PROG_4_BYTE_ADDR_CMD					0x12

#define DUAL_IN_FAST_PROG_CMD						0xA2
#define EXT_DUAL_IN_FAST_PROG_CMD					0xD2

#define QUAD_PROG_CMD								0x38
#define QUAD_IN_FAST_PROG_CMD						0x32
#define EXT_QUAD_IN_FAST_PROG_CMD					0x12
#define QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD			0x34

/* Erase Operations */
#define SECTOR_ERASE_CMD							0x20
#define SUBSECTOR_ERASE_4_BYTE_ADDR_CMD				0x21

#define BLOCK_ERASE_CMD								0xD8
#define BLOCK32_ERASE_CMD							0x52
#define SECTOR_ERASE_4_BYTE_ADDR_CMD				0xDC

#define CHIP_ERASE_CMD								0xC7

#define PROG_ERASE_RESUME_CMD						0x7A
#define PROG_ERASE_SUSPEND_CMD						0x75

/* One-Time Programmable Operations */
#define READ_OTP_ARRAY_CMD							0x4B
#define PROG_OTP_ARRAY_CMD							0x42

/* 4-byte Address Mode Operations */
#define ENTER_4_BYTE_ADDR_MODE_CMD					0xB7
#define EXIT_4_BYTE_ADDR_MODE_CMD					0xE9

/* Quad Operations */
#define MXIC_ENTER_QUAD_CMD							0x35
#define MXIC_EXIT_QUAD_CMD							0xF5

#define ZBIT_ENTER_QUAD_CMD							0x38
#define ZBIT_EXIT_QUAD_CMD							0xFF
/*****************************************************************/


extern Flash_Info_t Flash_Info[10];
extern Flash_CMD_t Flash_Cmd[FLASH_CMD_MAX];






#endif
