#ifndef __mh22xx_DSP_ALGORITHM_LIB_H
#define __mh22xx_DSP_ALGORITHM_LIB_H

#include "mh22xx_dsp.h"
#include "math.h"


#define RAM32K_MODE  (1)  // 1:32K mode; 0:1K mode;

#if RAM32K_MODE

#define DSP_BASE_ADDR  (0x20008000U)	// DSP base 地址

#define 	INPUT_ADDR   		  (0x0U)	
#define 	OUTPUT_ADDR 		  (INPUT_ADDR		  + 4U)	
#define 	NEG_CONS_CONS_ADDR    (OUTPUT_ADDR		  + 4U)	
#define 	MINI_CONS_ADDR  	  (NEG_CONS_CONS_ADDR + 4U)	
#define 	FRAC_CONS_ADDR    	  (MINI_CONS_ADDR     + 4U)	// Factorial = 1;
#define 	I_CONS_ADDR  		  (FRAC_CONS_ADDR     + 4U)	// i         = 1;
#define 	CONS0_ADDR  		  (I_CONS_ADDR   	  + 4U)	// Negation  = 1;
#define 	CONS1_ADDR  		  (CONS0_ADDR    	  + 4U)	// 阶乘常量1  = 1;
#define 	CONS2_ADDR  		  (CONS1_ADDR    	  + 4U) // 阶乘常量2  = 2;	
#define 	END_ADDR_SIN_MEM	  (CONS2_ADDR    	  + 4U) // 正弦函数所用MEM空间结束地址;	

#define 	START_SIN_FUNC_ADDR	  (0x400U*4)	// 函数起始地址


/*
	SUM 地址
*/
#define INPUT_ADDR_PRT         (END_ADDR_SIN_MEM +     4U)  // 数组首地址
#define INPUT_ADDR_NUM         (INPUT_ADDR_PRT    +255*4U)  // 数组长度
#define OUTPUT_ADDR_SUM_FUNC   (INPUT_ADDR_NUM		 + 4U)	// 输出地址
#define CONS_ADDR_VAL_0  	   (OUTPUT_ADDR_SUM_FUNC + 4U)  // 常量地址
#define CONS_ADDR_VAL_1   	   (CONS_ADDR_VAL_0		 + 4U)  // 常量地址
#define VAR_ADDR_1       	   (CONS_ADDR_VAL_1 	 + 4U)  // 变量地址,存储的是输入数组的地址
#define END_ADDR_SUM_MEM	   (VAR_ADDR_1    	     + 4U)  // SUM函数所用MEM空间结束地址;	

#define START_SUM_FUNC_ADDR	   (START_SIN_FUNC_ADDR + 0x400)	// SUM函数起始地址

/*
	SUB 地址
*/
#define INPUT_ADDR_PRT_SUB     (END_ADDR_SUM_MEM      +4U)   // 数组首地址
#define INPUT_ADDR_NUM_SUB     (INPUT_ADDR_PRT_SUB+255*4U)   // 数组长度
#define OUTPUT_ADDR_SUB_FUNC   (INPUT_ADDR_NUM_SUB   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_SUB    (OUTPUT_ADDR_SUB_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_SUB    (CONS_ADDR_VAL_0_SUB  + 4U)   // 常量地址
#define VAR_ADDR_1_SUB     	   (CONS_ADDR_VAL_1_SUB  + 4U)   // 变量地址,存储的是输入数组的地址
#define END_ADDR_SUB_MEM	   (VAR_ADDR_1_SUB    	 + 4U)   // SUB函数所用MEM空间结束地址;	

#define START_SUB_FUNC_ADDR	   (START_SUM_FUNC_ADDR+0x400)	// SUB函数起始地址


