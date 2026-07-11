/**
 * @file    at_port_template.c  (複製到新平台後改名為 at_port.c)
 * @brief   AT_Command 平台抽象層實作「空殼」—— 跨平台移植範本
 *
 * 每個函式都有：
 *   - TODO: 平台實作說明
 *   - [AT32 對照]：原 AT32F413 的等價寫法 (供參考)
 *   - [範例]：常見平台 (STM32 HAL / lwIP host / PC) 的填法
 *
 * 對應文件：doc/porting_v01/02-at-port-hal-spec.md
 * 原 AT32F413 專案不引用此檔。
 */
#include "at_port.h"
#include <string.h>

/* ------------------------------------------------------------
 * 1. UART
 * ------------------------------------------------------------ */

/* 沿用原模組的全域 RX 三件套語意。移植時可讓平台的 UART RX ISR/輪詢
 * 直接填這三個變數 (與原 atcommand.c 一致)，本檔只是包一層存取器。
 * 若你選擇在 atcommand.c 內保留這些全域，則此處改用 extern 引用即可。 */
#ifndef AT_RX_BUF_SIZE
#define AT_RX_BUF_SIZE 1460   /* = RecData_Size (atcommand.h) */
#endif
static char         s_rx_buf[AT_RX_BUF_SIZE];
static unsigned int s_rx_len = 0;
static int          s_rx_ready = 0;

void atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
    /* TODO: 用目標 HAL 設定 UART 參數。
     * [AT32 對照] UART_CMD_Init_Update(baud, wordlen, parity, stop);
     * [範例 STM32 HAL]
     *   huart2.Init.BaudRate = baud; ... HAL_UART_Init(&huart2);
     */
    (void)baud; (void)wordlen; (void)parity; (void)stop;
}

void atp_uart_putc(char c)
{
    /* TODO: 阻塞送出一個 byte。
     * [AT32 對照]
     *   USART_SendData(USART2, (uint8_t)c);
     *   while (USART_GetFlagStatus(USART2, USART_FLAG_TRAC) == RESET);
     * [範例 STM32 HAL]
     *   HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
     * [範例 PC] putchar(c);
     */
    (void)c;
}

char        *atp_uart_rx_buf(void)  { return s_rx_buf; }
unsigned int atp_uart_rx_len(void)  { return s_rx_len; }
int          atp_uart_rx_ready(void){ return s_rx_ready; }

void atp_uart_rx_clear(void)
{
    memset(s_rx_buf, 0, sizeof(s_rx_buf));
    s_rx_len = 0;
    s_rx_ready = 0;
}

/* 由平台 UART RX ISR/輪詢呼叫 (非 at_port.h 對外介面，按需提供)：
 * 收到一個字元就 append；遇 '\n' 設 ready。 */
void atp_uart_rx_feed(char c)
{
    if (s_rx_len < sizeof(s_rx_buf) - 1)
        s_rx_buf[s_rx_len++] = c;
    if (c == '\n')
        s_rx_ready = 1;
}

/* ------------------------------------------------------------
 * 2. 非揮發儲存 (NV) —— word 序列化
 * ------------------------------------------------------------ */

/* [範例] 無 flash 平台：RAM 鏡像 + 檔案/EEPROM 持久化。
 * 換成真 flash 時，把 nv[] 換成 FLASH_ProgramWord/Read。 */
#ifndef AT_NV_WORDS
#define AT_NV_WORDS 64
#endif
static uint32_t s_nv[AT_NV_WORDS];

void atp_nv_erase(void)
{
    /* TODO: 抹除設定區。
     * [AT32 對照]
     *   FLASH_Unlock();
     *   FLASH_ClearFlag(FLASH_FLAG_PRGMFLR | FLASH_FLAG_PRCDN);
     *   FLASH_ErasePage(StartAddress);   // 0x08020000 - 2KB
     * 注意：確保抹除範圍涵蓋整個設定區、且不誤刪程式碼。 */
    memset(s_nv, 0xFF, sizeof(s_nv));
}

void atp_nv_write_word(uint32_t idx, uint32_t word)
{
    /* TODO: [AT32 對照] FLASH_ProgramWord(StartAddress + idx*4, word); */
    if (idx < AT_NV_WORDS) s_nv[idx] = word;
}

uint32_t atp_nv_read_word(uint32_t idx)
{
    /* TODO: [AT32 對照] return Read_AT_FlashDWord(idx); */
    return (idx < AT_NV_WORDS) ? s_nv[idx] : 0xFFFFFFFFu;
}

void atp_nv_commit(void)
{
    /* TODO: [AT32 對照] FLASH_Lock();
     * [範例] 若用檔案：把 s_nv[] fwrite 到檔案。即時寫入媒介可留空。 */
}

/* ------------------------------------------------------------
 * 3. 系統控制
 * ------------------------------------------------------------ */

void atp_system_reset(void)
{
    /* TODO:
     * [Cortex-M 通用] NVIC_SystemReset();
     * [PC/Linux]      exit(0);
     * 原 atcmd_rst() 的 __asm MSR_MSP / INTX_DISABLE / IAP 跳轉整段刪除。 */
    for (;;) { /* 安全停滯，避免未實作時跑飛 */ }
}

/* ------------------------------------------------------------
 * 4. 計時 (選用)
 * ------------------------------------------------------------ */

uint32_t atp_mstick(void)
{
    /* TODO: [範例 STM32 HAL] return HAL_GetTick(); */
    return 0;
}
