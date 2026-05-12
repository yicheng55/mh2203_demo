#ifndef __PSRAM_H_
#define __PSRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include "mh22xx.h"
#include <stdint.h>

#define PRINTF_LOG 	printf

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
}QSPI_CMD_t;

typedef enum
{
	/* Reset Operations */
	QSPI_CMD_RESET_ENABLE = 0,
	QSPI_CMD_RESET_MEMORY,
	/* Identification Operations */
	QSPI_CMD_READ_ID,
	/* Read Operations */
	QSPI_CMD_QUAD_INOUT_FAST_READ,
	/* Program Operations */
	QSPI_CMD_QUAD_PROG,
	/* QPI Operations */
	QSPI_CMD_ENTER_QUAD_MODE,
	QSPI_CMD_EXIT_QUAD_MODE,
	QSPI_CMD_MAX,
}QSPI_CMD_e;

/*********************************************************************/
#define QSPIx_CLK_PIN						GPIO_Pin_2
#define QSPIx_CLK_GPIO_PORT					GPIOB
#define QSPIx_CLK_GPIO_CLK					RCC_APB2Periph_GPIOB

#define QSPIx_BANK1_CS_PIN					GPIO_Pin_10
#define QSPIx_BANK1_CS_GPIO_PORT			GPIOB
#define QSPIx_BANK1_CS_GPIO_CLK				RCC_APB2Periph_GPIOB

#define QSPIx_BANK1_D0_PIN					GPIO_Pin_6
#define QSPIx_BANK1_D0_GPIO_PORT			GPIOB
#define QSPIx_BANK1_D0_GPIO_CLK				RCC_APB2Periph_GPIOB

#define QSPIx_BANK1_D1_PIN					GPIO_Pin_5
#define QSPIx_BANK1_D1_GPIO_PORT			GPIOB
#define QSPIx_BANK1_D1_GPIO_CLK				RCC_APB2Periph_GPIOB

#define QSPIx_BANK1_D2_PIN					GPIO_Pin_4
#define QSPIx_BANK1_D2_GPIO_PORT			GPIOB
#define QSPIx_BANK1_D2_GPIO_CLK				RCC_APB2Periph_GPIOB

#define QSPIx_BANK1_D3_PIN					GPIO_Pin_3
#define QSPIx_BANK1_D3_GPIO_PORT			GPIOB
#define QSPIx_BANK1_D3_GPIO_CLK				RCC_APB2Periph_GPIOB

#define QSPI_XIP_ADDR						0xB1000000

#define QSPI_DMA_CHANNEL					DMA1_Channel1
#define QSPI_DMA_FLAG_TC					DMA1_FLAG_TC1

#define QSPI_M2M_DMA_CHANNEL				DMA1_Channel2
#define QSPI_M2M_DMA_FLAG_TC				DMA1_FLAG_TC2


/***************************CMD**************************************/
/* Reset Operations */
#define RESET_ENABLE_CMD							0x66
#define RESET_MEMORY_CMD							0x99

/* Identification Operations */
#define READ_ID_CMD											0x9F

#define QUAD_INOUT_FAST_READ_CMD				0xEB
#define QUAD_PROG_CMD										0x38
#define QUAD_ENTER_QUAD_MODE						0x35
#define QUAD_EXIT_QUAD_MODE							0xF5

void QSPI_Configuration(void);
void PSRANM_Configuration(void);
void PSRANM_Test(void);
#endif