/*
	Product 地址
*/
#define INPUT_ADDR_PRT_PRO     (END_ADDR_SUB_MEM      +4U)   // 数组首地址
#define INPUT_ADDR_NUM_PRO     (INPUT_ADDR_PRT_PRO+255*4U)   // 数组长度
#define OUTPUT_ADDR_PRO_FUNC   (INPUT_ADDR_NUM_PRO   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_PRO    (OUTPUT_ADDR_PRO_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_PRO    (CONS_ADDR_VAL_0_PRO  + 4U)   // 常量地址
#define VAR_ADDR_1_PRO     	   (CONS_ADDR_VAL_1_PRO  + 4U)   // 变量地址,存储的是输入数组的地址
#define END_ADDR_RPO_MEM	   (VAR_ADDR_1_PRO    	 + 4U)   // Product函数所用MEM空间结束地址;	

#define START_PRO_FUNC_ADDR	   (START_SUB_FUNC_ADDR+0x400)	 // Product函数起始地址


/*
	SUMSQ 地址
*/
#define INPUT_ADDR_PRT_SUMSQ     (END_ADDR_RPO_MEM      +4U)     // 数组首地址
#define INPUT_ADDR_NUM_SUMSQ     (INPUT_ADDR_PRT_SUMSQ+255*4U)   // 数组长度
#define OUTPUT_ADDR_SUMSQ_FUNC   (INPUT_ADDR_NUM_SUMSQ   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_SUMSQ    (OUTPUT_ADDR_SUMSQ_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_SUMSQ    (CONS_ADDR_VAL_0_SUMSQ  + 4U)   // 常量地址
#define VAR_ADDR_1_SUMSQ     	 (CONS_ADDR_VAL_1_SUMSQ  + 4U)   // 变量地址,存储的是输入数组的地址
#define END_ADDR_SUMSQ_MEM	   	 (VAR_ADDR_1_SUMSQ    	 + 4U)   // SUMSQ函数所用MEM空间结束地址;	

#define START_SUMSQ_FUNC_ADDR	 (START_PRO_FUNC_ADDR+0x400)	 // SUMSQ函数起始地址


/*
	DOT 地址
*/
#define INPUT_ADDR_PRT1_DOT    (END_ADDR_SUMSQ_MEM   + 4U)   // 数组1首地址
#define INPUT_ADDR_PRT2_DOT    (INPUT_ADDR_PRT1_DOT  +16*4U) // 数组2首地址
#define INPUT_ADDR_NUM_DOT     (INPUT_ADDR_PRT2_DOT  +16*4U) // 数组长度
#define OUTPUT_ADDR_DOT_FUNC   (INPUT_ADDR_NUM_DOT   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_DOT    (OUTPUT_ADDR_DOT_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_DOT    (CONS_ADDR_VAL_0_DOT  + 4U)   // 常量地址
#define VAR_ADDR_1_DOT     	   (CONS_ADDR_VAL_1_DOT  + 4U)   // 变量地址1,存储的是输入数组1首地址
#define VAR_ADDR_2_DOT     	   (VAR_ADDR_1_DOT       + 4U)   // 变量地址2,存储的是输入数组2首地址
#define END_ADDR_DOT_MEM	   (VAR_ADDR_2_DOT    	 + 4U)   // DOT函数所用MEM空间结束地址;	

#define START_DOT_FUNC_ADDR	   (START_SUMSQ_FUNC_ADDR+0x400) // DOT函数起始地址

/*
	I2F 地址
*/
#define INPUT_ADDR_NUM_I2F     (END_ADDR_DOT_MEM     + 4U) // 输入整形数据
#define OUTPUT_ADDR_I2F_FUNC   (INPUT_ADDR_NUM_I2F   + 4U) // 输出地址
#define END_ADDR_I2F_MEM	   (OUTPUT_ADDR_I2F_FUNC + 4U) // I2F函数所用MEM空间结束地址;	

#define START_I2F_FUNC_ADDR	   (START_DOT_FUNC_ADDR+0x200) // I2F函数起始地址

