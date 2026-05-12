#include "mh22xx_dsp.h"

///*
#define __MH_FPU_ADD
#define __MH_FPU_SUB
#define __MH_FPU_MUL
#define __MH_FPU_DIV
#define __MH_FPU_F2I
#define __MH_FPU_I2F
#define __MH_FPU_CMP
//*/

/* FPU支持的算法单元函数 **************************************/
#ifdef __MH_FPU_ADD
float __aeabi_fadd(float a, float b) {
	__disable_irq();
	
	DSP->F_ADD_A = a;
	DSP->F_ADD_B = b;
	
	__enable_irq();
	return (DSP->F_ADD_D);
}
#endif

#ifdef __MH_FPU_SUB
float __aeabi_fsub(float a, float b) {
	__disable_irq();
	
	DSP->F_SUB_A = a;
	DSP->F_SUB_B = b;

	__enable_irq();
	return (DSP->F_SUB_D);
}

float __aeabi_frsub(float a, float b) {
	__disable_irq();
	
	DSP->F_SUB_A = b;
	DSP->F_SUB_B = a;
	
	__enable_irq();	
	return (DSP->F_SUB_D);
}
#endif

#ifdef __MH_FPU_MUL
float __aeabi_fmul(float a, float b) {
	__disable_irq();	
	
	DSP->F_MUL_A = a;
	DSP->F_MUL_B = b;
	
	__enable_irq();		
	return (DSP->F_MUL_D);
}
#endif

#ifdef __MH_FPU_DIV
float __aeabi_fdiv(float a, float b) {
	__disable_irq();	
	
	DSP->F_DIV_A = a;
	DSP->F_DIV_B = b;
	
	__enable_irq();		
	return (DSP->F_DIV_D);
}
#endif

#ifdef __MH_FPU_F2I
int32_t __aeabi_f2i(float a) {	
	__disable_irq();	

	DSP->F_F2I_A = a;

	__enable_irq();		
	return (DSP->F_F2I_D);
}
#endif

#ifdef __MH_FPU_I2F
float __aeabi_i2f(int32_t a) {
	__disable_irq();	
	
	DSP->F_I2F_A = a;
	
	__enable_irq();			
	return (DSP->F_I2F_D);
}
#endif

#ifdef __MH_FPU_CMP
void __aeabi_cfrcmple(float a, float b) {
	__disable_irq();	
	
	DSP->F_CMP_A = b;	
	DSP->F_CMP_B = a;	
		
	if ((DSP->F_CMP_D & 0x1)) {
		__ASM{CMP 0, 0};
	}
	else if ((DSP->F_CMP_D & 0x2)){
		__ASM{CMP 1, 2};
	}
	else 
		__ASM{CMP 2, 1};	
		
	__enable_irq();				
}
#endif


/* DSP模块的初始化函数 **************************************/
/*
  * @brief 配置DSP中断;
  * @param DSP_IT: 要配置的中断.
  * @param	NewState: new state of the specified DSP interrupts . This parameter can be: ENABLE or DISABLE.

*/
void DSP_ITConfig(uint32_t DSP_IT, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_DSP_IT(DSP_IT));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
	
  if (NewState != DISABLE)
  {
    /* Perform Byte access to RCC_CIR bits to enable the selected interrupts */
    DSP->CR |= DSP_IT;
  }
  else
  {
    /* Perform Byte access to RCC_CIR bits to disable the selected interrupts */
    DSP->CR &= (uint8_t)~DSP_IT;
  }
}

/*
  * @brief 读取DSP的标志;
  * @param dsp_flag: 要读取的标志位.
  * @retval The new state of RCC_FLAG (SET or RESET).
*/
FlagStatus DSP_GetFlagStatus(uint32_t dsp_flag)
{
  uint32_t statusreg = 0;
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_DSP_FLAG(dsp_flag));

  /* Get the RCC register index */
  statusreg = DSP->CR;

  /* Get the flag position */
  if ((statusreg & dsp_flag) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }

  /* Return the flag status */
  return bitstatus;
}

/*
  * @brief 清除标志；
  * @param dsp_flag: 要清除的标志位, 写1清除；
  * @retval None.
*/
void DSP_ClearFlag(uint32_t dsp_flag)
{	
  /* Check the parameters */
  assert_param(IS_DSP_FLAG(dsp_flag));
  DSP->CR |= dsp_flag;
}

