/**
 * @file    at_port.c
 * @brief   AT_Command 平台抽象層 (HAL) — MH2203 實作
 *
 * 全部接上 MH2203 既有的板級驅動，不新增任何底層週邊初始化：
 *   - UART TX：直接呼叫 mh2203_board.c 既有的 SER_PutChar() (USART2)
 *   - UART RX：USART2 RX 中斷 (WEAK default，目前沒有任何檔案 override，
 *     新增這裡不影響既有 DM9051A_mh2203_uip.uvprojx 建置)
 *   - NV：MH22xxLib 既有的 FLASH_* API，寫在全 flash 最後一頁 (2KB)
 *   - Reset：NVIC_SystemReset (CMSIS)
 *   - Tick：既有 mh2203_uip_millis() (SysTick 已在跑，供 uIP 使用)
 */
#include "at_port.h"
#include "mh2203_platform.h"
#include "mh2203_uip_clock.h"
#include "atcommand.h" /* g_u8RecData / gt_u32comRbytes / atcmd_flag 三件套 */

#include <string.h>

/* mh2203_board.c 內定義、供除錯 printf 使用的阻塞式 UART 送字元函式；
 * 該檔未於 header 宣告，這裡補一個 extern 引用，不修改 mh2203_board.c。 */
extern int SER_PutChar(int ch);

/* ------------------------------------------------------------
 * 1. UART
 *
 * RX 三件套 (g_u8RecData/gt_u32comRbytes/atcmd_flag) 直接沿用 atcommand.c
 * 定義的全域變數 (対照 templates/at_port_template.c 的建議做法："最省事：
 * 讓這三個全域仍在 atcommand.c，平台 ISR 直接填它們，atp_uart_rx_* 只是包一層
 * 存取器")。之前誤把 ISR 寫進一份 at_port.c 私有緩衝，導致 at_cmdProcess()
 * 讀到的 atcmd_flag 永遠是 FALSE、UART 指令沒反應——已修正為直接寫入這三個
 * 真正的全域。
 * ------------------------------------------------------------ */

static int s_irq_configured = 0;

static void at_port_uart_irq_enable(void)
{
    NVIC_InitTypeDef nvic;

    if (s_irq_configured) {
        return;
    }
    s_irq_configured = 1;

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    nvic.NVIC_IRQChannel = USART2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 2;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop)
{
    USART_InitTypeDef usart;

    /* USART2 的 GPIO/時脈已由 mh2203_uip_board_init() -> mh2203_board.c 設定過，
     * 這裡只重設鮑率/字長/同位/停止位等通訊參數並開啟 RX 中斷。
     * wordlen/parity/stop 沿用 AT_Command 的既定編碼 (0=None,1=Odd,2=Even ; 1=1bit,2=2bit)。 */
    USART_StructInit(&usart);
    usart.USART_BaudRate = baud;
    usart.USART_WordLength = (wordlen == 9u) ? USART_WordLength_9b : USART_WordLength_8b;

    switch (parity) {
    case 1u:
        usart.USART_Parity = USART_Parity_Odd;
        break;
    case 2u:
        usart.USART_Parity = USART_Parity_Even;
        break;
    default:
        usart.USART_Parity = USART_Parity_No;
        break;
    }

    usart.USART_StopBits = (stop == 2u) ? USART_StopBits_2 : USART_StopBits_1;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);

    at_port_uart_irq_enable();
}

void atp_uart_putc(char c)
{
    (void)SER_PutChar((int)c);
}

char *atp_uart_rx_buf(void)
{
    return g_u8RecData;
}

unsigned int atp_uart_rx_len(void)
{
    return gt_u32comRbytes;
}

int atp_uart_rx_ready(void)
{
    return atcmd_flag;
}

void atp_uart_rx_clear(void)
{
    memset(g_u8RecData, 0, sizeof(g_u8RecData));
    gt_u32comRbytes = 0;
    atcmd_flag = FALSE;
}

/* USART2_IRQHandler 為 startup_mh22xx.s 內的 WEAK default，目前沒有其他檔案
 * override 它；在此提供簡單輪詢式 RX (逐字元 append 進 g_u8RecData，遇 '\n'
 * 設 atcmd_flag)，直接對齊 atcommand.c 原本的三件套語意，讓 at_cmdProcess()
 * 收得到資料。 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        char c = (char)USART_ReceiveData(USART2);

        if (gt_u32comRbytes < (sizeof(g_u8RecData) - 1u)) {
            g_u8RecData[gt_u32comRbytes++] = c;
        }
        if (c == '\n') {
            atcmd_flag = TRUE;
        }
    }
}

/* ------------------------------------------------------------
 * 2. 非揮發儲存 (NV) — word 序列化，寫在全 flash 最後一頁 (2KB)
 * ------------------------------------------------------------ */

/* 128KB flash (0x08000000-0x0801FFFF)，MH22xxLib 頁大小為 2KB，
 * 取最後一頁當設定區；dataflash.c 目前只用到 ~42 個 word，遠低於
 * 512 word 的頁容量。 */
#define AT_NV_PAGE_ADDR   0x0801F800u
#define AT_NV_WORDS_MAX   (2048u / 4u)

void atp_nv_erase(void)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR | FLASH_FLAG_EOP);
    (void)FLASH_ErasePage(AT_NV_PAGE_ADDR);
}

void atp_nv_write_word(uint32_t idx, uint32_t word)
{
    if (idx < AT_NV_WORDS_MAX) {
        (void)FLASH_ProgramWord(AT_NV_PAGE_ADDR + (idx * 4u), word);
    }
}

uint32_t atp_nv_read_word(uint32_t idx)
{
    if (idx < AT_NV_WORDS_MAX) {
        return *((volatile uint32_t *)(AT_NV_PAGE_ADDR + (idx * 4u)));
    }
    return 0xFFFFFFFFu;
}

void atp_nv_commit(void)
{
    FLASH_Lock();
}

/* ------------------------------------------------------------
 * 3. 系統控制
 * ------------------------------------------------------------ */

void atp_system_reset(void)
{
    NVIC_SystemReset();
}

/* ------------------------------------------------------------
 * 4. 計時
 * ------------------------------------------------------------ */

uint32_t atp_mstick(void)
{
    return mh2203_uip_millis();
}