/*
	F2I 地址
*/
#define INPUT_ADDR_NUM_F2I     (END_ADDR_I2F_MEM     + 4U) // 输入整形数据
#define OUTPUT_ADDR_F2I_FUNC   (INPUT_ADDR_NUM_F2I   + 4U) // 输出地址
#define END_ADDR_F2I_MEM	   (OUTPUT_ADDR_F2I_FUNC + 4U) // F2I函数所用MEM空间结束地址;	

#define START_F2I_FUNC_ADDR	   (START_I2F_FUNC_ADDR+0x200) // F2I函数起始地址

/*
	CMP_F 地址
*/
#define INPUT_ADDR_A_CMP_F     (END_ADDR_F2I_MEM       + 4U) // 输入整形数据
#define INPUT_ADDR_B_CMP_F     (INPUT_ADDR_A_CMP_F     + 4U) // 输入整形数据
#define OUTPUT_ADDR_CMP_F_FUNC (INPUT_ADDR_B_CMP_F     + 4U) // 输出地址
#define END_ADDR_CMP_F_MEM	   (OUTPUT_ADDR_CMP_F_FUNC + 4U) // CMP_F函数所用MEM空间结束地址;	

#define START_CMP_F_FUNC_ADDR  (START_F2I_FUNC_ADDR + 0x200) // CMP_F函数起始地址

/*
	DIV_F 地址
*/
#define INPUT_ADDR_A_DIV_F     (END_ADDR_CMP_F_MEM     + 4U) // 输入数据
#define INPUT_ADDR_B_DIV_F     (INPUT_ADDR_A_DIV_F     + 4U) // 输入数据
#define OUTPUT_ADDR_DIV_F_FUNC (INPUT_ADDR_B_DIV_F     + 4U) // 输出地址
#define END_ADDR_DIV_F_MEM	   (OUTPUT_ADDR_DIV_F_FUNC + 4U) // DIV_F函数所用MEM空间结束地址;	

#define START_DIV_F_FUNC_ADDR  (START_CMP_F_FUNC_ADDR + 0x200) // DIV_F函数起始地址

/*
	INVSQRT_F 地址
*/
#define INPUT_ADDR_A_INVSQRT_F     (END_ADDR_DIV_F_MEM         + 4U) // 输入数据
#define OUTPUT_ADDR_INVSQRT_F_FUNC (INPUT_ADDR_A_INVSQRT_F     + 4U) // 输出地址
#define CONS_1_INVSQRT_F_FUNC	   (OUTPUT_ADDR_INVSQRT_F_FUNC + 4U) // 常量地址
#define END_ADDR_INVSQRT_F_MEM	   (CONS_1_INVSQRT_F_FUNC + 4U) // INVSQRT_F函数所用MEM空间结束地址;	

#define START_INVSQRT_F_FUNC_ADDR  (START_DIV_F_FUNC_ADDR + 0x200) // INVSQRT_F函数起始地址

/*
	MAC_F 地址
*/
#define INPUT_ADDR_A_MAC_F     (END_ADDR_INVSQRT_F_MEM + 4U) // 输入数据
#define INPUT_ADDR_B_MAC_F     (INPUT_ADDR_A_MAC_F     + 4U) // 输入数据
#define INPUT_ADDR_C_MAC_F     (INPUT_ADDR_B_MAC_F     + 4U) // 输入数据
#define OUTPUT_ADDR_MAC_F_FUNC (INPUT_ADDR_C_MAC_F     + 4U) // 输出地址
#define END_ADDR_MAC_F_MEM	   (OUTPUT_ADDR_MAC_F_FUNC + 4U) // MAC_F函数所用MEM空间结束地址;	

#define START_MAC_F_FUNC_ADDR  (START_INVSQRT_F_FUNC_ADDR + 0x200) // MAC_F函数起始地址