/*
  * @brief 启动DSP的函数
  * @param start_addr DSP指令的启动地址.
  * @retval None.
*/
void DSP_Start(uint16_t start_addr)
{
	DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP);  // 清除了START 和 STOP控制位
	DSP->CR &=  ~(0xFFFF);
	
	DSP->CR |=  (start_addr >> 2);
	DSP->CR |=  (DSP_CR_START);
}

/*
  * @brief 停止DSP的函数.
  * @param none.
  * @retval None.
*/
void DSP_Stop(void)
{
	DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP);  // 清除了START 和 STOP控制位
	DSP->CR &=  ~(0xFFFF);
	
	DSP->CR |=  (DSP_CR_STOP);
}

/*
  * @brief 	DMA使能的函数
  * @param	NewState: new state of the DMA mode. This parameter can be: ENABLE or DISABLE.
  * @retval None.
*/
void DSP_DMA_CMD(FunctionalState NewState)
{
    if (NewState != DISABLE) {
		DSP->CR |=  DSP_CR_DMA;
	}
	else {
		DSP->CR &=  ~DSP_CR_DMA;
	}	
}

/*
  * @brief DMA自动恢复
  * @param	NewState: new state of the auto resume mode. This parameter can be: ENABLE or DISABLE.
  * @retval None.
*/
void DSP_DMA_AutoResume(FunctionalState NewState)
{
    if (NewState != DISABLE) {
		DSP->CR |=  DSP_CR_DMA_AUTO_RESUME;
	}
	else {
		DSP->CR &=  ~DSP_CR_DMA_AUTO_RESUME;
	}	
}

/*
  * @brief DSP恢复的函数
  * @param none.
  * @retval None.
*/
void DSP_RESUME(void)
{
	DSP->CR |=  DSP_CR_RESUME;	
}

/*
  * @brief 使能DSP调试模式。
  * @note 在DSP_CR寄存器的start_dsp bit置位之前使能，即进入DEBUG模式。
		  DEBUG模式，是调试时单步执行的模式。
  * @param	NewState: new state of the debug mode. This parameter can be: ENABLE or DISABLE.
  * @retval None
*/
void DSP_Debug_CMD(FunctionalState NewState)
{
	if (NewState != DISABLE) {
		DSP->CR |=  DSP_CR_DEBUG_EN;
	}
	else {
		DSP->CR &=  ~DSP_CR_DEBUG_EN;  
	}
}

/*
  * @brief DSP单步执行的函数。
  * @note 函数每执行一次，DSP的指令执行一次。
  * @param none.
  * @retval None
*/
void DSP_Debug_Step(void)
{
	DSP->CR |=  DSP_CR_DEBUG_STEP;
}

/*
  * @brief 精确度计数保留配置。
  * @param mode_sel: specifies the rounding mode. 
  *   this parameter can be one of the following values:
  *   @arg DSP_ROUND_EVEN: IEEE round to nearest(even).     
  *   @arg DSP_ROUND_ZERO: IEEE round to zero.        
  *   @arg DSP_ROUND_POS_INFINITY: IEEE round to positive infinity.        
  *   @arg DSP_ROUND_NEG_INFINITY: IEEE round to negative infinity.        
  *   @arg DSP_ROUND_UP: round to nearest up.                
  *   @arg DSP_ROUND_AWAY: round away from zero.                       
  * @retval None
*/
void DSP_Round_Mode(uint32_t mode_sel)
{
	/* Check the parameters */
    assert_param(IS_DSP_RND_MODE(mode_sel));

	DSP->CR2 &=  ~DSP_CR2_F_RND;	
	DSP->CR2 |=  DSP_CR2_F_RND & mode_sel;
}


/*
  * @brief DSP模式配置，可以选择32KB模式，也可以选择1KB模式。
  * @param Mode: 可选择DSP空间模式。
  *   this parameter can be one of the following values:
  *   @arg DSP_SPACE_MODE_32K: 32KB模式，DSP可以使用的SRAM空间是32KB，地址范围是0x20008000~0x2000FFFF.
  *   @arg DSP_SPACE_MODE_1K:   1KB模式，DSP可以使用的SRAM空间是1KB，地址范围是0x20018000~0x200183FF.
  * @retval None
*/
void DSP_Space_Mode_Config(uint8_t Mode)
{
	if(Mode != 0) {
		DSP->CR2 |= DSP_CR2_S_ADDR_32K;
	}
	else {
		DSP->CR2 &= ~DSP_CR2_S_ADDR_32K;
	}
}



