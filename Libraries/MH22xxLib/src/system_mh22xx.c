/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup mh22xx_system
  * @{
  */  

#include "mh22xx.h"

/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */ 
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x0

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

uint32_t SystemCoreClock = HSI_VALUE;
/**
  * @}
  */

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface, the PLL and update the 
  *         SystemCoreClock variable.
  * @note   This function should be used only after reset.
  * @param  None
  * @retval None
  */
void SystemInit (void)
{
  /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  RCC->CFGR &= (uint32_t)0xF8FF0000; 
  
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;

  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000;

#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif 
}

/**
  * @}
  */
  
/**
  * @}
  */    
/******************* (C) COPYRIGHT  MHSEMICONDUCTOR *****END OF FILE****/
