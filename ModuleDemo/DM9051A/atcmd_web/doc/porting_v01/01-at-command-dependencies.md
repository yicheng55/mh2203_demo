# 01 — AT_Command 平台相依性分析

> 對象：移植主體 4 檔 — `atcommand.c`、`at_base_cmd.c`、`at_tcpip_cmd.c`、`dataflash.c`（+ `atcommand.h` / `dataflash.h`）
> 目的：標出**每一個平台相依點**（含檔名:行號），讓你知道在新平台上要替換/重接什麼。所有相依點最後都收斂到兩個抽象層：**`at_port`（平台 HAL）** 與 **`at_net_shim`（網路）**。

行號以本專案目前版本為準；移植時請在原檔抽查比對。

---

## 1. #include 分類

| 檔案 | 平台專屬（要處理） | uIP（→ shim） | 專案內 | 標準 C |
|------|--------------------|---------------|--------|--------|
| `atcommand.c` | `at32f4xx.h` (:8) | `uip.h` (:3), `uip-split.h` (:4) | `includes.h`, `atcommand.h`, `etherbridge.h`, `DM9051.h` | `stdio.h` |
| `at_base_cmd.c` | `at32f4xx.h` (:4) | （經由 `atcommand.h`） | `includes.h`, `atcommand.h`, `DM9051.h` | `stdio.h` |
| `at_tcpip_cmd.c` | `at32f4xx.h` (:12), `at32_board_uart.h` (:13) | `uip.h` (:4), `uip_arp.h` (:5), `uiplib.h` (:6), `tapdev.h` (:7), `uip_init.h` (:8) | `includes.h`, `app_call.h`, `atcommand.h`, `etherbridge.h`, `DM9051.h` | — |
| `dataflash.c` | `at32f4xx.h` (:4) | （經由 `atcommand.h`） | `includes.h`, `atcommand.h`, `dataflash.h`, `trans_level2.h` | `stdio.h` |

**移植動作**：把 `at32f4xx.h` / `at32_board_uart.h` 換成 `#include "at_port.h"`；uIP 標頭由 `at_net_shim.h` 統一提供（內部再決定 include 目標堆疊或保留 uip.h）。

---

## 2. UART / 序列埠層

| 相依點 | 位置 | 說明 |
|--------|------|------|
| `u_writeuart1(char)` | [at_tcpip_cmd.c:26-31](../../SDK/DM9051/App/AT_Command/at_tcpip_cmd.c) | **唯一的字元輸出原語**，逐字元 busy-wait 送出 |
| `USART_SendData(AT32_CMD_USART, …)` | at_tcpip_cmd.c:28 | 底層 AT32 HAL 送一個 byte |
| `USART_GetFlagStatus(AT32_CMD_USART, USART_FLAG_TRAC)` | at_tcpip_cmd.c:30 | 等待傳輸完成旗標 |
| `AT32_CMD_USART` 巨集 | `at32_board_uart.h`（= USART2） | 命令/資料介面埠 |
| `atcmd_baseline()` / `atcmd_resp_cmd()` | at_tcpip_cmd.c:33,43 | 上層都呼叫 `u_writeuart1` |
| 設定埠初始化 `UART_CMD_Init_Update(baud,wordlen,parity,stop)` | `at32_board_uart.c`（由 main 與 baud 指令呼叫） | 改變 UART 參數 |
| RX 緩衝 `g_u8RecData[RecData_Size]` | [atcommand.c:20](../../SDK/DM9051/App/AT_Command/atcommand.c) | 一行命令的接收緩衝 |
| RX 計數 `gt_u32comRbytes` | atcommand.c:24 | 已接收位元組數 |
| RX 完成旗標 `atcmd_flag` | atcommand.c:19 | ISR 收到 `\n` 後置位，`at_cmdProcess` 消費 |

> **抽象化**：`u_writeuart1` → `atp_uart_putc`；`UART_CMD_Init_Update` → `atp_uart_init`。RX 端維持 `g_u8RecData`/`gt_u32comRbytes`/`atcmd_flag` 三件套語意，由目標平台的 UART ISR/輪詢填入（見 02 與 04 第 2、5 步）。

