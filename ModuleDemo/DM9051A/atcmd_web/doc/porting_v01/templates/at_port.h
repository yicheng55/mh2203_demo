/**
 * @file    at_port.h
 * @brief   AT_Command 平台抽象層 (HAL) 介面 —— 跨平台移植範本
 *
 * 這是「範本檔」。複製到新平台專案的 port/ 目錄使用。
 * 原 AT32F413 專案不引用此檔，故對既有韌體零影響。
 *
 * 對應文件：doc/porting_v01/02-at-port-hal-spec.md
 * 用途：把 AT_Command 對「UART / 非揮發儲存 / 系統重置 / 計時」的相依，
 *       全部收斂到這組函式。新平台只需實作 at_port.c (見 at_port_template.c)。
 */
#ifndef __AT_PORT_H__
#define __AT_PORT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 1. UART (命令 / 資料介面)
 *    取代原本：u_writeuart1 / USART_SendData / UART_CMD_Init_Update
 * ============================================================ */

/** 初始化命令 UART 參數。對應原 UART_CMD_Init_Update()。 */
void         atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop);

/** 阻塞送出單一字元 (送完才回)。對應原 u_writeuart1()。 */
void         atp_uart_putc(char c);

/** 回傳目前命令列 RX 緩衝指標 (原 g_u8RecData)。 */
char        *atp_uart_rx_buf(void);

/** 回傳目前已接收位元組數 (原 gt_u32comRbytes)。 */
unsigned int atp_uart_rx_len(void);

/** 清空 RX 緩衝/計數/就緒旗標 (一行命令處理完呼叫)。 */
void         atp_uart_rx_clear(void);

/** 是否已收到一整行命令 (遇 '\n')。對應原 atcmd_flag。回傳非 0 表就緒。 */
int          atp_uart_rx_ready(void);

/* ============================================================
 * 2. 非揮發儲存 (NV) —— word(4 byte) 序列化
 *    取代原本：FLASH_Unlock/ErasePage/ProgramWord/Lock + Read_AT_FlashDWord
 *    note: 沿用 dataflash.c 既有的 word 版面，只換底層媒介。
 *          idx 為相對 word 索引 (0 = 設定區起點)。
 * ============================================================ */

/** 抹除整個設定區 (原 Unlock + ClearFlag + ErasePage)。 */
void         atp_nv_erase(void);

/** 寫入第 idx 個 word (原 FLASH_ProgramWord(StartAddress + idx*4, word))。 */
void         atp_nv_write_word(uint32_t idx, uint32_t word);

/** 讀回第 idx 個 word (原 Read_AT_FlashDWord)。 */
uint32_t     atp_nv_read_word(uint32_t idx);

/** 提交/上鎖 (原 FLASH_Lock；即時寫入的媒介可為空實作)。 */
void         atp_nv_commit(void);

/* ============================================================
 * 3. 系統控制
 *    取代原本：atcmd_rst() 內的 __asm / IAP 跳轉 (Keil 專屬，移植時整段刪除)
 * ============================================================ */

/** 軟體重置 MCU。Cortex-M 預設可用 NVIC_SystemReset()。 */
void         atp_system_reset(void);

/* ============================================================
 * 4. 計時 (選用；原模組少用，可回傳 0)
 * ============================================================ */

/** 毫秒級 tick，供 keepalive / 逾時自管使用。 */
uint32_t     atp_mstick(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_PORT_H__ */
