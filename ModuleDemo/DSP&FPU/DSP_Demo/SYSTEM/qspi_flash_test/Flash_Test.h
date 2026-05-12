#ifndef __FLASH_TEST_H_
#define __FLASH_TEST_H_

#include <stdio.h>
#include <stdlib.h>
#include "mh210x.h"
#include "Flash_Info.h"

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

void QSPI_Configuration(void);
void Flash_Init(void);
void Flash_Indirect_Test(void);
#endif