/*
	ASIN_F 地址
*/
#define INPUT_ADDR_ASIN_F    	 (END_ADDR_MAC_F_MEM + 4U)  // 输入数据
#define OUTPUT_ADDR_ASIN_F  	 (INPUT_ADDR_ASIN_F  + 4U)  // 输出地址
#define MINI_CONS_ADDR_ASIN_F  	 (OUTPUT_ADDR_ASIN_F + 4U)	
#define NEG_CONS_ADDR_ASIN_F	 (MINI_CONS_ADDR_ASIN_F + 4U) // 常量-1 = -1;
#define CONS0_ADDR_ASIN_F	  	 (NEG_CONS_ADDR_ASIN_F  + 4U) // 常量0  = 0;
#define CONS1_ADDR_ASIN_F	  	 (CONS0_ADDR_ASIN_F + 4U) 	  // 常量1  = 1;
#define CONS2_ADDR_ASIN_F  		 (CONS1_ADDR_ASIN_F + 4U) // 常量2  = 2;
#define END_ADDR_ASIN_F_MEM	     (CONS2_ADDR_ASIN_F + 4U) // ASIN_F函数所用MEM空间结束地址;	

#define DSP_ASINF
#define START_ASIN_F_ADDR        (START_MAC_F_FUNC_ADDR + 0x200) // ASIN_F函数起始地址


#else

#define DSP_BASE_ADDR  (0x20018000U)	// DSP base 地址


/*
	SIN 地址
*/
#define 	INPUT_ADDR   		  (0x0U)	
#define 	OUTPUT_ADDR 		  (INPUT_ADDR		  + 4U)	
#define 	NEG_CONS_CONS_ADDR    (OUTPUT_ADDR		  + 4U)	
#define 	MINI_CONS_ADDR  	  (NEG_CONS_CONS_ADDR + 4U)	
#define 	FRAC_CONS_ADDR    	  (MINI_CONS_ADDR     + 4U)	// Factorial = 1;
#define 	I_CONS_ADDR  		  (FRAC_CONS_ADDR     + 4U)	// i         = 1;
#define 	CONS0_ADDR  		  (I_CONS_ADDR   	  + 4U)	// Negation  = 1;
#define 	CONS1_ADDR  		  (CONS0_ADDR    	  + 4U)	// 阶乘常量1  = 1;
#define 	CONS2_ADDR  		  (CONS1_ADDR    	  + 4U) // 阶乘常量2  = 2;	
#define 	END_ADDR_SIN_MEM	  (CONS2_ADDR    	  + 4U) // 正弦函数所用MEM空间结束地址;	

#define 	START_SIN_FUNC_ADDR	  (END_ADDR_SIN_MEM)        // 函数起始地址


/*
	SUM 地址
*/
#define INPUT_ADDR_PRT         (0x0U)  // 数组首地址
#define INPUT_ADDR_NUM         (INPUT_ADDR_PRT     +16*4U)  // 数组长度
#define OUTPUT_ADDR_SUM_FUNC   (INPUT_ADDR_NUM		 + 4U)	// 输出地址
#define CONS_ADDR_VAL_0  	   (OUTPUT_ADDR_SUM_FUNC + 4U)  // 常量地址
#define CONS_ADDR_VAL_1   	   (CONS_ADDR_VAL_0		 + 4U)  // 常量地址
#define VAR_ADDR_1       	   (CONS_ADDR_VAL_1 	 + 4U)  // 变量地址,存储的是输入数组的地址
#define END_ADDR_SUM_MEM	   (VAR_ADDR_1    	     + 4U)  // SUM函数所用MEM空间结束地址;	

#define START_SUM_FUNC_ADDR	   (END_ADDR_SUM_MEM)	// SUM函数起始地址

/*
	SUB 地址
*/

