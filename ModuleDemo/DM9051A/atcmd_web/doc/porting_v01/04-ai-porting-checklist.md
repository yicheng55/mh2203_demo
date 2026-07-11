# 04 — AI 分步驟自動移植檢查清單

> 這份是**動手執行**用的有序清單。每一步含「動作 / 驗收 / 風險」。AI 可逐步勾選、每步驗收通過再進下一步。
> 前提：已讀過 [`00`](00-project-file-inventory.md)~[`03`](03-uip-api-mapping.md)。前提工具：目標平台的編譯器、目標網路堆疊（lwIP/socket/…）。

---

## Step 0 — 準備

- [ ] 確認目標平台：MCU/OS、編譯器、是否有 flash/EEPROM/檔案系統、tick 來源。
- [ ] **確認網路堆疊策略**：預設**保留 uIP**（整包複製，見 [`03`](03-uip-api-mapping.md)）；只有目標平台無法共存 uIP 時，才改走備用方案（改接 lwIP/socket）。
- [ ] 在新專案建立目錄結構（見 [README 第 4 節](README.md#4-建議的新專案目錄結構v11-新增)）：`app/{at_command,etherbridge,webserver}/`、`net/uip/{core,apps,include}/`、`port/net_if/`。

**驗收**：目錄就緒，`net/uip/` 可獨立編過（先用 stub tick + stub 收發）。
**風險**：若誤選備用方案（棄用 uIP、改接 lwIP raw callback），shim 工作量與回歸風險最大；預設情況下不需要走這條路。

---

## Step 1 — 複製 App 應用層 + uIP 堆疊 + 範本（依新目錄結構分類）

依 [README 第 4 節](README.md#4-建議的新專案目錄結構v11-新增) 的 `app/` / `net/uip/` / `port/` 三分結構複製，**不要照抄原本 `SDK/DM9051/` 的扁平配置**：

- [ ] `app/at_command/` ← `atcommand.c/h`, `at_base_cmd.c`, `at_tcpip_cmd.c`, `dataflash.c/h`（原 `SDK/DM9051/App/AT_Command/`），**邏輯不改**
- [ ] `app/etherbridge/` ← `etherbridge.c/h`（原 `SDK/DM9051/App/etherbridge/`），**邏輯不改**
- [ ] `app/webserver/` ← `httpd.c/h`, `httpd-cgi.c/h`, `httpd-fs.c/h`, `httpd-fsdata.c/h`, `http-strings.c/h`, `webopts.h`, `httpd-fs/style*.css`（原 `SDK/DM9051/App/webserver/`），**邏輯不改**
- [ ] `net/uip/core/` ← `uip.c`, `uip_arp.c`, `uip_init.c`, `uip_timer.c`, `uip_keepalive.c`, `uip-split.c`, `uiplib.c`, `uip-fw.c`, `uip-neighbor.c`，**邏輯不改**
- [ ] `net/uip/apps/` ← `dhcpc.c`, `resolv.c`, `psock.c`, `memb.c`, `trans_level.c`，**邏輯不改**
- [ ] `net/uip/include/` ← 對應 `include/*.h`（uip 標頭）
- [ ] `port/clock-arch.c` ← 複製 `clock-arch.c`（之後要改寫，見 Step 3）
- [ ] `port/net_if/tapdev.c` ← 複製 `tapdev.c`（之後要改寫，見 Step 3）
- [ ] `port/net_if/DM9051.c` ← 複製 `DM9051.c`（沿用晶片則之後只動 SPI 呼叫，見 Step 3）
- [ ] `port/at_port.h`, `port/at_port.c` ← 複製範本 `templates/at_port.h`, `templates/at_port_template.c`(改名 `at_port.c`)
- [ ] 在 `app/at_command/` 4 個 `.c` 與 `app/etherbridge/etherbridge.c` 頂端，把 `#include "at32f4xx.h"` / `#include "at32_board_uart.h"` 換成 `#include "at_port.h"`；`atcommand.h` 的 `#include "uip.h"` **保留原樣**（因為 uIP 也一起複製過去了，不需要 shim）
- [ ] 依新路徑更新專案的 include path：`app/at_command`、`app/webserver`、`net/uip/include`、`port`、`port/net_if`

**驗收**：檔案到位，目錄結構符合 `app/` / `net/uip/` / `port/` 三分法，日後可一眼看出「複製即用」與「需改寫」的範圍。
**風險**：別動原專案的檔案——這裡是**複製**。若目標平台無法共存 uIP，才改用 `templates/at_net_shim.h`（見 [`03`](03-uip-api-mapping.md) 備用方案）。

---

## Step 2 — 實作 `at_port.c`（平台 HAL），先讓它編過

依 [`02`](02-at-port-hal-spec.md) 實作：

- [ ] **UART**：`atp_uart_init` / `atp_uart_putc` / `atp_uart_rx_*`。把 `u_writeuart1` 改為呼叫 `atp_uart_putc`（或在 shim/port 內提供 `u_writeuart1` 包裝）。
- [ ] **NV**：`atp_nv_erase / write_word / read_word / commit`。沿用 `dataflash.c` 的 word 版面。
- [ ] **Reset**：`atp_system_reset`（Cortex-M 用 `NVIC_SystemReset`）。把 `atcmd_rst()` 的 `__asm`/IAP 段換成呼叫它。
- [ ] **Time**：`atp_mstick`（可先 `return 0`）。

**驗收**：`at_port.c` + 4 個 AT 檔案能編譯成功（網路 shim 先用 stub 也行）。
**風險**：
- `__asm WFI_SET/INTX_*/MSR_MSP` 是 Keil 專屬，**整段刪除**，別嘗試移植。
- flash 抹除粒度不同 → 確保 NV 區不覆蓋程式碼。

---

## Step 3 — 改寫 uIP 硬體膠合層（`clock-arch.c` / `tapdev.c`），讓 uIP 編過

依 [`03`](03-uip-api-mapping.md) 「0. 主策略」：uIP 核心邏輯不改，只重寫兩個硬體介面檔。

- [ ] **`port/clock-arch.c`**：把 `clock_time()` 改讀目標平台的 tick 來源（systick 計數器 / RTOS tick），維持回傳型別 `clock_time_t` 語意不變。
- [ ] **`port/net_if/tapdev.c`**：`tapdev_init()` / `tapdev_read()` / `tapdev_send()` 改呼叫目標平台的乙太網晶片驅動：
  - 沿用 DM9051 晶片 → `port/net_if/DM9051.c` 把其中的 SPI/GPIO 呼叫改走 `at_port`（新增 SPI 收發 API 到 `at_port.h`）。
  - 換乙太網晶片 → `tapdev_init/read/send` 改呼叫新驅動對應的 init/收包/發包函式，`port/net_if/DM9051.c` 整份換成新驅動檔。
- [ ] **確認 `uip-conf.h` 型別與位元組序**：`u8_t/u16_t/u32_t` 大小、`BYTE_ORDER` 與目標 MCU 一致。
- [ ] （僅無法共存 uIP 時才需要）改走備用方案：實作 `at_net_shim.h` 的 `atshim_*`，把 `uip_*` 巨集對接到 lwIP/socket。

**驗收**：整個 `app/` + `net/uip/` + `port/` 編譯連結成功；`tapdev_read()` 能收到封包、`uip_input()` 正常解出 ARP/IP。
**風險**：`uip_appdata` 是 uIP 的單一全域收發 buffer，沿用 uIP 時此假設不變，不需額外處理；只有走備用方案（換掉 uIP）時才要注意 shim TX buffer 的生命週期。

---

## Step 4 — 設定載入/儲存接上

- [ ] 開機呼叫 `Read_AT_DataFlash()`；若回傳 2（空）→ `atcmd_restore()` 寫預設。
- [ ] 確認 `at_type` 被正確填充（IP/role/port/baud…）。
- [ ] `atp_uart_init(at_type.baudrate, at_type.wordlen, at_type.parity, at_type.stop)` 套用設定。

**驗收**：開機讀回設定，存檔後重開值還在。
**風險**：word 版面讀寫不對稱 → 抓 `atp_nv_read_word/write_word` 的 idx 對齊。

---

## Step 5 — 串接主迴圈

參考 [main.c:246-258](../../SDK/User/main.c) 的順序，在新平台主迴圈/任務中：

- [ ] （若移植 etherbridge）`bridge_init()` 等價
- [ ] uIP 輪詢/事件處理（`tapdev_read()` → `uip_input()`；週期呼叫 `uip_periodic()`；`UIP_APPCALL` 分派沿用 `app_call.c` 原邏輯）
- [ ] **`at_cmdProcess()`** ← 必接
- [ ] 確保 UART RX（ISR 或輪詢）填 `g_u8RecData`/`gt_u32comRbytes`，收到 `\n` 設 `atcmd_flag`。

**驗收**：UART 打 `version\r\n` 會回 `IPSPP...` 版本字串。
**風險**：命令解析需要一行收齊（`atcmd_flag`）；RX 行結束判定要與原 ISR 一致。

---

## Step 6 — 煙霧測試（逐項）

| # | 測試 | 期望 |
|---|------|------|
| 1 | `version` | 回版本字串 |
| 2 | `restore` 後重開 | 回預設 IP `192.168.1.100` |
| 3 | 設 IP/mask/gw（`ip`/`mask`/`gw` 指令）+ `savep` + 重開 | 設定保留 |
| 4 | TCP server（role + tcplport）→ 外部 client 連入 | 可連、可雙向收發 |
| 5 | TCP client（tcpconn 到遠端）| 可連、可收發 |
| 6 | UDP server/client（udplport/udpconn）| 可收發 |
| 7 | `RST` 指令 | MCU 重置 |

**驗收**：7 項全綠。
**風險**：透傳模式（`on_trans_mode`）下 UART 輸入被當資料而非命令；測試命令時確認不在透傳態，或用 `+++` 退出。

---

## 移植完成定義（DoD）

- [ ] `app/at_command/` 4 檔在目標平台**未改解析邏輯**即可運作（只動 include 與平台呼叫對接）。
- [ ] `app/etherbridge/`、`app/webserver/` 邏輯未改，僅 `etherbridge.c` 的 UART DMA 存取改走 `at_port`。
- [ ] `net/uip/core/`、`net/uip/apps/` 內 uIP 核心檔（`uip.c`/`uip_arp.c`/`dhcpc.c`/`resolv.c`/`psock.c`/`memb.c`…）**未改邏輯**，與原專案內容一致。
- [ ] `port/at_port.c`、`port/clock-arch.c`、`port/net_if/tapdev.c`（含 NIC 驅動）為僅有的平台相依檔。
- [ ] Step 6 七項煙霧測試通過。
- [ ] 原 AT32F413 專案（`SDK/MDK/Project.uvprojx`）**完全未被修改**。
