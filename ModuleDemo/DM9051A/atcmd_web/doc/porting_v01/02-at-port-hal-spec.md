# 02 — `at_port.h` 平台 HAL 介面規格

> 對應範本：[`templates/at_port.h`](templates/at_port.h)、[`templates/at_port_template.c`](templates/at_port_template.c)
> 目的：定義 AT_Command 需要平台提供的**最小抽象介面**。把 [`01`](01-at-command-dependencies.md) 收斂出來的 UART / NV / Reset / 時間 四類相依，全部統一成這組函式；新平台只需實作 `at_port_template.c`。

---

## 1. 設計原則

- **薄**：只抽象「真的跟硬體綁定」的東西，AT 命令解析邏輯不動。
- **同步阻塞語意**：沿用原韌體（`u_writeuart1` 是 busy-wait），介面保持簡單，不引入回呼。
- **NV 用 word index**：沿用 `dataflash.c` 既有的 word 序列化版面，平台只換底層儲存媒介。
- **可先 stub**：每個函式都能先用空殼/PC 模擬編過，再逐步接真硬體。

---

## 2. UART 介面

| 函式 | 原型 | 取代原本 | 語意 |
|------|------|----------|------|
| 初始化 | `void atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop);` | `UART_CMD_Init_Update` | 設定命令/資料 UART 參數 |
| 送一字元 | `void atp_uart_putc(char c);` | `u_writeuart1` | 阻塞送出單一 byte（送完才回） |
| 取 RX 緩衝 | `char *atp_uart_rx_buf(void);` | 直接存取 `g_u8RecData` | 回傳目前命令列緩衝指標 |
| 取 RX 長度 | `unsigned int atp_uart_rx_len(void);` | `gt_u32comRbytes` | 已收位元組數 |
| 清 RX | `void atp_uart_rx_clear(void);` | 清 `g_u8RecData`+`gt_u32comRbytes`+`atcmd_flag` | 一行命令處理完後重置 |
| 查 RX 就緒 | `int atp_uart_rx_ready(void);` | `atcmd_flag` | 是否收到一整行（遇 `\n`） |

### 移植做法
- 最省事：移植時讓 `g_u8RecData` / `gt_u32comRbytes` / `atcmd_flag` 仍是全域，平台的 UART RX ISR/輪詢照原邏輯填它們；`atp_uart_rx_*` 只是包一層存取器。
- `u_writeuart1` 改成：
  ```c
  void u_writeuart1(char c){ atp_uart_putc(c); }
  ```

### AT32 對照（原實作）
```c
// atp_uart_putc 等價於：
USART_SendData(USART2, (uint8_t)c);
while (USART_GetFlagStatus(USART2, USART_FLAG_TRAC) == RESET);
```

### 範例填法（STM32 HAL）
```c
void atp_uart_putc(char c){
    HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
}
```

---

## 3. 非揮發儲存（NV）介面

對應 `dataflash.c` 的 word 序列化。**版面不變**，平台只實作這 4 個原語：

| 函式 | 原型 | 取代原本 |
|------|------|----------|
| 抹除 | `void atp_nv_erase(void);` | `FLASH_Unlock`+`ClearFlag`+`ErasePage` |
| 寫 word | `void atp_nv_write_word(uint32_t idx, uint32_t word);` | `FLASH_ProgramWord(StartAddress+idx*4, …)` |
| 讀 word | `uint32_t atp_nv_read_word(uint32_t idx);` | `Read_AT_FlashDWord` |
| 提交 | `void atp_nv_commit(void);` | `FLASH_Lock`（flash 即時寫則為空） |

> `idx` 是**相對 word 索引**（0 = 設定區起點），平台內部換算成自己的位址/offset。原本用負 offset（word -12…）描述版面；移植時改成 idx 0,1,2… 線性遞增即可，只要讀寫一致。

### AT32 對照（原實作）
```c
// atp_nv_erase:
FLASH_Unlock();
FLASH_ClearFlag(FLASH_FLAG_PRGMFLR | FLASH_FLAG_PRCDN);
FLASH_ErasePage(StartAddress);          // StartAddress = 0x08020000 - 2KB
// atp_nv_write_word(idx,w):
FLASH_ProgramWord(StartAddress + idx*4, w);
// atp_nv_commit:
FLASH_Lock();
```

### 範例填法（無 flash 平台 → 檔案/EEPROM）
```c
static uint32_t nv[64];
void atp_nv_erase(void){ memset(nv,0xFF,sizeof(nv)); }
void atp_nv_write_word(uint32_t i,uint32_t w){ nv[i]=w; }
uint32_t atp_nv_read_word(uint32_t i){ return nv[i]; }
void atp_nv_commit(void){ /* fwrite nv -> file */ }
```

> ⚠️ **flash 抹除粒度**：原平台一次抹一頁（2KB）。若目標平台抹除粒度不同（如 sector），請確保 `atp_nv_erase` 抹掉的範圍涵蓋整個設定區，且不誤刪程式碼。

---

## 4. 系統控制介面

| 函式 | 原型 | 取代原本 |
|------|------|----------|
| 軟重置 | `void atp_system_reset(void);` | `atcmd_rst()` 內的 IAP `__asm` 跳轉 |

### 移植做法
`atcmd_rst()` 改成直接呼叫 `atp_system_reset()`，**刪掉 `WFI_SET/INTX_*/MSR_MSP/__asm` 整段**（Keil 專屬）。

### 範例填法
```c
// Cortex-M 通用：
void atp_system_reset(void){ NVIC_SystemReset(); }
// PC/Linux 模擬：
void atp_system_reset(void){ exit(0); }
```

---

## 5. 時間介面（選用）

| 函式 | 原型 | 用途 |
|------|------|------|
| 毫秒 tick | `uint32_t atp_mstick(void);` | keepalive/逾時自管（原模組少用，可回傳 0 stub） |

---

## 6. 介面總表（即 `at_port.h` 內容）

```c
/* UART */
void         atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop);
void         atp_uart_putc(char c);
char        *atp_uart_rx_buf(void);
unsigned int atp_uart_rx_len(void);
void         atp_uart_rx_clear(void);
int          atp_uart_rx_ready(void);
/* NV */
void         atp_nv_erase(void);
void         atp_nv_write_word(uint32_t idx, uint32_t word);
uint32_t     atp_nv_read_word(uint32_t idx);
void         atp_nv_commit(void);
/* System */
void         atp_system_reset(void);
/* Time (optional) */
uint32_t     atp_mstick(void);
```

> 下一份：[`03-uip-api-mapping.md`](03-uip-api-mapping.md) — 網路堆疊對接。