---

## 3. Flash / 非揮發儲存層（`dataflash.c`）

設定以 **word（4 byte）序列化**寫進一個 flash page，開機用 `TEST_PATTERN` 判斷是否已初始化。

| 相依點 | 位置 | 說明 |
|--------|------|------|
| `FLASH_Unlock()` | dataflash.c:41 等 | 解鎖 |
| `FLASH_ClearFlag(FLASH_FLAG_PRGMFLR \| FLASH_FLAG_PRCDN)` | dataflash.c:44 | 清旗標 |
| `FLASH_ErasePage(StartAddress)` | dataflash.c:47 | 抹除頁 |
| `FLASH_ProgramWord(addr, data)` | dataflash.c:25（`Write_AT_Data`） | 寫一個 word，回傳 `addr+4` |
| `FLASH_Lock()` | dataflash.c:169 等 | 上鎖 |
| 狀態型別 `FLASH_Status` / `FLASH_PRC_DONE` | dataflash.c:18,50 | 操作結果 |
| `Read_AT_FlashDWord(idx)` | dataflash.c（讀） | 以 word index 讀回 |
| 位址 `StartAddress = 0x08020000-(1024*2)` | [trans_level2.h:16](../../SDK/DM9051/include/trans_level2.h) | 主設定區（page 末 2KB） |
| 位址 `StartAddress2 = 0x08020000-(1024*4)` | trans_level2.h:19 | 備援區 |
| `TEST_PATTERN = 0x55AA55AA` | [dataflash.h:7](../../SDK/DM9051/App/AT_Command/dataflash.h) | 「已寫入」標記（word -12） |

**word 版面（節錄，`Write_AT_Show_DataFlash`，[dataflash.c:52~](../../SDK/DM9051/App/AT_Command/dataflash.c)）**：
```
word -12: TEST_PATTERN
word -11: role | (dhcpc_mode?0x8:0)
word -10: hostip   (hostip[1]<<16 | hostip[0])
word  -9: hostmask
word  -8: hostgw
word  -7: tcp_lport
...        (其餘 IP/port/baud/dns/keepalive 欄位依序)
```

> **抽象化**：把 unlock/erase/clearflag/lock 收進 `atp_nv_erase()` + `atp_nv_commit()`；`FLASH_ProgramWord`→`atp_nv_write_word(idx,word)`；`Read_AT_FlashDWord`→`atp_nv_read_word(idx)`。**序列化版面（哪個 word 放什麼）原樣保留**，只換底層讀寫。目標平台若無 flash，可用 EEPROM/檔案/KV store 實作同一介面。

---

## 4. Reset / 系統控制（`at_base_cmd.c`）

| 相依點 | 位置 | 說明 |
|--------|------|------|
| `atcmd_rst()` | [at_base_cmd.c:557-571](../../SDK/DM9051/App/AT_Command/at_base_cmd.c) | `RST` 指令：驗證向量後跳回 app（IAP 風格軟重置） |
| `__asm WFI_SET / INTX_DISABLE / INTX_ENABLE / MSR_MSP` | at_base_cmd.c:532-551 | Cortex-M 內嵌組語（**Keil ARMCC 語法專屬**） |
| `jump_defapp`（`iapfun` 函式指標） | at_base_cmd.c:11,563-566 | 跳轉位址 |
| `FMC_IAP_PROGARM_ADDR = 0x08000000` | at_base_cmd.c:8 | 向量表基底 |

> **抽象化**：`atcmd_rst()` 內部改呼叫 `atp_system_reset()`。預設實作 = `NVIC_SystemReset()`（任何 Cortex-M 都有；其他架構用對應重置）。原本的 `MSR_MSP`/`__asm` IAP 跳轉**整段可移除**，因為 `__asm` 是 Keil 專屬、且多數平台不需要這套自跳轉。IAP 需求若存在，列為平台可選實作。

