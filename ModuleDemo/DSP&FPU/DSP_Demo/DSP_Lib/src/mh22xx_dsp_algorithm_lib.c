#include "mh22xx_dsp_algorithm_lib.h"

/*
	文件介绍：
	函数分两部分，第一部分是DSP指令基础函数，第二部分是已经实现的DSP函数。
	第一部分包括：寄存器指令，内存指令，浮点运算指令，事件指令。
	第二部分包括：
	1. 基础函数，包括：DSP_I2F，DSP_F2I，DSP_CMP_F，DSP_DIV_F，DSP_INVSQRT_F，DSP_MAC_F
	2. 数组函数，包括：DSP_SUM，DSP_SUB，DSP_PRDCT，DSP_SUMSQ，DSP_DOT
	3. 三角函数，包括：DSP_SIN，DSP_COS，DSP_TAN，DSP_ASIN
*/


/* DSP指令函数 **************************************/
/*
	dsp_addr: DSP指令存储地址；为DSP模块编辑的代码指令依次存储在该变量存储的地址中；
	DSP_BASE_ADDR: DSP指令存储区域基地址。
*/
uint32_t dsp_addr = DSP_BASE_ADDR;


									 
uint32_t R_LOAD(uint32_t OP1 ,uint32_t OP2 ) {

	*(__IO uint16_t *)dsp_addr = (((DSP_LOAD & 0X1F) << 11) |
			                      ((OP1      & 0X7)  << 8 ) |
			                      ((OP2      & 0X7)  << 5 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}




uint32_t R_LOAD_R (uint32_t OP1, uint32_t OP2) 
{
	*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_R & 0x1F) << 11) |
			                      ((OP1        & 0x7 ) << 8 ) |
			                      ((OP2        & 0xFF) << 0 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}


uint32_t R_CMP(uint32_t OP1, uint32_t OP2, uint32_t OP3, uint32_t OP4 ) {

	*(__IO uint16_t *)dsp_addr = (((DSP_CMP & 0X1F) << 11) |
			                      ((OP1     & 0X7 ) <<8)   |
			                      ((OP2     & 0X7 ) <<5)   |
			                      ((OP3     & 0X7 ) <<2)   |
			                      ((OP4     & 0X3 ) <<0));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t R_CMP_EQ(uint32_t OP1, uint32_t OP2, uint32_t OP3, uint32_t OP4 ) {

	*(__IO uint16_t *)dsp_addr = (((DSP_CMP_EQ & 0X1F) << 11) |
			                      ((OP1        & 0X7 ) << 8 ) |
			                      ((OP2        & 0X7 ) << 5 ) |
			                      ((OP3        & 0X7 ) << 2 ) |
			                      ((OP4        & 0X3 ) << 0 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t R_ADD_R(uint32_t OP1, uint32_t OP2, uint32_t OP3) {

	*(__IO uint16_t *)dsp_addr = (((DSP_ADD_R & 0X1F) << 11) |
			                      ((OP1       & 0X7 ) << 8 ) |
			                      ((OP2       & 0X7 ) << 5 ) |
			                      ((OP3       & 0X7 ) << 2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t R_SUB_R(uint32_t OP1, uint32_t OP2, uint32_t OP3) {

	*(__IO uint16_t *)dsp_addr = (((DSP_SUB_R & 0X1F) << 11) |
			                      ((OP1       & 0X7 ) << 8 ) |
			                      ((OP2       & 0X7 ) << 5 ) |
			                      ((OP3       & 0X7 ) << 2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t R_GOTO(uint32_t OP1 ) {

	*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M_A & 0x01F) << 11) | (((OP1 >> 8) & 0x7FF) << 0 ));  // 写入高字节地址
	dsp_addr = dsp_addr + 2;	

	*(__IO uint16_t *)dsp_addr = (((DSP_GOTO & 0X1F)<<11) | (OP1 & 0XFF));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 4);
}


// 从内存地址装载内容
// 当OP3 = 0时，从地址装载内容；当OP = 1时，从寄存器装载内容；
uint32_t M_LOAD_M(uint32_t OP1, uint32_t OP2, uint32_t OP3) {

	if(OP3 != 0) {
		*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M_A & 0x1F)<< 11) | (4 << 8));  // 配置寄存器装载模式；
		dsp_addr = dsp_addr + 2;	
		
		*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M & 0x1F) << 11)  |
									  ((OP1        & 0x7 ) << 8 )  |
									  ((OP2        & 0x7 ) << 5 ));
		dsp_addr = dsp_addr + 2;			
	}
	else {
		*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M_A & 0x1F)<< 11) | ((OP2 >> 8) & 0x7FF));  // 写入高字节地址
		dsp_addr = dsp_addr + 2;	
		
		*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M & 0x1F) << 11)  |
									  ((OP1        & 0x7 ) << 8)   |
									  ((OP2        & 0xFF)));
		dsp_addr = dsp_addr + 2;	
	}
	return (dsp_addr - DSP_BASE_ADDR - 4);
}

// 从寄存器装载内容
// 当OP3 = 0时，从寄存器装载到内存；当OP = 1时，从寄存器装载到寄存器；
uint32_t M_STR_M(uint32_t OP1 ,uint32_t OP2, uint32_t OP3) {

	if(OP3 != 0) {	
		*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M_A & 0x1F)<< 11) | (4 << 8));  // 配置寄存器装载模式；
		dsp_addr = dsp_addr + 2;	
		
		*(__IO uint16_t *)dsp_addr = (((DSP_STR_M & 0x1F) << 11) |
									  ((OP1       & 0x7 ) << 8 ) |
									  ((OP2       & 0x7 ) << 5 ));
		 dsp_addr = dsp_addr + 2;				
	}
	else {
		*(__IO uint16_t *)dsp_addr = (((DSP_LOAD_M_A & 0x1F)<< 11) | ((OP2 >> 8) & 0x7FF));  // 写入高字节地址
		dsp_addr = dsp_addr + 2;	
		
		*(__IO uint16_t *)dsp_addr = (((DSP_STR_M & 0x1F) << 11) |
									  ((OP1       & 0x7 ) << 8 ) |
									  ((OP2       & 0xff) << 0 ));
		dsp_addr = dsp_addr + 2;	
	}	
	return (dsp_addr - DSP_BASE_ADDR - 4);
}

uint32_t F_FADD(uint32_t OP1 ,uint32_t OP2 ,uint32_t OP3) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_ADD & 0X1F) << 11) |
			                      ((OP1       & 0X7 ) << 8 ) |
			                      ((OP2       & 0X7 ) << 5 ) |
			                      ((OP3       & 0X7 ) << 2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_FSUB(uint32_t OP1 ,uint32_t OP2 ,uint32_t OP3) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_SUB & 0X1F) <<11) |
			                      ((OP1       & 0X7 ) <<8 ) |
			                      ((OP2       & 0X7 ) <<5 ) |
			                      ((OP3       & 0X7 ) <<2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_FMUL(uint32_t OP1 ,uint32_t OP2 ,uint32_t OP3) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_MUL & 0X1F) << 11) |
			                      ((OP1       & 0X7 ) << 8 ) |
			                      ((OP2       & 0X7 ) << 5 ) |
			                      ((OP3       & 0X7 ) << 2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_FDIV(uint32_t OP1 ,uint32_t OP2 ,uint32_t OP3) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_DIV & 0X1F) <<11) |
			                      ((OP1       & 0X7 ) <<8 ) |
			                      ((OP2       & 0X7 ) <<5 ) |
			                      ((OP3       & 0X7 ) <<2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_FSQRT(uint32_t OP1 ,uint32_t OP2) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_SQRT & 0X1F) <<11) |
			                      ((OP1        & 0X7 ) <<8 ) |
			                      ((OP2        & 0X7 ) <<2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_F2I(uint32_t OP1 ,uint32_t OP2) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_F2I & 0X1F) <<11) |
			                      ((OP1       & 0X7 ) <<8 ) |
			                      ((OP2       & 0X7 ) <<2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_I2F(uint32_t OP1 ,uint32_t OP2) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_I2F & 0X1F) <<11) |
			                      ((OP1       & 0X7 ) <<8 ) |
			                      ((OP2       & 0X7 ) <<2 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}


