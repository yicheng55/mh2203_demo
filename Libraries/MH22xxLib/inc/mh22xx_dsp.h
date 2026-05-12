/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MH22xx_DSP_H
#define __MH22xx_DSP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mh22xx.h"


/** @defgroup DSP_CommandDefinition 
  * @{
  */
#define DSP_LOAD     1
#define DSP_LOAD_M   2
#define DSP_STR_M    3
#define DSP_EVINT    4
#define DSP_DONE     5
#define DSP_LOAD_R   6
#define DSP_GOTO     7
#define DSP_CMP      8
#define DSP_CMP_EQ   9
#define DSP_ADD_R    10
#define DSP_SUB_R    11
#define DSP_LOAD_M_A 14
#define DSP_SUNPEND  15

#define DSP_F_ADD   16
#define DSP_F_SUB   17
#define DSP_F_MUL   18
#define DSP_F_DIV   19
#define DSP_F_SQRT  20
#define DSP_F_I2F   21
#define DSP_F_F2I   22
#define DSP_F_CMP     23
#define DSP_F_CMP_EQ  24

/** @defgroup DSP_GeneralRegistersDefinition 
  * @{
  */
#define  DSP_R0    0
#define  DSP_R1    1
#define  DSP_R2    2
#define  DSP_R3    3
#define  DSP_R4    4
#define  DSP_R5    5
#define  DSP_R6    6
#define  DSP_R7    7

/** @defgroup DSP_FlagsDefinition 
  * @{
  */
#define DSP_FLAG_RUN           ((uint32_t)(1U << 31U))
#define DSP_FLAG_DONE          ((uint32_t)(1U << 30U))
#define DSP_FLAG_EVINT         ((uint32_t)(1U << 29U))
#define DSP_FLAG_ERR           ((uint32_t)(1U << 28U))
#define DSP_FLAG_SUSPEND       ((uint32_t)(1U << 27U))
#define IS_DSP_FLAG(FLAG) 	   (((FLAG) == DSP_FLAG_RUN)   || ((FLAG) == DSP_FLAG_DONE) || \
                               ((FLAG) == DSP_FLAG_EVINT)  || ((FLAG) == DSP_FLAG_ERR)  || \
                               ((FLAG) == DSP_FLAG_SUSPEND))
							   
/** @defgroup DSP_SpaceModeDefinition 
  * @{
  */
#define DSP_SPACE_MODE_32K        ((uint8_t)1U)
#define DSP_SPACE_MODE_1K         ((uint8_t)0U)


#define DSP_ROUND_EVEN            ((uint32_t)0)
#define DSP_ROUND_ZERO            ((uint32_t)1)
#define DSP_ROUND_POS_INFINITY    ((uint32_t)2)
#define DSP_ROUND_NEG_INFINITY    ((uint32_t)3)
#define DSP_ROUND_UP              ((uint32_t)4)
#define DSP_ROUND_AWAY            ((uint32_t)5)
#define IS_DSP_RND_MODE(MODE) (((MODE) == DSP_ROUND_EVEN)          || ((MODE) == DSP_ROUND_ZERO)          || \
                               ((MODE) == DSP_ROUND_POS_INFINITY)  || ((MODE) == DSP_ROUND_NEG_INFINITY)  || \
                               ((MODE) == DSP_ROUND_UP)            || ((MODE) == DSP_ROUND_AWAY))

/** @defgroup DSP_InterruptsDefinition 
  * @{
  */
#define DSP_IT_ERR             ((uint32_t)(1U << 20U))
#define DSP_IT_EVINT           ((uint32_t)(1U << 19U))
#define DSP_IT_DONE            ((uint32_t)(1U << 18U))
#define IS_DSP_IT(IT) (((IT) == DSP_IT_ERR) || ((IT) == DSP_IT_EVINT) || \
                       ((IT) == DSP_IT_DONE))

/* Initialization and Configuration functions *********************************/
void DSP_Space_Mode_Config(uint8_t Mode);
void DSP_ITConfig(uint32_t DSP_IT, FunctionalState NewState);
FlagStatus DSP_GetFlagStatus(uint32_t dsp_flag);
void DSP_ClearFlag(uint32_t dsp_flag);
void DSP_Start(uint16_t start_addr);
void DSP_Stop(void);
void DSP_DMA_CMD(FunctionalState NewState);
void DSP_RESUME(void);
void DSP_Debug_CMD(FunctionalState NewState);
void DSP_Debug_Step(void);
void DSP_DMA_AutoResume(FunctionalState NewState);


#endif