#define INPUT_ADDR_PRT_SUB     (0x0U)   // 数组首地址
#define INPUT_ADDR_NUM_SUB     (INPUT_ADDR_PRT_SUB+ 16*4U)   // 数组长度
#define OUTPUT_ADDR_SUB_FUNC   (INPUT_ADDR_NUM_SUB   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_SUB    (OUTPUT_ADDR_SUB_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_SUB    (CONS_ADDR_VAL_0_SUB  + 4U)   // 常量地址
#define VAR_ADDR_1_SUB     	   (CONS_ADDR_VAL_1_SUB  + 4U)   // 变量地址,存储的是输入数组的地址
#define END_ADDR_SUB_MEM	   (VAR_ADDR_1_SUB    	 + 4U)   // SUB函数所用MEM空间结束地址;	

#define START_SUB_FUNC_ADDR	   (END_ADDR_SUB_MEM)	// SUB函数起始地址


/*
	Product 地址
*/

#define INPUT_ADDR_PRT_PRO     (0x0U)   // 数组首地址
#define INPUT_ADDR_NUM_PRO     (INPUT_ADDR_PRT_PRO+ 16*4U)   // 数组长度
#define OUTPUT_ADDR_PRO_FUNC   (INPUT_ADDR_NUM_PRO   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_PRO    (OUTPUT_ADDR_PRO_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_PRO    (CONS_ADDR_VAL_0_PRO  + 4U)   // 常量地址
#define VAR_ADDR_1_PRO     	   (CONS_ADDR_VAL_1_PRO  + 4U)   // 变量地址,存储的是输入数组的地址
#define END_ADDR_RPO_MEM	   (VAR_ADDR_1_PRO    	 + 4U)   // Product函数所用MEM空间结束地址;	

#define START_PRO_FUNC_ADDR	   (END_ADDR_RPO_MEM)	 // Product函数起始地址


/*
	SUMSQ 地址
*/

#define INPUT_ADDR_PRT_SUMSQ     (0U)   // 数组首地址
#define INPUT_ADDR_NUM_SUMSQ     (INPUT_ADDR_PRT_SUMSQ+ 16*4U)   // 数组长度
#define OUTPUT_ADDR_SUMSQ_FUNC   (INPUT_ADDR_NUM_SUMSQ   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_SUMSQ    (OUTPUT_ADDR_SUMSQ_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_SUMSQ    (CONS_ADDR_VAL_0_SUMSQ  + 4U)   // 常量地址
#define VAR_ADDR_1_SUMSQ     	 (CONS_ADDR_VAL_1_SUMSQ  + 4U)   // 变量地址,存储的是输入数组的地址
#define END_ADDR_SUMSQ_MEM	   	 (VAR_ADDR_1_SUMSQ    	 + 4U)   // SUMSQ函数所用MEM空间结束地址;	

#define START_SUMSQ_FUNC_ADDR	 (END_ADDR_SUMSQ_MEM)	 // SUMSQ函数起始地址


/*
	DOT 地址
*/

#define INPUT_ADDR_PRT1_DOT    (0U)   // 数组1首地址
#define INPUT_ADDR_PRT2_DOT    (INPUT_ADDR_PRT1_DOT  +16*4U) // 数组2首地址
#define INPUT_ADDR_NUM_DOT     (INPUT_ADDR_PRT2_DOT  +16*4U) // 数组长度
#define OUTPUT_ADDR_DOT_FUNC   (INPUT_ADDR_NUM_DOT   + 4U)	 // 输出地址
#define CONS_ADDR_VAL_0_DOT    (OUTPUT_ADDR_DOT_FUNC + 4U)   // 常量地址
#define CONS_ADDR_VAL_1_DOT    (CONS_ADDR_VAL_0_DOT  + 4U)   // 常量地址
#define VAR_ADDR_1_DOT     	   (CONS_ADDR_VAL_1_DOT  + 4U)   // 变量地址1,存储的是输入数组1首地址
#define VAR_ADDR_2_DOT     	   (VAR_ADDR_1_DOT       + 4U)   // 变量地址2,存储的是输入数组2首地址
#define END_ADDR_DOT_MEM	   (VAR_ADDR_2_DOT    	 + 4U)   // DOT函数所用MEM空间结束地址;	

