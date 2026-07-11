# 00 — Project.uvprojx 檔案/路徑清單與分類

> 來源：[`SDK/MDK/Project.uvprojx`](../../SDK/MDK/Project.uvprojx)
> 目的：在動手移植前，先看清楚整個專案用到哪些檔案、各自的角色，並標明**移植時要不要搬**。

---

## 1. Build Target 說明（重要）

`Project.uvprojx` 內含 **6 個 build target**，但只有第一個是真的：

| # | TargetName | Device | 狀態 |
|---|------------|--------|------|
| 1 | **`STM32100E-EVAL`** | **`AT32F413C8T7`** | ✅ **唯一有效目標**（名稱沿用舊 STM32 範本，實際是 AT32） |
| 2 | `STM3210E-EVAL_XL` | `STM32F103ZG` | ⚠️ 舊 STM32 殘留設定 |
| 3~6 | `STM3210C/E/B…` | STM32F10x | ⚠️ 舊 STM32 殘留設定 |

> **移植時只看 target #1。** 其餘 5 個是從 ST 官方範例沿用下來、未清理的設定，與本韌體無關，可忽略。

### Target #1 關鍵設定（來源 `Project.uvprojx`）

| 項目 | 值 | 行號 |
|------|----|------|
| Device | `AT32F413C8T7`（Cortex-M4, FPU） | `:17`, `:20` |
| IROM (Flash) | `0x08000000`, 64 KB (`0x10000`) | `:254` |
| IRAM (RAM) | `0x20000000`, 32 KB (`0x8000`) | `:248` |
| 編譯巨集 (`Define`) | `AT32F413Cx_MD, USE_STDPERIPH_DRIVER, AT_START_F413_V1_0` | `:340` |
| C 標準 | C99 (`uC99=1`) | `:328` |
| 輸出 | HEX，燒錄至 `0x08000000` | `:54`, `:370` |

### IncludePath（target #1，來源 `Project.uvprojx:342`）

```
..\DM9051\include
..\Libraries\AT32F4xx_StdPeriph_Driver\inc
..\User
..\DM9051\App\webserver
..\DM9051\App\etherbridge
..\DM9051\App\AT_Command
..\bsp ; ..\bsp\inc
..\libraries_nmdk\cmsis\cm4\core_support
..\Libraries\CMSIS\CM4\DeviceSupport
```

---

## 2. 來源檔分類總覽

圖例：🟢 **移植主體**（要搬）｜🔵 **uIP 網路堆疊**（**整包複製，核心邏輯不改**，v1.1 起不再改用目標網路堆疊）｜🟡 **平台 HAL**（用 `at_port` 取代）｜⚪ **第三方/晶片驅動/與 AT 無關**

### Group: User（`..\User\`）

| 檔案 | 角色 | 類別 |
|------|------|------|
| `main.c` | 進入點；主迴圈呼叫 `bridge_init()` / `tcpip_process()` / **`at_cmdProcess()`**（[main.c:246-258](../../SDK/User/main.c)）；開機 `Read_AT_DataFlash()`（[main.c:220](../../SDK/User/main.c)） | 🟡 參考 |
| `at32_board_uart.c` | **UART HAL**：`UART_CMD_Init_Update()`、ISR、DMA、RX FIFO；`AT32_CMD_USART` = USART2 | 🟡 → `at_port` UART |
| `app_call.c` | uIP application callback 分派（`UIP_APPCALL`） | 🔵 |
| `spi_config.c` | DM9051 SPI 設定 | ⚪ |
| `system_at32f4xx.c` / `at32f4xx_it.c` / `at32f4xx_assert.c` | 系統時脈、中斷向量、assert | ⚪ |
| `windows_app.c` | 應用初始化雜項 | ⚪ |

### Group: DM9051（`..\DM9051\`）— 🔵 uIP 堆疊，**整包複製、核心邏輯不改**

| 檔案 | 角色 | 移植做法 |
|------|------|----------|
| `uip.c`, `uip_arp.c`, `uip_init.c`, `uip_keepalive.c`, `uip_timer.c`, `uip-fw.c`, `uiplib.c`, `uip-neighbor.c`, `uip-split.c` | uIP 核心狀態機 | 🔵 **直接複製，不改** |
| `dhcpc.c`, `resolv.c` | DHCP client / DNS | 🔵 **直接複製，不改** |
| `memb.c`, `trans_level.c` | 記憶體池、傳輸層輔助 | 🔵 **直接複製，不改** |
| `clock-arch.c` | uIP tick 來源（讀 `g_RunTime`） | 🟡 **需改寫**，接目標平台計時器 |
| `tapdev.c` | uIP↔NIC 收發膠合（呼叫 `DM9051_Init/RX/TX`） | 🟡 **需改寫**，接目標平台乙太網晶片驅動 |
| `DM9051.c` | DM9051 SPI 以太網 MAC/PHY 驅動 | ⚪ 沿用晶片則複製＋SPI 呼叫改走 `at_port`；換晶片則改用新驅動 |

