/*
 * Header file: trans_level2.h
 * Created 20240628 Joseph
 */
#ifndef __TRANS_LEVEL2_H__
#define __TRANS_LEVEL2_H

/* Private but updated define ------------------------------------------------------------*/

//Example Size: Flash page sizes in microcontrollers can vary but are often in the range of 1 KB to 4 KB. 
//				For example, some STM32 microcontrollers (related to AT32 series) 
//				have a flash page size of 2 KB.

#undef StartAddress
#undef EndAddress
#define StartAddress  ((uint32_t)0x08020000 - (1024 * 2))
#define EndAddress 		((uint32_t)0x08020000)

#define StartAddress2  ((uint32_t)0x08020000 - (1024 * 4))

#endif //__TRANS_LEVEL2_H__
