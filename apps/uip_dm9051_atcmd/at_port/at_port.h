/**
 * @file    at_port.h
 * @brief   AT_Command 平台抽象層 (HAL) — MH2203 實作
 *
 * 對應 porting_v01 範本 templates/at_port.h。把 AT_Command 對「UART / 非揮發
 * 儲存 / 系統重置 / 計時」的相依，收斂到這組函式，由 at_port.c 接上 MH2203
 * 既有的板級驅動 (mh2203_board.c / mh2203_uip_clock.c / MH22xxLib flash)。
 */
#ifndef AT_PORT_H
#define AT_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AT_Command/trans_level.c 內的除錯輸出巨集。AT32 原版預設也是空巨集
 * (SDK/User/System_Init.h `#define _printf(args...)`，除錯用版本被註解掉)，
 * 這裡維持同樣預設行為，不接任何 UART/UDP 輸出。 */
#ifndef _printf
#define _printf(...)
#endif

/* ============================================================
 * 1. UART (AT 指令介面，沿用 USART2)
 * ============================================================ */

/** 初始化/重設 AT 指令 UART 參數 (baud/wordlen/parity/stop)。 */
void         atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop);

/** 阻塞送出單一字元 (送完才回)。對應原 u_writeuart1()。 */
void         atp_uart_putc(char c);

/** 回傳目前命令列 RX 緩衝指標 (原 g_u8RecData)。 */
char        *atp_uart_rx_buf(void);

/** 回傳目前已接收位元組數 (原 gt_u32comRbytes)。 */
unsigned int atp_uart_rx_len(void);

/** 清空 RX 緩衝/計數/就緒旗標 (一行命令處理完呼叫)。 */
void         atp_uart_rx_clear(void);

/** 是否已收到一整行命令 (遇 '\n')。回傳非 0 表就緒。 */
int          atp_uart_rx_ready(void);

/* ============================================================
 * 2. 非揮發儲存 (NV) — word(4 byte) 序列化，沿用 dataflash.c 版面
 *    idx 為相對 word 索引 (0 = 設定區起點)。
 * ============================================================ */

/** 抹除整個設定區。 */
void         atp_nv_erase(void);

/** 寫入第 idx 個 word。 */
void         atp_nv_write_word(uint32_t idx, uint32_t word);

/** 讀回第 idx 個 word。 */
uint32_t     atp_nv_read_word(uint32_t idx);

/** 提交 (MH2203 為即時寫入，實作為空)。 */
void         atp_nv_commit(void);

/* ============================================================
 * 3. 系統控制
 * ============================================================ */

/** 軟體重置 MCU (NVIC_SystemReset)。 */
void         atp_system_reset(void);

/* ============================================================
 * 4. 計時
 * ============================================================ */

/** 毫秒級 tick，沿用既有 mh2203_uip_millis()。 */
uint32_t     atp_mstick(void);

#ifdef __cplusplus
}
#endif

#endif /* AT_PORT_H */