> 這些檔案的 API 名稱對照表（僅供「無法共存 uIP」時的備用方案參考）見 [`03-uip-api-mapping.md`](03-uip-api-mapping.md)。

### Group: AT_Command（`..\DM9051\App\AT_Command\`）— 🟢 **移植主體**

| 檔案 | 角色 |
|------|------|
| `atcommand.c` | AT 指令主解析 `at_cmdProcess()`（[:711](../../SDK/DM9051/App/AT_Command/atcommand.c)）、全域狀態定義（`at_type`, `eeprom_type`, `g_u8RecData`…） |
| `at_base_cmd.c` | 基礎指令（help/version/restore/rst/baud/translen…）、IAP reset（[:557](../../SDK/DM9051/App/AT_Command/at_base_cmd.c)） |
| `at_tcpip_cmd.c` | TCP/IP 指令（ip/mask/gw/tcplport/tcpconn/udpconn/dns/keepalive）、`u_writeuart1()`（[:26](../../SDK/DM9051/App/AT_Command/at_tcpip_cmd.c)） |
| `dataflash.c` | 設定的 Flash word 序列化讀寫（`Write_AT_Show_DataFlash` / `Read_AT_DataFlash`） |
| `atcommand.h` | `struct at_funcation` / `struct eeprom_funcation`、所有原型、預設值巨集 |
| `dataflash.h` | `TEST_PATTERN` |

> 注意：`atcommand.h` `#include "uip.h"`（取 `uip_ipaddr_t`）。見 [`03`](03-uip-api-mapping.md) 的型別對應。

### Group: etherbridge（`..\DM9051\App\etherbridge\`）— 🟢 **與 AT_Command 一起整包複製**

| 檔案 | 角色 | 類別 |
|------|------|------|
| `etherbridge.c` | UART↔Ethernet 透傳橋接：`bridge_init()`、`tcp_bridge_appcall()`/`udp_bridge_appcall()`（被 `app_call.c` 的 `UIP_APPCALL` 呼叫）、`senddata()` | 🟢 移植主體（直接用 `uip_buf`，因 uIP 整包保留而**不必改**；UART DMA 暫存器存取需改走 `at_port`） |
| `etherbridge.h` | 型別/原型宣告 | 🟢 直接複製 |

> v1.1 起：`SDK/DM9051/App/` 整個目錄（`AT_Command/`、`etherbridge/`、`webserver/`）視為同一個移植單元，一起複製到新專案，不再區分「視需求移植」。

### Group: webserver（`..\DM9051\App\webserver\`）— 🟢 **整包複製**（CGI 已客製化，因 uIP 保留而 HTTP 引擎也不用換）

| 檔案 | 角色 | 類別 |
|------|------|------|
| `httpd-cgi.c` | **13 個客製化 CGI handler**（AT 設定頁：IP/role/DHCP/DNS/port/keepalive/baud/MAC 顯示與設定） | 🟢 直接複製（對 `at_show`/`at_type` 的存取邏輯不變） |
| `httpd.c` | HTTP daemon（`psock`/`protothread`，含 `inputbuf` 擴大至 300B，joseph 20240604） | 🟢 直接複製（因 uIP 的 `psock`/`protothread` 一併保留，**不必換成其他 HTTP server**） |
| `httpd-fs.c` / `httpd-fsdata.c` | ROM 靜態 FS | 🟢 可原樣搬 |
| `http-strings.c` / `http-strings.h` | HTTP 字串常數 | 🟢 直接複製 |
| `webopts.h` | 編譯選項（`WEB_BAUD_UPDATE`） | 🟢 直接複製 |
| `httpd.h` / `httpd-cgi.h` | 結構/CGI 框架標頭 | 🟢 直接複製 |
| `httpd-fs/style*.css` | 靜態 CSS | 🟢 直接複製 |

> `TCP_App/webserver/` 與 `App/webserver/` 內容相同，以 `App/webserver/` 為移植基準。
> 詳細分析見 [`05-webserver-porting.md`](05-webserver-porting.md)。