#define START_DOT_FUNC_ADDR	   (END_ADDR_DOT_MEM) // DOT函数起始地址

/*
	I2F 地址
*/
#define INPUT_ADDR_NUM_I2F     (0U) // 输入整形数据
#define OUTPUT_ADDR_I2F_FUNC   (INPUT_ADDR_NUM_I2F   + 4U) // 输出地址
#define END_ADDR_I2F_MEM	   (OUTPUT_ADDR_I2F_FUNC + 4U) // I2F函数所用MEM空间结束地址;	

#define START_I2F_FUNC_ADDR	   (END_ADDR_I2F_MEM) // I2F函数起始地址

/*
	F2I 地址
*/
#define INPUT_ADDR_NUM_F2I     (0U) // 输入整形数据
#define OUTPUT_ADDR_F2I_FUNC   (INPUT_ADDR_NUM_F2I   + 4U) // 输出地址
#define END_ADDR_F2I_MEM	   (OUTPUT_ADDR_F2I_FUNC + 4U) // F2I函数所用MEM空间结束地址;	

#define START_F2I_FUNC_ADDR	   (END_ADDR_F2I_MEM) // F2I函数起始地址

/*
	CMP_F 地址
*/
#define INPUT_ADDR_A_CMP_F     (0U) // 输入整形数据
#define INPUT_ADDR_B_CMP_F     (INPUT_ADDR_A_CMP_F     + 4U) // 输入整形数据
#define OUTPUT_ADDR_CMP_F_FUNC (INPUT_ADDR_B_CMP_F     + 4U) // 输出地址
#define END_ADDR_CMP_F_MEM	   (OUTPUT_ADDR_CMP_F_FUNC + 4U) // CMP_F函数所用MEM空间结束地址;	

#define START_CMP_F_FUNC_ADDR  (END_ADDR_CMP_F_MEM) // CMP_F函数起始地址

/*
	DIV_F 地址
*/
#define INPUT_ADDR_A_DIV_F     (0U) // 输入数据
#define INPUT_ADDR_B_DIV_F     (INPUT_ADDR_A_DIV_F     + 4U) // 输入数据
#define OUTPUT_ADDR_DIV_F_FUNC (INPUT_ADDR_B_DIV_F     + 4U) // 输出地址
#define END_ADDR_DIV_F_MEM	   (OUTPUT_ADDR_DIV_F_FUNC + 4U) // DIV_F函数所用MEM空间结束地址;	

#define START_DIV_F_FUNC_ADDR  (END_ADDR_DIV_F_MEM) // DIV_F函数起始地址

/*
	INVSQRT_F 地址
*/
#define INPUT_ADDR_A_INVSQRT_F     (0U) // 输入数据
#define OUTPUT_ADDR_INVSQRT_F_FUNC (INPUT_ADDR_A_INVSQRT_F     + 4U) // 输出地址
#define CONS_1_INVSQRT_F_FUNC	   (OUTPUT_ADDR_INVSQRT_F_FUNC + 4U) // 常量地址
#define END_ADDR_INVSQRT_F_MEM	   (CONS_1_INVSQRT_F_FUNC + 4U) // INVSQRT_F函数所用MEM空间结束地址;	

#define START_INVSQRT_F_FUNC_ADDR  (END_ADDR_INVSQRT_F_MEM) // INVSQRT_F函数起始地址

/*
	MAC_F 地址
*/
#define INPUT_ADDR_A_MAC_F     (0U) // 输入数据
#define INPUT_ADDR_B_MAC_F     (INPUT_ADDR_A_MAC_F     + 4U) // 输入数据
#define INPUT_ADDR_C_MAC_F     (INPUT_ADDR_B_MAC_F     + 4U) // 输入数据
#define OUTPUT_ADDR_MAC_F_FUNC (INPUT_ADDR_C_MAC_F     + 4U) // 输出地址
#define END_ADDR_MAC_F_MEM	   (OUTPUT_ADDR_MAC_F_FUNC + 4U) // MAC_F函数所用MEM空间结束地址;	

