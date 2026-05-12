/**
  ******************************************************************************
  * @file    mh22xx_it.c 
  * @author  NONE
  * @version NONE
  * @date    NONE
  * @brief   NONE
  ******************************************************************************/
#include "mh22xx_it.h" 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


 
void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 mh22xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_mh22xx_xx.s).                                            */
/******************************************************************************/
void RNG_IRQHandler(void)
{
    if (TRNG_GetITStatus(TRNG_IT_RNG0_S128) == SET){
        printf("Rng : %08X %08X %08X %08X \r\n",TRNG->RNG_DATA,TRNG->RNG_DATA,TRNG->RNG_DATA,TRNG->RNG_DATA);
        TRNG_ClearITPendingBit(TRNG_IT_RNG0_S128);
    }
    if (TRNG_GetITStatus(TRNG_IT_RNG0_ATTACK) == SET){
        TRNG_ClearITPendingBit(TRNG_IT_RNG0_ATTACK);
    }
    NVIC_ClearPendingIRQ(TRNG_IRQn);
}