### Group: StdPeriph_Driver_AT32（`..\Libraries\AT32F4xx_StdPeriph_Driver\src\`）— 🟡/⚪

`misc.c`, `at32f4xx_{comp,crc,dma,flash,gpio,gpio_ex,i2c,pwr,rcc,rtc,spi,usart,adc}.c`
> AT32 廠商 HAL。目標平台改用自己的 HAL；其中 **`at32f4xx_flash.c`**（Flash）與 **`at32f4xx_usart.c`**（UART）是 `dataflash.c` / UART 層的底層，移植時由 `at_port` 取代。
> ⚠️ 注意此 Group 內混放了一個 `atcommand.c`（[`..\DM9051\App\AT_Command\atcommand.c`](../../SDK/DM9051/App/AT_Command/atcommand.c)，`Project.uvprojx:622`）—它其實是 AT_Command 主體，只是被列在這個 Group 裡。

### Group: CMSIS_AT32 / BSP / Doc

| 檔案 | 角色 | 類別 |
|------|------|------|
| `startup_at32f413cx_md.s` | 啟動碼/向量表 | ⚪ 平台專屬 |
| `bsp_uart_fifo.c` | UART FIFO 緩衝 | 🟡 UART 相關 |
| `readme.txt` / `info.txt` / `UserTips.txt` / `rom size*.txt` | 文件 | ⚪ |

---

## 3. 移植時的檔案處理對照

> 目標路徑採用 [README 第 4 節](README.md#4-建議的新專案目錄結構v11-新增) 的 `app/` / `net/uip/` / `port/` 三分目錄結構，取代原本扁平的 `SDK/DM9051/` 配置，方便日後維護時一眼分辨「複製就好」與「要改」的範圍。

| 原檔 | 新專案目標路徑 | 移植動作 |
|------|----------------|----------|
| `atcommand.c`, `at_base_cmd.c`, `at_tcpip_cmd.c`, `dataflash.c`, `atcommand.h`, `dataflash.h` | `app/at_command/` | **複製**，把平台呼叫改走 `at_port`；`atcommand.h` 的 `#include "uip.h"` **保留原樣**（uIP 已整包複製，不需要 `at_net_shim.h` 轉接） |
| `etherbridge.c` / `etherbridge.h` | `app/etherbridge/` | **複製**，`uip_buf` 相關邏輯不改；UART DMA 暫存器存取改走 `at_port` |
| `httpd-cgi.c` / `httpd.c` / `httpd-fs.c` / `httpd-fsdata.c` / `http-strings.c` / `webopts.h` / 對應 `.h` / `httpd-fs/style*.css` | `app/webserver/` | **複製**，因 uIP 的 `psock`/`protothread` 保留而**不必換 HTTP 引擎**；保留對 `at_show`/`at_type` 的存取邏輯 |
| `uip.c`, `uip_arp.c`, `uip_init.c`, `uip_timer.c`, `uip_keepalive.c`, `uip-split.c`, `uiplib.c`, `uip-fw.c`, `uip-neighbor.c` | `net/uip/core/` | **整包複製，核心邏輯不改** |
| `dhcpc.c`, `resolv.c`, `psock.c`, `memb.c`, `trans_level.c` | `net/uip/apps/` | **整包複製，核心邏輯不改** |
| `uip*.h`（`include/`） | `net/uip/include/` | **直接複製** |
| `at32_board_uart.c` 的 UART 部分 | `port/at_port.c` | **不搬原檔**，用目標 HAL 重寫 UART |
| `at32f4xx_flash.c` 的 Flash 部分 | `port/at_port.c` | **不搬原檔**，用目標 HAL/檔案系統重寫 NV |
| `clock-arch.c` | `port/clock-arch.c` | 複製後**需改寫**，接上目標平台計時器 |
| `tapdev.c` | `port/net_if/tapdev.c` | 複製後**需改寫**，接上目標平台乙太網晶片驅動 |
| `DM9051.c` | `port/net_if/DM9051.c` | 沿用 DM9051 晶片：複製＋SPI 呼叫改走 `at_port`；換晶片：改用新驅動 |
| `main.c` | （無固定目標，供參考） | **參考**其主迴圈順序，在新平台寫等價的整合（見 04 第 5 步） |

> 下一份：[`01-at-command-dependencies.md`](01-at-command-dependencies.md) — 逐一列出 4 個移植主體檔案的平台相依點與行號。