#define START_MAC_F_FUNC_ADDR  (END_ADDR_MAC_F_MEM) // MAC_F函数起始地址


/*
	ASIN_F 地址
*/
#define INPUT_ADDR_ASIN_F    	 (0U)  // 输入数据
#define OUTPUT_ADDR_ASIN_F  	 (INPUT_ADDR_ASIN_F  + 4U)  // 输出地址
#define MINI_CONS_ADDR_ASIN_F  	 (OUTPUT_ADDR_ASIN_F + 4U)	
#define NEG_CONS_ADDR_ASIN_F	 (MINI_CONS_ADDR_ASIN_F + 4U) // 常量-1 = -1;
#define CONS0_ADDR_ASIN_F	  	 (NEG_CONS_ADDR_ASIN_F  + 4U) // 常量0  = 0;
#define CONS1_ADDR_ASIN_F	  	 (CONS0_ADDR_ASIN_F + 4U) 	  // 常量1  = 1;
#define CONS2_ADDR_ASIN_F  		 (CONS1_ADDR_ASIN_F + 4U) // 常量2  = 2;
#define END_ADDR_ASIN_F_MEM	     (CONS2_ADDR_ASIN_F + 4U) // ASIN_F函数所用MEM空间结束地址;	

#define START_ASIN_F_ADDR        (END_ADDR_ASIN_F_MEM) // ASIN_F函数起始地址

#endif


/* Part1: 基础函数 ********************************************************************/
float Software_I2F(int32_t num);
float DSP_I2F(int32_t num);
void DSP_I2F_Process(void);

int32_t Software_F2I(float num);
int32_t DSP_F2I(float num);
void DSP_F2I_Process(void);

uint32_t Software_CMP_F(float a, float b);
uint32_t DSP_CMP_F(float a, float b);
void DSP_CMP_F_Process(void);

float Software_DIV_F(float a, float b);
float DSP_DIV_F(float a, float b);
void DSP_DIV_F_Process(void);

float Software_INVSQRT_F(float a);
float DSP_INVSQRT_F(float a);
void DSP_INVSQRT_F_Process(void);

float Software_MAC_F(float a, float b, float c);
float DSP_MAC_F(float a, float b, float c);
void DSP_MAC_F_Process(void);

/* Part2: 数组操作函数 ****************************************************************/
float Software_SUM(float * ptr, uint8_t num);	
float DSP_SUM(float *ptr, uint8_t num);
void DSP_SUM_Process(void);

float Software_SUB(float * ptr, uint8_t num);
float DSP_SUB(float *ptr, uint8_t num);
void DSP_SUB_Process(void);

float Software_PRDCT(float * ptr, uint8_t num);
float DSP_PRDCT(float *ptr, uint8_t num);
void DSP_PRDCT_Process(void);

float Software_SUMSQ(float * ptr, uint8_t num);
float DSP_SUMSQ(float *ptr, uint8_t num);
void DSP_SUMSQ_Process(void);

float Software_DOT(float * ptr1, float * ptr2, uint8_t num);
float DSP_DOT(float *ptr1, float *ptr2, uint8_t num);
void DSP_DOT_Process(void);

/* Part3: 三角函数 ********************************************************************/
float Software_Sin(float num2);
float DSP_SIN(float a);
void DSP_Taylor_Sin_Process(void);

float Software_Cos(float num2);
float DSP_COS(float a);

float Software_tan(float num2);
float DSP_TAN(float a);

float Software_ASIN(float num2);
float DSP_ASIN(float a);
void DSP_Taylor_ASIN_Process(void);

/* Part4: DSP指令写入函数 ********************************************************************/
void DSP_FUNC_Write(void);

#endif