---

## 5. 全域狀態（跨檔共用，務必一起搬）

| 全域 | 定義處 | 用途 |
|------|--------|------|
| `at_type` (`struct at_funcation`) | [atcommand.c:14](../../SDK/DM9051/App/AT_Command/atcommand.c) | **中央執行期設定**（IP/role/port/baud/dns/keepalive…） |
| `eeprom_type` (`struct eeprom_funcation`) | atcommand.c:15 | DM9051 EEPROM 設定（MAC/VID/PID…） |
| `at_show` / `eeprom_show` | at_base_cmd.c | flash 讀寫的暫存鏡像（`Copy_AT_Type_to_Show`） |
| `g_u8RecData[]` / `gt_u32comRbytes` | atcommand.c:20,24 | UART RX 緩衝 + 計數 |
| `atcmd_flag` | atcommand.c:19 | 一行命令收齊旗標 |
| `tcp_connected` / `udp_connected` | atcommand.c:21,22 | 連線狀態 |
| `manualset_dns_srvip` / `gt_comeDataUsart2` | atcommand.c:26,25 | DNS/資料旗標 |
| `at_tmpstr[AT_TMPSTR_LEN]` | atcommand.c:18 | sprintf 暫存 |

> 這些都是**純資料/邏輯**，不含平台呼叫，原樣保留。`struct at_funcation` 內含 `uip_ipaddr_t` 欄位（IP/mask/gw/raddr…），型別由 `at_net_shim.h` 提供（見 03）。

---

## 6. 時間 / 計時器

- AT_Command 這 4 個檔案 **幾乎不直接用 tick/delay**；`struct timer`（uIP）出現在結構宣告但本層未驅動。
- keepalive / 連線逾時的計時由 uIP 層（`uip_timer.c` / `uip_keepalive.c`）處理。
- 抽象層仍提供 `atp_mstick()`，給移植時若需自管逾時使用。

---

## 7. 進入點（整合時要接的）

| 函式 | 位置 | 在主迴圈的角色 |
|------|------|----------------|
| `at_cmdProcess()` | [atcommand.c:711](../../SDK/DM9051/App/AT_Command/atcommand.c) | 每圈呼叫；有 `atcmd_flag` 就解析 `g_u8RecData` 一行命令 |
| `Read_AT_DataFlash()` | dataflash.c | 開機載入設定到 `at_type`（回傳 2 = 空 → `atcmd_restore()`） |
| `atcmd_restore()` | atcommand.c:43 | 寫入預設值 |
| `at_defaultset()` | atcommand.c | 設定 `at_type` 預設值 |
| `on_trans_mode()` / `on_trans_out_mode()` | atcommand.c | 判斷是否在透傳模式（決定 UART 輸入當資料或命令） |

> 原主迴圈順序見 [main.c:246-258](../../SDK/User/main.c)：`bridge_init()` → `tcpip_process()` → `at_cmdProcess()`。新平台要保留「**先收網路、再處理 UART 命令**」的順序語意。

---

## 8. 相依點 → 抽象層 收斂表

| 相依類別 | 原 API | 收斂到 |
|----------|--------|--------|
| UART 送字元 | `u_writeuart1` / `USART_SendData` | `at_port.h` → `atp_uart_putc` |
| UART 初始化 | `UART_CMD_Init_Update` | `at_port.h` → `atp_uart_init` |
| Flash 讀寫 | `FLASH_*` / `Read_AT_FlashDWord` | `at_port.h` → `atp_nv_*` |
| 系統重置 | `atcmd_rst` 的 `__asm`/IAP | `at_port.h` → `atp_system_reset` |
| 網路設定/連線 | `uip_*` / `dhcpc_*` / `resolv_*` | `at_net_shim.h`（見 03） |
| IP 型別/巨集 | `uip_ipaddr_t` / `uip_ipaddr*` | `at_net_shim.h` |

> 下一份：[`02-at-port-hal-spec.md`](02-at-port-hal-spec.md) — `at_port.h` 介面規格。
