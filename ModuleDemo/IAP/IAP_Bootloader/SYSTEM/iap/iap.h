#ifndef __IAP_H__
#define __IAP_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "mh22xx.h"

typedef  void (*iapfun)(void);				//定义一个函数类型的参数.

#define FLASH_APP1_ADDR		0x08008000  	//第一个应用程序起始地址(存放在FLASH)
											//保留0X08000000~0X08007FFF的空间为IAP使用(32K)
											
#define MH2103_FLASH_BASE 	0x08000000 		//FLASH的起始地址											
#define FLASH_SECTOR_SIZE 	(2048) 			//字节

void IAP_Write_Appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t appsize);
void IAP_Load_App(uint32_t appxaddr);
#endif
