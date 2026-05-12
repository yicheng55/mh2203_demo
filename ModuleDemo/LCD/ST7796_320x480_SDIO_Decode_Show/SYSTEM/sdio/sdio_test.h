#ifndef __SDIO_TEST_H
#define __SDIO_TEST_H

#include "mh22xx.h"
#include "delay.h"
#include <stdio.h>

#define TESTDEBUG 		printf

#define DMA_MODE			2
#define POLLING_MODE		1

#define COUNT_TEST  1000 //测试次数

void SD_EraseTest(void);
void SD_SingleBlockTest(uint8_t tradatamode);
void SD_MultiBlockTest(void);

extern uint32_t CountFail;

typedef struct
{
	u32 Addr;//寄存器地址
	u32 ResetVal;//寄存器默认值
	u32 mask;//寄存器屏蔽位（保留位）
	u8 UnlockEn;//私有寄存器是否需要解锁（1级锁  3级锁）  0:不解锁  1：解锁
	u8 KeyEn;//某些寄存器需要特定位解锁  0：不需要解锁    1：高16位写0xA55A解锁     2：BIT31写A，同时BIT15写5解锁
}PrivaRegistTypeDef;

#endif


/*****************************END OF FILE**************************/