uint32_t F_FCMP(uint32_t OP1, uint32_t OP2, uint32_t OP3, uint32_t OP4 ) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_CMP & 0X1F) << 11) |
			                      ((OP1       & 0X7 ) << 8 ) |
			                      ((OP2       & 0X7 ) << 5 ) |
			                      ((OP3       & 0X7 ) << 2 ) |
			                      ((OP4       & 0X3 ) << 0 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t F_FCMP_EQ(uint32_t OP1, uint32_t OP2, uint32_t OP3, uint32_t OP4 ) {

	*(__IO uint16_t *)dsp_addr = (((DSP_F_CMP_EQ & 0X1F) << 11) |
			                      ((OP1          & 0X7 ) << 8 ) |
			                      ((OP2          & 0X7 ) << 5 ) |
			                      ((OP3          & 0X7 ) << 2 ) |
			                      ((OP4          & 0X3 ) << 0 ));
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t E_EVINT(void) {

	*(__IO uint16_t *)dsp_addr = ((DSP_EVINT & 0X1F) << 11);
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t E_DONE(void) {

	*(__IO uint16_t *)dsp_addr = ((DSP_DONE & 0X1F) << 11);
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

uint32_t E_SUSPEND(void) {

	*(__IO uint16_t *)dsp_addr = ((DSP_SUNPEND & 0X1F) << 11);
	dsp_addr = dsp_addr + 2;
	
	return (dsp_addr - DSP_BASE_ADDR - 2);
}

/*
	Memory空间内容写入浮点数。
	idx: 地址编号。每个编号代表一个“字”地址，4字节对齐； 
	data：写入的数据；
*/ 
void MEM_Write_F(uint32_t idx, float data)
{
	*(__IO float*) (DSP_BASE_ADDR + idx) = data;
}

/*
	Memory空间内容写入无符号整形。
	idx: 地址编号。每个编号代表一个“字”地址，4字节对齐； 
	data：写入的数据；
*/ 
void MEM_Write_UINT32(uint32_t idx, uint32_t data)
{
	*(__IO uint32_t*) (DSP_BASE_ADDR + idx) = data;
}

/*
	Memory空间内容写入整形。
	idx: 地址编号。每个编号代表一个“字”地址，4字节对齐； 
	data：写入的数据；
*/ 
void MEM_Write_INT32(uint32_t idx, int32_t data)
{
	*(__IO int32_t*) (DSP_BASE_ADDR + idx) = data;
}

/*
	Memory空间内容读出浮点数。
	idx: 地址编号。每个编号代表一个“字”地址，4字节对齐； 
	data：读出的数据；
*/ 
void MEM_Read(uint16_t idx, float * data)
{
	*data = *(__IO float*) (DSP_BASE_ADDR + idx);
}

/* Part1: 基础函数 ********************************************************************/

/*
	计算I2F，整形数据转换为浮点数；
	para num: 待转换的整形数据。
	out    Y: 转换完成的浮点数	
*/
float Software_I2F(int32_t num)	
{	
	return (float)(num);
}
/*
	DSP计算 I2F

	固定寄存器：
	num     ：R6
	
	临时寄存器
	R7；
*/
void DSP_I2F_Process(void)
{	
	M_LOAD_M(DSP_R6, INPUT_ADDR_NUM_I2F, 0);    // dot = 0;
	F_I2F(DSP_R6, DSP_R7);     		   // num = f(num);
	
	M_STR_M(DSP_R7, OUTPUT_ADDR_I2F_FUNC, 0); // DONE DATA   94
	E_DONE();
}

/*
	计算I2F，整形数据转换为浮点数；
	para num: 待转换的整形数据。
	out    Y: 转换完成的浮点数	
*/
float DSP_I2F(int32_t num)
{
	float Y;
	
	MEM_Write_INT32(INPUT_ADDR_NUM_I2F, num);	// 整形数据写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_I2F_FUNC_ADDR);

	while(1){		
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_I2F_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}


/*
	计算F2I，浮点数转换为整形数据；
	para num: 待转换的整形数据。
	out    Y: 转换完成的浮点数	
*/
int32_t Software_F2I(float num)	
{
	return (rint(num));
}
/*
	DSP计算F2I

	固定寄存器：
	num     ：R6
	
	临时寄存器
	R7；
*/
void DSP_F2I_Process(void)
{		
	M_LOAD_M(DSP_R6, INPUT_ADDR_NUM_F2I, 0);    // dot = 0;	 	
	F_F2I(DSP_R6, DSP_R7);     		   // num = int(num);
	
	M_STR_M(DSP_R7, OUTPUT_ADDR_F2I_FUNC, 0); // DONE DATA   94
	E_DONE();
}

/*
	计算F2I，浮点数转换为整形数据；
	para num: 待转换的浮点数。
	out    Y: 转换完成的整形数据	
*/
int32_t DSP_F2I(float num)
{
	float Y;
	
	MEM_Write_F(INPUT_ADDR_NUM_F2I, num);	// 数据写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_F2I_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			Y = *(__IO int32_t*) (DSP_BASE_ADDR + OUTPUT_ADDR_F2I_FUNC);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}


/*
	计算CMP，浮点数比较；
	para a: 浮点数1；
	para b: 浮点数2；
	out  1: a==b;
	out  2: a < b;
	out  4: a > b;
	out  8: a b数据异常;
*/
uint32_t Software_CMP_F(float a, float b)	
{	
	if ((a - b) == 0 ) {   // a==b;
		return (1);
	}
	else if ((a - b) < 0){  // a < b;
		return (2);
	}
	else if ((a - b) > 0){  // a > b;
		return (4);
	}	
	else // a b数据异常;
		return (8);				
}
/*
	计算CMP，浮点数比较；

	固定寄存器：
	a     ：R0
	b     ：R1	

	临时寄存器
	R7；
*/
void DSP_CMP_F_Process(void)
{		
	uint32_t big_addr;
	uint32_t eq_addr;
	
	M_LOAD_M(DSP_R0, INPUT_ADDR_A_CMP_F, 0);    // 输入a      00
	M_LOAD_M(DSP_R1, INPUT_ADDR_B_CMP_F, 0);    // 输入b 	 04
	
	eq_addr = F_FCMP_EQ(DSP_R0, DSP_R1, 0, 2);   	// a == b ?	 08
	R_GOTO(eq_addr + 5*2);  
	R_GOTO(eq_addr + 18*2);    			// a == b 跳转地址
	
	big_addr = F_FCMP(DSP_R0, DSP_R1, 0, 2);  	 // a > b ?	 0A
	R_GOTO(big_addr + 5*2);    		 // a < b 跳转地址
	R_GOTO(big_addr + 9*2);    		 // a > b 跳转地址
	
	R_LOAD_R(DSP_R7, 2);	         			      // a < b      0C
	M_STR_M(DSP_R7, OUTPUT_ADDR_CMP_F_FUNC, 0);  // DONE DATA  0E
	E_DONE();
	
	R_LOAD_R(DSP_R7, 4);	         			       // a > b      14
	M_STR_M(DSP_R7, OUTPUT_ADDR_CMP_F_FUNC, 0);  // DONE DATA  16
	E_DONE();
	
	R_LOAD_R(DSP_R7, 1);	         			      // a == b	    1C
	M_STR_M(DSP_R7, OUTPUT_ADDR_CMP_F_FUNC, 0);  // DONE DATA  1E
	E_DONE();
}

/*
	计算CMP，浮点数比较；
	para a: 浮点数1；
	para b: 浮点数2；
	out  1: a==b;
	out  2: a < b;
	out  4: a > b;
	out  8: a b数据异常;
*/

uint32_t DSP_CMP_F(float a, float b)
{
	float Y;
	
	MEM_Write_F(INPUT_ADDR_A_CMP_F, a);	// 数据写入；
	MEM_Write_F(INPUT_ADDR_B_CMP_F, b);	// 数据写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_CMP_F_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			Y = *(__IO uint32_t*) (DSP_BASE_ADDR + OUTPUT_ADDR_CMP_F_FUNC);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}

/*
	计算DIV，浮点数除法；
	para a: 浮点数1；
	para b: 浮点数2；
	out   : 商;
*/
float Software_DIV_F(float a, float b)	
{		
	return (float) (a / b);
}
/*
	计算DIV，浮点数除法；

	固定寄存器：
	a     ：R0
	b     ：R1	

	临时寄存器
	R7；
*/
void DSP_DIV_F_Process(void)
{			
	M_LOAD_M(DSP_R0, INPUT_ADDR_A_DIV_F, 0);    // 输入a 
	M_LOAD_M(DSP_R1, INPUT_ADDR_B_DIV_F, 0);    // 输入b 
	F_FDIV(DSP_R0, DSP_R1, DSP_R7);   	   // a / b 
	
	M_STR_M(DSP_R7, OUTPUT_ADDR_DIV_F_FUNC, 0);  // DONE DATA  
	E_DONE();
}

/*
	计算DIV，浮点数除法；
	para a: 浮点数1；
	para b: 浮点数2；
	out  Y: 商;
*/

float DSP_DIV_F(float a, float b)
{
	float Y;
	
	MEM_Write_F(INPUT_ADDR_A_DIV_F, a);	// 数据写入；
	MEM_Write_F(INPUT_ADDR_B_DIV_F, b);	// 数据写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_DIV_F_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_DIV_F_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}
/*
	计算INVSQRT，平方根的倒数；
	para a: 浮点数1；
	out   : 平方根的倒数;
*/
float Software_INVSQRT_F(float a)	
{	
	return (1/sqrtf(a));
}
/*
	计算INVSQRT，平方根的倒数；

	固定寄存器：
	a     ：R0

	临时寄存器
	R7；
*/
void DSP_INVSQRT_F_Process(void)
{	
	MEM_Write_F(CONS_1_INVSQRT_F_FUNC, 1);	// 常量1写入； 				

	M_LOAD_M(DSP_R6, CONS_1_INVSQRT_F_FUNC, 0);     // 写入常量1 
	M_LOAD_M(DSP_R0, INPUT_ADDR_A_INVSQRT_F, 0);    // 输入a 
	
	F_FSQRT(DSP_R0, DSP_R0);   	   // sqrt a
	F_FDIV(DSP_R6, DSP_R0, DSP_R7);   	       // 1 / sqrt a
	
	M_STR_M(DSP_R7, OUTPUT_ADDR_INVSQRT_F_FUNC, 0);  // DONE DATA  
	E_DONE();
}

/*
	计算INVSQRT，平方根的倒数；
	para a: 浮点数1；
	out  Y: 平方根的倒数;
*/

float DSP_INVSQRT_F(float a)
{
	float Y;
	
	MEM_Write_F(INPUT_ADDR_A_INVSQRT_F, a);	// 数据写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_INVSQRT_F_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_INVSQRT_F_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}

/*
	计算MAC，乘加计算；
	para a: 浮点数1；
	para b: 浮点数2；
	para c: 浮点数3；
	out   : a*b+c;
*/
float Software_MAC_F(float a, float b, float c)	
{		
	return (a*b+c);
}
/*
	计算MAC，乘加计算

	固定寄存器：
	a     ：R0
	b     ：R1
	c     ：R2

	临时寄存器
	R7；
*/
void DSP_MAC_F_Process(void)
{					
	M_LOAD_M(DSP_R0, INPUT_ADDR_A_MAC_F, 0);     
	M_LOAD_M(DSP_R1, INPUT_ADDR_B_MAC_F, 0);     
	M_LOAD_M(DSP_R2, INPUT_ADDR_C_MAC_F, 0);     
		
	F_FMUL(DSP_R0, DSP_R1, DSP_R3);   	 	  // a*b
	F_FADD(DSP_R3, DSP_R2, DSP_R7);   		  // a*b+c
	
	M_STR_M(DSP_R7, OUTPUT_ADDR_MAC_F_FUNC, 0);  // DONE DATA  
	E_DONE();
}

/*
	计算MAC，乘加计算；
	para a: 浮点数1；
	para b: 浮点数2；
	para c: 浮点数3；
	out  Y: a*b+c;
*/

float DSP_MAC_F(float a, float b, float c)
{
	float Y;
	
	MEM_Write_F(INPUT_ADDR_A_MAC_F, a);	// 数据写入；
	MEM_Write_F(INPUT_ADDR_B_MAC_F, b);	// 数据写入；
	MEM_Write_F(INPUT_ADDR_C_MAC_F, c);	// 数据写入；
	
//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_MAC_F_FUNC_ADDR);

	while(1){

		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_MAC_F_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}
/* Part2: 数组操作函数 ****************************************************************/

/*
	软件计算SUM，数组求和；
*/
float Software_SUM(float * ptr, uint8_t num)	
{
	float sum = ptr[0];
	uint8_t i = 0;
	
	for( i = 0; i<num; i++){
		sum += ptr[i+1];
	}
	return sum;
}


/*
	DSP计算SUM，多项式求和；

	固定寄存器：
	sum ：R0
	i   ：R1
	ptr ：R2
	num ：R3
	
	临时寄存器
	R4；
	R5；
*/
void DSP_SUM_Process(void)
{
	uint32_t for_big_temp_addr;
	
	/* 初始化数据存储地址,常量写入*/ 
	MEM_Write_F(CONS_ADDR_VAL_1, 1);	 		 	// i 递增需要的常量1；
	MEM_Write_UINT32(VAR_ADDR_1, INPUT_ADDR_PRT);	// 数组地址写入；
	
	/* init，初始值装载到寄存器中； 注：初始值存储可以使用内存指令直接装载到寄存器中。 */ 
	// SUM = R0 = ptr[0]; 
	// i   = R1 = 0; 写入立即数0；
	// ptr = R2 = VAR_ADDR_1;
	// num = R3 = num; 写入立即数num;
	R_LOAD_R(DSP_R1, 0);	         		 // i = 0; 
	M_LOAD_M(DSP_R2, VAR_ADDR_1, 0);      // R2 存入变量地址ptr;	);	
	M_LOAD_M(DSP_R3, INPUT_ADDR_NUM, 0);  // R3 = num;	
	M_LOAD_M(DSP_R0, DSP_R2, 1); 	     // sum = ptr[0]；	
	
	/* for 循环，大于比较 */
	for_big_temp_addr = R_CMP(DSP_R3, DSP_R1, 0, 2);  // num > i      1C  !!!
	R_GOTO(for_big_temp_addr + 12*2);    		// 不满足，向下GOTO到“==”比较 	1E	
	
	/* 计算 */
	// ptr地址递增
	R_LOAD_R(DSP_R4, 4);              	 // PRT的地址递增常量4;	 80
	R_ADD_R(DSP_R2, DSP_R4, DSP_R2);  	 // ptr++; 地址递增4；    82	
	
	// 数组地址写入R2
	M_LOAD_M(DSP_R4, DSP_R2, 1); 	   	 // ptr值装载到R4；	74
	F_FADD(DSP_R0, DSP_R4, DSP_R0);  // sum = sum+ptr;  78
	
	// i++;
	R_LOAD_R(DSP_R4, 1);   // i 的递增常量1;	84
	
	R_ADD_R(DSP_R1, DSP_R4, DSP_R1);  	 // i = i+1;  86
	
	R_GOTO(for_big_temp_addr);    	 // 90
	
	M_STR_M(DSP_R0, OUTPUT_ADDR_SUM_FUNC, 0); // DONE DATA   94
	
	E_DONE();
}


/*
	DSP计算SUM()函数；
*/
float DSP_SUM(float *ptr, uint8_t num)
{
	uint32_t idx = 0;          // 数组写入变量；
	float Y;

	// ptr 写入
	for( idx = 0; idx <= num; idx++) {
		MEM_Write_F(INPUT_ADDR_PRT + (idx*4), ptr[idx]);	//数组数据写入； 				
	}
	// num 写入
	MEM_Write_UINT32(INPUT_ADDR_NUM, num);	// 数组长度写入；
	
//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_SUM_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_SUM_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}

/*
	软件计算SUB，数组求差；
*/
float Software_SUB(float * ptr, uint8_t num)	
{
	float sub = ptr[0];
	uint8_t i = 0;
	
	for( i = 1; i <= num; i++){
		sub -= ptr[i];
	}
	return sub;
}


/*
	DSP计算SUB，多项式求差；

	固定寄存器：
	sub ：R0
	i   ：R1
	ptr ：R2
	num ：R3
	
	临时寄存器
	R4；
	R5；
*/
void DSP_SUB_Process(void)
{
	uint32_t for_big_temp_addr;
	uint32_t for_eq_temp_addr;
	
	/* 初始化数据存储地址,常量写入*/ 
//	MEM_Write_F(CONS_ADDR_VAL_0, 0.0);	 				// SUB 初始化常量 = ptr[0];
	MEM_Write_F(CONS_ADDR_VAL_1_SUB, 1);	 			// i 递增需要的常量1；
	MEM_Write_UINT32(VAR_ADDR_1_SUB, INPUT_ADDR_PRT_SUB);	// 数组地址写入；
	

	/* init，初始值装载到寄存器中； 注：初始值存储可以使用内存指令直接装载到寄存器中。 */ 
	// i   = R1 = 1; 写入立即数1；
	// ptr = R2 = VAR_ADDR_1_SUB;
	// num = R3 = num; 写入立即数num;
	// SUB = R0 = ptr[0]; 
	R_LOAD_R(DSP_R1, 1);	         				// i = 1;   
	M_LOAD_M(DSP_R2, VAR_ADDR_1_SUB, 0);  		// R2 存入变量地址ptr;	02
	M_LOAD_M(DSP_R3, INPUT_ADDR_NUM_SUB, 0); 	// R3 = num;	 06
	M_LOAD_M(DSP_R0, DSP_R2, 1); 	  			// ptr[0]值装载到R0；	0A

	R_LOAD_R(DSP_R4, 4);              			// PRT的地址递增常量4;	 
	R_ADD_R(DSP_R2, DSP_R4, DSP_R2);  			// ptr++; 地址递增4；    
	
	/* for 循环，大于比较 */
	for_big_temp_addr = R_CMP(DSP_R3, DSP_R1, 0, 2);  // num > i      1C  !!!
	R_GOTO(for_big_temp_addr + 12*2);    		// 不满足，向下GOTO到“==”比较 	1E
	
	/* 计算 */
	// 数组地址写入R2
	M_LOAD_M(DSP_R4, DSP_R2, 1); 	   // ptr值装载到R4；	0E	
	F_FSUB(DSP_R0, DSP_R4, DSP_R0);  // sum = sum-ptr;  12
	
	// ptr地址递增
	R_LOAD_R(DSP_R4, 4);              	 // PRT的地址递增常量4;	 14
	R_ADD_R(DSP_R2, DSP_R4, DSP_R2);  	 // ptr++; 地址递增4；    16
	
	// i++;
	R_LOAD_R(DSP_R4, 1);              	 // i 的递增常量1;	18
	R_ADD_R(DSP_R1, DSP_R4, DSP_R1);  	 // i = i+1;        1A
	R_GOTO(for_big_temp_addr);    	 //  		      1E

	/* for 循环，等于比较 */
	for_eq_temp_addr = R_CMP_EQ(DSP_R3, DSP_R1, 0, 2);   // num > i      1C  
	R_GOTO(for_eq_temp_addr + 6*2);    		   //  		      1E
	
	// 数组地址写入R2
	M_LOAD_M(DSP_R4, DSP_R2, 1); 	     // ptr值装载到R4；	0E
	F_FSUB(DSP_R0, DSP_R4, DSP_R0);  // sum = sum-ptr;  12	

	M_STR_M(DSP_R0, OUTPUT_ADDR_SUB_FUNC, 0);  // DONE DATA    22
	E_DONE();        // stop dsp  
}

/*
	计算SUB()函数；
*/
float DSP_SUB(float *ptr, uint8_t num)
{
	uint32_t idx = 0;          // 数组写入变量；
	float Y;

	// ptr 写入
	for( idx = 0; idx <= num; idx++) {
		MEM_Write_F(INPUT_ADDR_PRT_SUB + (idx*4), ptr[idx]);	//数组数据写入； 				
	}
	// num 写入
	MEM_Write_UINT32(INPUT_ADDR_NUM_SUB, num);	// 数组长度写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_SUB_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_SUB_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}
/*
	软件计算PRDCT，数组求积；
	para ptr: 数组首地址；
	para num: 需要计算的数组长度；
*/
float Software_PRDCT(float * ptr, uint8_t num)	
{
	float product = ptr[0];
	uint8_t i = 0;
	
	for( i = 0; i<num; i++){
		product *= ptr[i+1];
	}
	return product;
}
/*
	DSP计算PRDCT，多项式求积；

	固定寄存器：
	product ：R0
	i       ：R1
	ptr     ：R2
	num     ：R3
	
	临时寄存器
	R4；
	R5；
*/
void DSP_PRDCT_Process(void)
{
	uint32_t for_big_temp_addr;
	
	/* 初始化数据存储地址,常量写入*/ 
	MEM_Write_UINT32(VAR_ADDR_1_PRO, INPUT_ADDR_PRT_PRO);	// 数组地址写入；
	
	/* init，初始值装载到寄存器中； 注：初始值存储可以使用内存指令直接装载到寄存器中。 */ 
	// Product = R0 = ptr[0]; 
	// i       = R1 = 0; 写入立即数0；
	// ptr     = R2 = VAR_ADDR_1;
	// num     = R3 = ; 写入立即数num; 
	R_LOAD_R(DSP_R1, 0);	         		     // i = 0;  
	M_LOAD_M(DSP_R2, VAR_ADDR_1_PRO, 0);      // R2 存入数组首地址ptr;
	M_LOAD_M(DSP_R3, INPUT_ADDR_NUM_PRO, 0);  // R3 = num;	
	M_LOAD_M(DSP_R0, DSP_R2, 1); 	 		 // ptr值装载到R0；	
		
	/* for 循环，大于比较 */
	for_big_temp_addr = R_CMP(DSP_R3, DSP_R1, 0, 2);  // num > i     
	R_GOTO(for_big_temp_addr + 12*2);    		// 不满足，向下GOTO到“==”比较 ~~~~
	
	/* 计算 */
	// ptr地址递增
	R_LOAD_R(DSP_R4, 4);             	 // PRT的地址递增常量4;	
	R_ADD_R(DSP_R2, DSP_R4, DSP_R2);  	 // ptr++; 地址递增4；    
	
	// 数组地址写入R2
	M_LOAD_M(DSP_R4, DSP_R2, 1); 	   // ptr值装载到R4；
	F_FMUL(DSP_R0, DSP_R4, DSP_R0);  // Product = Product * ptr;  
	
	// i++;
	R_LOAD_R(DSP_R4, 1);             	 // i 的递增常量1;	
	R_ADD_R(DSP_R1, DSP_R4, DSP_R1);  	 // i = i+1;  
	R_GOTO(for_big_temp_addr);    	 //
	
	M_STR_M(DSP_R0, OUTPUT_ADDR_PRO_FUNC, 0);     // DONE DATA   
	E_DONE();
}

/*
	计算PRDCT()函数:连乘函数。
*/
float DSP_PRDCT(float *ptr, uint8_t num)
{
	uint32_t idx = 0;          // 数组写入变量；
	float Y;

	// ptr 写入
	for( idx = 0; idx <= num; idx++) {
		MEM_Write_F(INPUT_ADDR_PRT_PRO + (idx*4), ptr[idx]);	//数组数据写入； 				
	}
	// num 写入
	MEM_Write_UINT32(INPUT_ADDR_NUM_PRO, num);	// 数组长度写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_PRO_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_PRO_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}
/*
	软件计算SUM Square，数组平方和；
	para ptr: 数组首地址；
	para num: 需要计算的数组长度；
*/
float Software_SUMSQ(float * ptr, uint8_t num)	
{
	float sumsq = 0;
	uint8_t i = 0;
	
	for( i = 0; i <= num; i++){
		sumsq += ptr[i]*ptr[i];
	}
	return sumsq;
}
/*
	DSP计算SUMSQ，数组平方和；

	固定寄存器：
	sumsq   ：R0
	i       ：R1
	ptr     ：R2
	num     ：R3
	
	临时寄存器
	R4；
	R5；
*/
/*
	SUMSQ 地址
*/
void DSP_SUMSQ_Process(void)
{
	uint32_t for_big_temp_addr;
	uint32_t for_eq_temp_addr;
	
	/* 初始化数据存储地址,常量写入*/ 
	MEM_Write_F(CONS_ADDR_VAL_0_SUMSQ, 0);	 					// SUMSQ 初始化常量 = 0；
	MEM_Write_UINT32(VAR_ADDR_1_SUMSQ, INPUT_ADDR_PRT_SUMSQ);	// 数组地址写入；
	
	/* init，初始值装载到寄存器中； 注：初始值存储可以使用内存指令直接装载到寄存器中。 */ 
	// sumsq = R0 = 1; 
	// i       = R1 = 0; 写入立即数0；
	// ptr     = R2 = VAR_ADDR_1_SUMSQ;
	// num     = R3 = num; 写入立即数num;
	M_LOAD_M(DSP_R0, CONS_ADDR_VAL_0_SUMSQ, 0); // sumsq = 0;	 
	R_LOAD_R(DSP_R1, 0);	         			   // i = 0;  !!! 改为从1开始，因为比较的符号由"<"变成">"！！！  64
	M_LOAD_M(DSP_R2, VAR_ADDR_1_SUMSQ, 0);      // R2 存入数组首地址ptr;	66
	M_LOAD_M(DSP_R3, INPUT_ADDR_NUM_SUMSQ, 0);  // R3 = num;	 70

	/* for 循环，大于比较 */
	for_big_temp_addr = R_CMP(DSP_R3, DSP_R1, 0, 2);  // num > i     
	R_GOTO(for_big_temp_addr + 13*2);    		// 不满足，向下GOTO到“==”比较 ~~~~
	
	/* 计算 */
	M_LOAD_M(DSP_R4, DSP_R2, 1); 	  	// ptr值装载到R4；	74
	F_FMUL(DSP_R4, DSP_R4, DSP_R4); // ptr = ptr*ptr;
	F_FADD(DSP_R0, DSP_R4, DSP_R0); // sumsq = sumsq + ptr*ptr;
	
	// ptr地址递增
	R_LOAD_R(DSP_R4, 4);               // PRT的地址递增常量4;	 80
	R_ADD_R(DSP_R2, DSP_R4, DSP_R2); // ptr++; 地址递增4；    82
	
	// i++;
	R_LOAD_R(DSP_R4, 1);               // i 的递增常量1;	84
	R_ADD_R(DSP_R1, DSP_R4, DSP_R1); // i = i+1;  86
	R_GOTO(for_big_temp_addr);   // 90

	/* for 循环，等于比较 */
	for_eq_temp_addr = R_CMP_EQ(DSP_R3, DSP_R1, 0, 2);   // num > i      1C  
	R_GOTO(for_eq_temp_addr + 6*2);    		   // 1E
	
	/* 计算 */
	M_LOAD_M(DSP_R4, DSP_R2, 1); 	  	// ptr值装载到R4；	74	
	F_FMUL(DSP_R4, DSP_R4, DSP_R4); // ptr = ptr*ptr;
	F_FADD(DSP_R0, DSP_R4, DSP_R0); // sumsq = sumsq + ptr*ptr;
	
	M_STR_M(DSP_R0, OUTPUT_ADDR_SUMSQ_FUNC, 0); // DONE DATA   94
	E_DONE();
}

/*
	计算平方和。
*/
float DSP_SUMSQ(float *ptr, uint8_t num)
{
	uint32_t idx = 0;          // 数组写入变量；
	float Y;
	
	for( idx = 0; idx <= num; idx++) {
		MEM_Write_F(INPUT_ADDR_PRT_SUMSQ + (idx*4), ptr[idx]);	//数组数据写入； 				
	}
	MEM_Write_UINT32(INPUT_ADDR_NUM_SUMSQ, num);	// 数组长度写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_SUMSQ_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_SUMSQ_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}


/*
	软件计算DOT，向量点乘；
	para ptr1: 数组1首地址；
	para ptr2: 数组2首地址；
	para num: 需要计算的数组长度；
	out  dot: 点乘结果
*/
float Software_DOT(float * ptr1, float * ptr2, uint8_t num)	
{
	float dot = 0;
	uint8_t i = 0;
	
	for( i = 0; i <= num; i++){
		dot += ptr1[i]*ptr2[i];
	}
	return dot;
}
/*
	DSP计算DOT

	固定寄存器：
	dot     ：R0
	i       ：R1
	ptr1    ：R2
	num     ：R3
	ptr2    ：R6
	
	临时寄存器
	R4；
	R5；
*/
/*
	dot 地址
*/
void DSP_DOT_Process(void)
{
	uint32_t for_big_temp_addr;
	uint32_t for_eq_temp_addr;
	
	/* 初始化数据存储地址,常量写入*/ 
	MEM_Write_F(CONS_ADDR_VAL_0_DOT, 0);	 			    // dot 初始化常量 = 0；
	MEM_Write_UINT32(VAR_ADDR_1_DOT, INPUT_ADDR_PRT1_DOT);	// 数组1首地址写入；
	MEM_Write_UINT32(VAR_ADDR_2_DOT, INPUT_ADDR_PRT2_DOT);	// 数组2首地址写入；
	
	/* init，初始值装载到寄存器中； 注：初始值存储可以使用内存指令直接装载到寄存器中。 */ 
	// dot  = R0 = 1; 
	// i    = R1 = 0; 写入立即数0；
	// ptr1 = R2 = VAR_ADDR_1_DOT;
	// num  = R3 = num; 写入立即数num;
	// ptr2 = R6 = VAR_ADDR_2_DOT;	
	M_LOAD_M(DSP_R0, CONS_ADDR_VAL_0_DOT, 0); // dot = 0;
	R_LOAD_R(DSP_R1, 0);	         			 // i = 0; 
	M_LOAD_M(DSP_R2, VAR_ADDR_1_DOT, 0);      // R2 存入数组首地址ptr1;	
	
	M_LOAD_M(DSP_R6, VAR_ADDR_2_DOT, 0);      // R6 存入数组首地址ptr2;	
	M_LOAD_M(DSP_R3, INPUT_ADDR_NUM_DOT, 0);  // R3 = num;	 
	
	/* for 循环，大于比较 */
	for_big_temp_addr = R_CMP(DSP_R3, DSP_R1, 0, 2);  // num > i    
	R_GOTO(for_big_temp_addr + 16*2);    		// 不满足，向下GOTO到“==”比较 ~~~~

	/* 计算 */
	M_LOAD_M(DSP_R4, DSP_R2, 1);  // ptr1值装载到R4；	
	M_LOAD_M(DSP_R5, DSP_R6, 1);  // ptr2值装载到R5；		
	F_FMUL(DSP_R4, DSP_R5, DSP_R4);  // ptr1*ptr2;
	F_FADD(DSP_R0, DSP_R4, DSP_R0);  // dot = dot + ptr1*ptr2;
	
	// ptr地址递增
	R_LOAD_R(DSP_R4, 4);             	 // PRT的地址递增常量4;	
    R_ADD_R(DSP_R2, DSP_R4, DSP_R2);  	 // ptr1++; 地址递增4； 
	R_ADD_R(DSP_R6, DSP_R4, DSP_R6);  	 // ptr2++; 地址递增4； 
	
	// i++;
	R_LOAD_R(DSP_R4, 1);             	 // i 的递增常量1;	84
	R_ADD_R(DSP_R1, DSP_R4, DSP_R1);  	 // i = i+1;  86
	R_GOTO(for_big_temp_addr);    	 // 90
	
	/* for 循环，等于比较 */
	for_eq_temp_addr = R_CMP_EQ(DSP_R3, DSP_R1, 0, 2);   // num > i     
	R_GOTO(for_eq_temp_addr + 9*2);    		   //  		     
	
	/* 计算 */
	M_LOAD_M(DSP_R4, DSP_R2, 1);  // ptr1值装载到R4；	
	M_LOAD_M(DSP_R5, DSP_R6, 1);  // ptr2值装载到R5；		
	
	F_FMUL(DSP_R4, DSP_R5, DSP_R4);  // ptr1*ptr2;
	F_FADD(DSP_R0, DSP_R4, DSP_R0);  // dot = dot + ptr1*ptr2;
	
	M_STR_M(DSP_R0, OUTPUT_ADDR_DOT_FUNC, 0);  // DONE DATA  
	E_DONE();
}

/*
	计算DOT，向量点乘；
	para ptr1: 数组1首地址；
	para ptr2: 数组2首地址；
	para num: 需要计算的数组长度；
	out    Y: 点乘结果	
*/
float DSP_DOT(float *ptr1, float *ptr2, uint8_t num)
{
	uint32_t idx = 0;          // 数组写入变量；
	float Y;
	
	for( idx = 0; idx <= num; idx++) {
		MEM_Write_F(INPUT_ADDR_PRT1_DOT + (idx*4), ptr1[idx]);	//数组数据写入； 				
	}
	for( idx = 0; idx <= num; idx++) {
		MEM_Write_F(INPUT_ADDR_PRT2_DOT + (idx*4), ptr2[idx]);	//数组数据写入； 				
	}	
	MEM_Write_UINT32(INPUT_ADDR_NUM_DOT, num);	// 数组长度写入；

//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_DOT_FUNC_ADDR);

	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {	
			MEM_Read(OUTPUT_ADDR_DOT_FUNC, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}

/* Part3: 三角函数 ********************************************************************/
/*
	绝对值函数；
*/
double myabs(double num1)
{
    return((num1 > 0) ? num1 : -num1);
}

/*
	软件计算sin()的泰勒展开；
*/
float Software_Sin(float num2)
{
	float dat =  fmodf(num2, 6.2831852);		
    int i = 1, negation = 1; // 取反
    float sum;
    float index = dat;   // 指数
    float Factorial = 1;  // 阶乘
    float TaylorExpansion = dat; // 泰勒展开式求和
	
    do
    {
        Factorial = Factorial * (i + 1) * (i + 2);//求阶乘
        index *= dat * dat;  //求num2的次方
        negation = -negation;  //每次循环取反
        sum = index / Factorial * negation;
        TaylorExpansion += sum;
        i += 2;
    } while (myabs(sum) > 1e-8);
    return(TaylorExpansion);
}


/*
	软件计算COS；
	使用sin计算cos;
*/
float Software_Cos(float num2)	
{
	return Software_Sin(num2+3.1415926/2);
}

/*
	软件计算tan；
	使用sin计算cos;
*/
float Software_tan(float num2)	
{
	return Software_Sin(num2) / Software_Cos(num2);
}

/*
	   DSP计算sin()的泰勒展开；
	// in    MEM addr = 0x1C;
	// out   MEM addr = 0x1B;
	// start MEM addr = 0X2001806c;

	//	Cos 计算用SIN；
	//	in  MEM addr = 0x20018070;
	//	out MEM addr = 0X2001806c;

	// 相关变量定义；
	// Factorial       = R0 = 1  // 阶乘
	// i               = R1 = 1  // 阶乘递增临时参数  
	// INDEX           = R2 ;    // 指数
	// negation        = R3 = 1  // 取反
	// sum             = R4      // 单项求和
	// TaylorExpansion = R5      // 泰勒展开式求和


	实现思路：
	C实现有几个变量，就对应几个寄存器； 例如，Factorial，i, INDEX等分别对应一个寄存器R0，R1，R2；
	所有的常量，初始量都要存到MEM中，用的时候再取；

	注意：每一个写入的立即数都得转化为浮点数；
	立即数操作一般用来实现循环；
	
	执行循环操作、跳转操作，需要知道相对地址；所以知道相对地址是很必要的。
	常量全部存储到MEM中，方便使用。
	
*/
void DSP_Taylor_Sin_Process(void)
{	
	uint32_t goto_temp_addr;
	uint32_t while_temp_addr;	
	
	/* 初始化数据存储地址,常量写入*/ 
	MEM_Write_F(NEG_CONS_CONS_ADDR,-1);
	MEM_Write_F(MINI_CONS_ADDR, 1e-8);	
	MEM_Write_F(FRAC_CONS_ADDR, 1);	 // Factorial = 1;
	MEM_Write_F(I_CONS_ADDR, 1);	         // i  = 1;
	MEM_Write_F(CONS0_ADDR, 1);	         // Negation   = 1;
	MEM_Write_F(CONS1_ADDR, 1);	         // 阶乘常量1  = 1;	
	MEM_Write_F(CONS2_ADDR, 2);	 		 // 阶乘常量2  = 2;	
	
	/* IN值输入，装载到寄存器R2，R5；*/
	M_LOAD_M(DSP_R2, INPUT_ADDR, 0);	
	M_LOAD_M(DSP_R5, INPUT_ADDR, 0);	
	
    /* init，初始值装载到寄存器中； 注：初始值存储可以使用内存指令直接装载到寄存器中。 */ 
	// Factorial = R0 = 1;
	// I         = R1 = 1;
	// Negation  = R3 = 1;
	M_LOAD_M(DSP_R0, FRAC_CONS_ADDR, 0);		
	M_LOAD_M(DSP_R1, I_CONS_ADDR, 0);			
	M_LOAD_M(DSP_R3, CONS0_ADDR, 0);		
	
	/* 循环，do while begin */ 
    // Factorial = Factorial * (i + 1) * (i + 2);
	// R0           R0             R6                   R7
	// R0 = R0 * (R1 + 1) * (R1 + 2);
	while_temp_addr = M_LOAD_M(DSP_R6, CONS1_ADDR, 0); // 记录跳转地址: while循环地址；
	F_FADD(DSP_R1, DSP_R6, DSP_R6); //R6 = (R1 + R6)   14
	M_LOAD_M(DSP_R7, CONS2_ADDR, 0);
	 
	F_FADD(DSP_R1, DSP_R7, DSP_R7); //R7 = (R1 + R7)   1a
	F_FMUL(DSP_R6, DSP_R7, DSP_R6); //R6 =  R6 * R7    1c 
	F_FMUL(DSP_R6, DSP_R0, DSP_R0); //R0 =  R6 * R0    1e

	// index *= num2 * num2;//求num2的次方
	// NUM2  = 0X20018270 = R6
    // R2 = R2 * R6 * R6
	M_LOAD_M(DSP_R6, INPUT_ADDR, 0); //R6 *0X20018270             20
	F_FMUL(DSP_R6, DSP_R6, DSP_R6); //R6 =  R6 * R6      22
	F_FMUL(DSP_R2, DSP_R6, DSP_R2);   //R2 =  R2 * R6      24
	
	// negation = -negation;//每次循环取反
    // R6 = R6 * R7
	M_LOAD_M(DSP_R7, NEG_CONS_CONS_ADDR, 0);  // R7 = * 0X2001821E  26	
	F_FMUL(DSP_R3, DSP_R7, DSP_R3); // R6 =  R7 * R6      28

	// sum = index / Factorial * negation;
	// sum = R4
    // R4    = R2 / R0 * R3 (0X2001821d)
	F_FDIV(DSP_R2 , DSP_R0, DSP_R7); //R7    = R2 / R0     2a
	F_FMUL(DSP_R3 , DSP_R7, DSP_R4); //R4    = R7 * R3     2c

	// TaylorExpansion = R5
	// TaylorExpansion += sum;
	F_FADD(DSP_R4, DSP_R5, DSP_R5);  //R5 = R4 + R5       2e
	
	// i += 2;
	M_LOAD_M(DSP_R7, CONS2_ADDR, 0);
	F_FADD(DSP_R1, DSP_R7, DSP_R1);  //R1 = (R1 + R7)   34

	// myabs(sum)
	R_LOAD_R(DSP_R6, 0);   //R6 = 0           36
	M_LOAD_M(DSP_R7, NEG_CONS_CONS_ADDR, 0);   //R7 = -1          38

	// (myabs(sum)
	F_FCMP(DSP_R4, DSP_R6, 0, 3);  // sum>0?  PC + 6  
	goto_temp_addr = F_FMUL(DSP_R4, DSP_R7, DSP_R6); 
	R_GOTO(goto_temp_addr + 4*2);  // 跳转到while比较区域，地址计算：
	R_LOAD(DSP_R6, DSP_R4);   // cmp > 0

	// while ( > 1e-15);
	M_LOAD_M(DSP_R7, MINI_CONS_ADDR, 0); // 执行比较操作，记录跳转地址
	F_FCMP(DSP_R7, DSP_R6, 0, 2);  // (R6) > 1e-15 
	R_GOTO(while_temp_addr);  // 跳转到while比较区域，地址计算：
	
	M_STR_M(DSP_R5, OUTPUT_ADDR, 0);   // DONE DATA    
	E_DONE();  // stop dsp     
}


/*
	DSP计算sin()函数；
*/
float DSP_SIN(float a)
{
	float dat =  fmodf(a, 6.2831852);	
	float Y;

	MEM_Write_F(INPUT_ADDR, dat);	// data 写入
	
//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_SIN_FUNC_ADDR);
	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {
			MEM_Read(OUTPUT_ADDR, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}
/*
	计算cos()函数；
*/
float DSP_COS(float a)
{
	float dat = fmodf(a+3.1415926/2, 6.2831852);	
	float Y;

	MEM_Write_F(INPUT_ADDR, dat);	// data 写入
	DSP_Start(START_SIN_FUNC_ADDR);
	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {
			MEM_Read(OUTPUT_ADDR, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}
/*
	计算cos()函数；
*/
float DSP_TAN(float a)
{
	float Y;

	Y = DSP_SIN(a)/DSP_COS(a);

	return Y;
}

/*
	软件计算asin()的泰勒展开；
	注意输入范围是-1~1；
*/
float Software_ASIN(float num2)
{		
	float result = num2;  		// 结果项
	float term = num2;		 	// 单项
	float xsquared = num2*num2; // 平方
    int n = 1;  // 项数
	
	 result = num2;  		// 结果项
	 term = num2;		 	// 单项
	 xsquared = num2*num2; // 平方
     n = 1;  // 项数
	
	while(myabs(term) > 1e-6) {
		term *= xsquared * (2 * n -1) / (2 * n);
		result += term/(2 * n + 1);
		n++;
		
	}
		
    return(result);
}


/*
	DSP计算ASIN()的泰勒展开；


	实现思路：
	注意：
		1、常量注意区分是浮点数还是整形。
		2、循环操作通常需要知道相对地址，可以从“本地址”开始，计算相对地址。
	
	
	步骤：
	1、寄存器分配；每一个局部变量分配一个“固定”寄存器， 多个常量分配1个寄存器。
		示例：
		// 相关变量定义；
		// result    = R0 = 1  // 结果项
		// term      = R1 = 1  // 单项
		// xsquared  = R2 ;    // 平方
		// n         = R3 = 1  // 项数

		临时寄存器：R4，R5；
	2、常量存储；每一个常量分配一个“固定”地址，供之后装载到寄存器中。
		示例：
		CONS1_ADDR： 		1；
		CONS2_ADDR： 		2；		
		MINI_CONS_ADDR		1e-6;
		
		
	3、初始化；
	
*/
void DSP_Taylor_ASIN_Process(void)
{	
	uint32_t goto_temp_addr;
	uint32_t while_temp_addr;

	/* 初始化数据存储地址,常量写入*/ 
	MEM_Write_F(CONS0_ADDR_ASIN_F, 0);	         // 常量0 = 0;	
	MEM_Write_F(CONS1_ADDR_ASIN_F, 1);	         // 常量1 = 1;
	MEM_Write_F(CONS2_ADDR_ASIN_F, 2);	 		 // 常量2 = 2;		
	MEM_Write_F(MINI_CONS_ADDR_ASIN_F, 1e-6);	 // 最小常量 = 1e-6;			
	MEM_Write_F(NEG_CONS_ADDR_ASIN_F, -1);	     // 常量-1 = -1;			
	
    /* init，初始值装载到寄存器中； */ 
	M_LOAD_M(DSP_R0, INPUT_ADDR_ASIN_F, 0);  // result = num2
	M_LOAD_M(DSP_R1, INPUT_ADDR_ASIN_F, 0);  // term = num2;	
	M_LOAD_M(DSP_R3, CONS1_ADDR_ASIN_F, 0);  // n = 1;	
	F_FMUL(DSP_R0, DSP_R1, DSP_R2); // xsquared = num2*num2；

//	while(myabs(term) > 1e-6) {
//		term *= xsquared * (2 * n -1) / (2 * n);
//		result += term/(2 * n + 1);
//		n++;
//	}
	
	// myabs(term)
	while_temp_addr = M_LOAD_M(DSP_R6, CONS0_ADDR_ASIN_F, 0);  // R6 = 0；    OE  
	M_LOAD_M(DSP_R7, NEG_CONS_ADDR_ASIN_F, 0); // R7 = -1   12   // ok
	
	// (myabs(term)
	F_FCMP(DSP_R1, DSP_R6, 0, 3);   // term>0?  跳转到while()处； 16
	goto_temp_addr  = F_FMUL(DSP_R1, DSP_R7, DSP_R1);   //     18            
	R_GOTO(goto_temp_addr + 3*2);   // 跳转到while比较区域，1A
	
	// while ( > 1e-6);
	M_LOAD_M(DSP_R7, MINI_CONS_ADDR_ASIN_F, 0); // 执行比较操作，记录跳转地址 1E
	F_FCMP(DSP_R1, DSP_R7, 0, 2);          // myabs(term) > 1e-6   22
	R_GOTO(while_temp_addr + 28*2);  	   // 跳到结束地址  24
	
	// term *= xsquared * (2 * n -1) / (2 * n);
	M_LOAD_M(DSP_R4, CONS2_ADDR_ASIN_F, 0);  // R4 = 2；  28 	
	F_FMUL(DSP_R3, DSP_R4, DSP_R5);    // 2 * n;    2C
	M_LOAD_M(DSP_R4, CONS1_ADDR_ASIN_F, 0);  // R4 = 1；  2E     	
	F_FSUB(DSP_R5, DSP_R4, DSP_R6);    // 2n-1;     32
	F_FDIV(DSP_R6, DSP_R5, DSP_R7);    // (2*n-1) / 2 * (n); 34
	F_FMUL(DSP_R2, DSP_R7, DSP_R6);    //  xsquared * (2 * n -1) / (2 * n); 36
	F_FMUL(DSP_R1, DSP_R6, DSP_R1);    // term *= xsquared * (2 * n -1) / (2 * n); 38
	
	// result += term/(2 * n + 1);
	F_FADD(DSP_R5, DSP_R4, DSP_R6);    // (2 * n + 1); 3A
	F_FDIV(DSP_R1, DSP_R6, DSP_R7);    // term/(2 * n + 1); 3C
	F_FADD(DSP_R0, DSP_R7, DSP_R0);    // result += term/(2 * n + 1); 3E

	// n++;
	F_FADD(DSP_R3, DSP_R4, DSP_R3);    // n++;  40
	R_GOTO(while_temp_addr);  // 跳转到回去，重新循环  42
	
	// 结束处
	M_STR_M(DSP_R0, OUTPUT_ADDR_ASIN_F, 0);  // DONE DATA  46  
	E_DONE();
}

	
/*
	计算ASIN()函数；
*/
float DSP_ASIN(float a)
{
	float Y;

	MEM_Write_F(INPUT_ADDR_ASIN_F, a); 	// data 写入
	
//	DSP_Debug_CMD(ENABLE);
	DSP_Start(START_ASIN_F_ADDR);
	while(1){
		if((DSP->CR & DSP_CR_DONE_STAT) == DSP_CR_DONE_STAT) {
			MEM_Read(OUTPUT_ADDR_ASIN_F, &Y);

			DSP->CR &= ~(DSP_CR_START | DSP_CR_STOP); // clear start or stop config;
			break;
		}
	}
	return Y;
}

void DSP_Space_Clear(void)
{
	uint32_t idx = 0;
	
	for(idx = 0; idx < 0x8000; idx += 4) {
		*(__IO uint32_t*)(0x20008000 + idx) = 0;
	}
	
	for(idx = 0; idx < 0x400; idx += 4) {
		*(__IO uint32_t*)(0x20018000 + idx) = 0;
	}	
}

#if RAM32K_MODE

/*
	预先写入DSP函数；
	调用时直接向INPUT地址写内容即可。
*/ 
void DSP_FUNC_Write(void)
{
	DSP_Space_Clear();  // 清除DSP区域残留的数据和指令

	dsp_addr = DSP_BASE_ADDR + START_SIN_FUNC_ADDR;
	DSP_Taylor_Sin_Process();	
	
	dsp_addr = DSP_BASE_ADDR + START_SUM_FUNC_ADDR;	
	DSP_SUM_Process();
	
	dsp_addr = DSP_BASE_ADDR + START_SUB_FUNC_ADDR;	
	DSP_SUB_Process();	
	
	dsp_addr = DSP_BASE_ADDR + START_PRO_FUNC_ADDR;	
	DSP_PRDCT_Process();	
	
	dsp_addr = DSP_BASE_ADDR + START_SUMSQ_FUNC_ADDR;		
	DSP_SUMSQ_Process();

	dsp_addr = DSP_BASE_ADDR + START_DOT_FUNC_ADDR;		
	DSP_DOT_Process();
	
	dsp_addr = DSP_BASE_ADDR + START_I2F_FUNC_ADDR;			
	DSP_I2F_Process();
	
	dsp_addr = DSP_BASE_ADDR + START_F2I_FUNC_ADDR;			
	DSP_F2I_Process();

	dsp_addr = DSP_BASE_ADDR + START_CMP_F_FUNC_ADDR;			
	DSP_CMP_F_Process();

	dsp_addr = DSP_BASE_ADDR + START_DIV_F_FUNC_ADDR;			
	DSP_DIV_F_Process();

	dsp_addr = DSP_BASE_ADDR + START_INVSQRT_F_FUNC_ADDR;			
	DSP_INVSQRT_F_Process();

	dsp_addr = DSP_BASE_ADDR + START_MAC_F_FUNC_ADDR;			
	DSP_MAC_F_Process();
	
	dsp_addr = DSP_BASE_ADDR + START_ASIN_F_ADDR;				
	DSP_Taylor_ASIN_Process();
}

#else

void DSP_FUNC_Write(void)
{
	DSP_Space_Clear();  // 清除DSP区域残留的数据和指令

	dsp_addr = DSP_BASE_ADDR + START_SIN_FUNC_ADDR;
	DSP_Taylor_Sin_Process();	
	
//	dsp_addr = DSP_BASE_ADDR + START_SUM_FUNC_ADDR;	
//	DSP_SUM_Process();
	
//	dsp_addr = DSP_BASE_ADDR + START_SUB_FUNC_ADDR;	
//	DSP_SUB_Process();	
	
//	dsp_addr = DSP_BASE_ADDR + START_PRO_FUNC_ADDR;	
//	DSP_PRDCT_Process();	
	
//	dsp_addr = DSP_BASE_ADDR + START_SUMSQ_FUNC_ADDR;		
//	DSP_SUMSQ_Process();

//	dsp_addr = DSP_BASE_ADDR + START_DOT_FUNC_ADDR;		
//	DSP_DOT_Process();
	
//	dsp_addr = DSP_BASE_ADDR + START_I2F_FUNC_ADDR;			
//	DSP_I2F_Process();
	
//	dsp_addr = DSP_BASE_ADDR + START_F2I_FUNC_ADDR;			
//	DSP_F2I_Process();

//	dsp_addr = DSP_BASE_ADDR + START_CMP_F_FUNC_ADDR;			
//	DSP_CMP_F_Process();

//	dsp_addr = DSP_BASE_ADDR + START_DIV_F_FUNC_ADDR;			
//	DSP_DIV_F_Process();

//	dsp_addr = DSP_BASE_ADDR + START_INVSQRT_F_FUNC_ADDR;			
//	DSP_INVSQRT_F_Process();

//	dsp_addr = DSP_BASE_ADDR + START_MAC_F_FUNC_ADDR;			
//	DSP_MAC_F_Process();
	
//	dsp_addr = DSP_BASE_ADDR + START_ASIN_F_ADDR;				
//	DSP_Taylor_ASIN_Process();	
	
}
#endif
