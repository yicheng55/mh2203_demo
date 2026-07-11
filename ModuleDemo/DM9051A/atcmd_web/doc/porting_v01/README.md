# AT_Command 跨平台移植技術手冊

> 版本：v1.0 ／ 對應韌體：`AT32F413 IPSPP1001_V1.0.5R9`
> 目標：把本專案的 **AT_Command 命令解析模組**（UART↔Ethernet gateway 的 AT 指令層）移植到**其他 MCU 平台與其他網路堆疊**。

---

## 0. 這份手冊在做什麼

原韌體把 **AT 指令解析** 緊密綁在：

- **MCU 平台**：Artery AT32F413C8T7（Cortex-M4）+ Keil MDK + AT32 StdPeriph HAL
- **網路堆疊**：uIP（Adam Dunkels 的輕量 TCP/IP）
- **以太網晶片**：DM9051（SPI）

本手冊的策略是 **把「MCU 平台相依」隔離到一層薄抽象，並把 `SDK/DM9051/App/` 應用層與 uIP 網路堆疊整包複製過去**，讓 AT_Command / etherbridge / webserver 的邏輯、以及它所依附的 uIP 事件模型都幾乎原封不動地搬到新平台：

| 抽象層 / 元件 | 範本檔或來源 | 處理方式 |
|--------|--------|----------|
| **平台 HAL** | `templates/at_port.h` | UART 收發、Flash/NV 儲存、系統重置、計時 → 用 `at_port` 抽象，各平台各自實作 |
| **App 應用層** | `SDK/DM9051/App/`（`AT_Command/`、`etherbridge/`、`webserver/`） | **整個目錄整包複製到新專案，核心邏輯不改**，只把平台呼叫（UART/Flash/reset）改走 `at_port` |
| **uIP 網路堆疊** | `SDK/DM9051/uip*.c`、`dhcpc.c`、`resolv.c`、`psock.c`、`memb.c`、`uip_arp.c`… | **整包複製到新專案，核心邏輯不改** |
| **NIC 驅動介面** | `clock-arch.c` / `tapdev.c` / `DM9051.c` | uIP 與硬體之間的膠合層，**必須改寫**接上目標平台的計時器與乙太網晶片 |

> **App 目錄整包複製**：`SDK/DM9051/App/` 底下 `AT_Command/`、`etherbridge/`、`webserver/` 三個子目錄都直接複製到新專案（例如 `app/AT_Command/`、`app/etherbridge/`、`app/webserver/`），**不是只搬 AT_Command**。理由與 uIP 相同：這三個模組都直接操作 `uip_buf`/`uip_appdata`/`psock`/`protothread`，既然 uIP 已整包保留，這些應用層程式碼也不需要為了換網路堆疊而改寫，只需把 UART/Flash/Reset 呼叫接到 `at_port`。

> **策略更新（v1.1）：uIP 本身也要移植（整包複製），不再嘗試改接 lwIP / socket 等目標網路堆疊。**
> 原因：uIP 是單緩衝、事件回呼模型（`uip_appdata`、`UIP_APPCALL`），與 lwIP/socket 的事件模型差異極大；若要把 AT_Command、`etherbridge`、`httpd` 都改接到別的堆疊，shim 工作量與回歸風險遠高於直接把 uIP 原始碼一起搬過去。**保留 uIP 可以讓 AT_Command / webserver / etherbridge 三個模組完全不用改網路呼叫**，唯一要動的是 uIP 與硬體銜接的兩個檔案：`clock-arch.c`（tick 來源）與 `tapdev.c`（收發封包，目前呼叫 `DM9051_Init/RX/TX`）。
> 若目標平台**已強制要求**使用 lwIP/socket（例如 RTOS 內建網路棧、無法並存 uIP），[`03-uip-api-mapping.md`](03-uip-api-mapping.md) 仍保留 uIP→lwIP/socket 的 API 對照表作為**備用方案**。

---

## 1. 重要原則

1. **不修改原專案。** 本手冊與所有範本檔都位於 `doc/porting_v01/`，**不會被 `Project.uvprojx` 引用、不加入任何 build target**，因此對現有 AT32F413 韌體的建置與功能 **零影響**。
2. **複製、不剪下。** 移植時把 **`SDK/DM9051/App/` 整個目錄**（AT_Command 4 個 `.c` + 2 個 `.h`、etherbridge、webserver）以及 **uIP 網路堆疊全部原始檔**，都**複製**到新平台專案，在新專案裡加抽象層；原專案保持可正常編譯。
3. **uIP 核心邏輯不改，只換硬體膠合層。** `uip.c` / `uip_arp.c` / `dhcpc.c` / `resolv.c` / `psock.c` / `memb.c` … 原樣複製；只重寫 `clock-arch.c`（tick 來源）與 `tapdev.c`（NIC 收發，銜接目標平台的乙太網晶片驅動）。
4. **先讓它編過，再讓它連線。** 先用 stub/PC 假平台把 `at_port` 與 NIC 膠合層補滿、編譯通過，再逐項對接真實硬體。

---

## 2. AI / 工程師閱讀順序

請**依序**閱讀，每份文件是下一份的前提：

| 順序 | 文件 | 內容 |
|------|------|------|
| 1 | [`00-project-file-inventory.md`](00-project-file-inventory.md) | `Project.uvprojx` 用到的所有檔案/路徑清單與分類（哪些要移植、哪些是 uIP、哪些是平台 HAL）。 |
| 2 | [`01-at-command-dependencies.md`](01-at-command-dependencies.md) | AT_Command 4 個檔案對平台的所有相依點（含檔名:行號）：UART / Flash / Reset / 全域變數 / 進入點。 |
| 3 | [`02-at-port-hal-spec.md`](02-at-port-hal-spec.md) | `at_port.h` 平台 HAL 介面規格 + 各平台實作指引 + AT32 對照寫法。 |
| 4 | [`03-uip-api-mapping.md`](03-uip-api-mapping.md) | **主策略**：uIP 整包複製到新專案，只需重寫 `clock-arch.c` / `tapdev.c` 兩個硬體膠合檔；**備用方案**：若目標平台不能共存 uIP，才用內附的 uIP→lwIP/socket API 對照表改走 `at_net_shim.h`。 |
| 5 | [`04-ai-porting-checklist.md`](04-ai-porting-checklist.md) | **分步驟移植檢查清單**（可勾選，每步含 動作／驗收／風險）。實際動手時照這份做。 |
| 6 | [`05-webserver-porting.md`](05-webserver-porting.md) | **Webserver 移植分析**：13 個客製化 CGI handler（AT 設定 Web UI）、三層移植策略（靜態資源/HTTP引擎/CGI業務邏輯）、Save 流程與 AT_Command 的連動。 |

範本檔：

- [`templates/at_port.h`](templates/at_port.h) — 平台抽象介面宣告
- [`templates/at_port_template.c`](templates/at_port_template.c) — 各平台需實作的空殼（含 TODO 與 AT32 對照註解）
- [`templates/at_net_shim.h`](templates/at_net_shim.h) — 備用方案用：uIP→目標堆疊對接巨集/轉換函式（`AT_NET_BACKEND_UIP` 模式下本檔幾乎透通，直接沿用真正的 uIP）

> 新專案的建議目錄結構（`app/` / `net/uip/` / `port/` 三分法）見本檔 [第 4 節](#4-建議的新專案目錄結構v11-新增)。

---

## 3. 移植主體一覽（要搬的東西）

> `SDK/DM9051/App/` 下 `AT_Command/`、`etherbridge/`、`webserver/` 三個子目錄**整包複製**，以下依模組分表列出角色與相依程度。

### AT_Command 核心（詳見 [`01`](01-at-command-dependencies.md)）

| 檔案 | 角色 | 平台相依程度 |
|------|------|--------------|
| `atcommand.c` | AT 指令主解析（`at_cmdProcess`）、全域狀態定義 | 中（uIP appdata、UART） |
| `at_base_cmd.c` | 基礎指令（version/restore/rst/baud…）、IAP reset | 中（reset、`__asm`） |
| `at_tcpip_cmd.c` | TCP/IP 指令（IP/mask/gw/tcpconn/udpconn…） | 高（uIP 連線 API、UART） |
| `dataflash.c` | 設定的 Flash word 序列化讀寫 | 高（Flash HAL、位址） |
| `atcommand.h` / `dataflash.h` | 結構、巨集、原型 | 低（含 `uip.h`） |

### etherbridge（UART↔Ethernet 透傳橋接）

| 檔案 | 角色 | 平台相依程度 |
|------|------|--------------|
| `etherbridge.c` | `bridge_init()`、`tcp_bridge_appcall()`/`udp_bridge_appcall()`（uIP `UIP_APPCALL` 分派進來）、`senddata()`、UART↔TCP/UDP 透傳資料搬移 | 高（直接操作 `uip_buf`/`UIP_PROTO_TCP` — 因 uIP 保留而不用改；UART DMA 暫存器 `ATCMD_USART_DMA_TX_Channel` — 需改走 `at_port`） |
| `etherbridge.h` | 型別/原型宣告（`#include "atcommand.h"`） | 低 |

### Webserver（詳見 [`05`](05-webserver-porting.md)）

| 檔案 | 角色 | 平台相依程度 |
|------|------|--------------|
| `httpd-cgi.c` | **13 個客製 CGI handler**，AT 設定 Web UI（核心業務邏輯） | 高（uIP psock/appdata + at_show/at_type） |
| `httpd.c` | HTTP daemon 引擎 | 中（因 uIP 一併複製，`psock`/`protothread` 原樣可用，不必換 HTTP server） |
| `httpd-fs.c` / `httpd-fsdata.c` | ROM 靜態 FS | 低（可原樣搬移） |
| `http-strings.c` / `webopts.h` | 字串常數、選項 | 無，直接複製 |

### uIP 網路堆疊（詳見 [`03`](03-uip-api-mapping.md)）— **整包複製，核心不改**

| 檔案 | 角色 | 移植做法 |
|------|------|----------|
| `uip.c` / `uip_arp.c` / `uip_init.c` / `uip_timer.c` / `uip_keepalive.c` / `uip-split.c` / `uiplib.c` / `uip-fw.c` / `uip-neighbor.c` | uIP 核心（IPv4/TCP/UDP/ARP 狀態機） | 直接複製，**不改邏輯** |
| `dhcpc.c` / `resolv.c` | DHCP client / DNS resolver | 直接複製 |
| `psock.c` / `memb.c` | Protothread socket、記憶體池 | 直接複製 |
| `clock-arch.c` | uIP tick 來源（`clock_time()` 讀 `g_RunTime`） | **需改寫**，接上目標平台的計時器/systick |
| `tapdev.c` | uIP 與 NIC 的收發膠合（呼叫 `DM9051_Init/RX/TX`） | **需改寫**，接上目標平台的乙太網晶片驅動 |
| `DM9051.c` | DM9051 SPI MAC/PHY 驅動 | 若沿用 DM9051 晶片：複製 + SPI 呼叫改走 `at_port`；若換晶片：改用新晶片驅動 |

---

## 4. 建議的新專案目錄結構（v1.1 新增）

原專案把 App 邏輯、uIP、平台 HAL 混雜在 `SDK/DM9051/` 與 `SDK/User/` 底下，不利於日後維護。移植到新專案時，建議依「**要不要改**」重新分類目錄，而不是照抄原本扁平的檔案配置：

```
new_project/
├── app/                          🟢 App 應用層（原 SDK/DM9051/App/）—— 核心邏輯不改，拿來就用
│   ├── at_command/               atcommand.c/h, at_base_cmd.c, at_tcpip_cmd.c, dataflash.c/h
│   ├── etherbridge/               etherbridge.c/h
│   └── webserver/                 httpd.c/h, httpd-cgi.c/h, httpd-fs.c/h, httpd-fsdata.c/h,
│                                   http-strings.c/h, webopts.h, httpd-fs/style*.css
│
├── net/
│   └── uip/                      🔵 uIP 網路堆疊（原 SDK/DM9051/*.c）—— 核心邏輯不改，拿來就用
│       ├── core/                  uip.c, uip_arp.c, uip_init.c, uip_timer.c,
│       │                          uip_keepalive.c, uip-split.c, uiplib.c, uip-fw.c, uip-neighbor.c
│       ├── apps/                  dhcpc.c, resolv.c, psock.c, memb.c, trans_level.c
│       └── include/               uip*.h 標頭
│
└── port/                         🟡 平台相依層 —— 唯一需要為新硬體動手改寫的地方
    ├── at_port.h / at_port.c      UART / Flash-NV / Reset / Timer HAL（見 02）
    ├── clock-arch.c               uIP tick 來源，改讀目標平台計時器
    ├── net_if/
    │   ├── tapdev.c                uIP↔NIC 收發膠合，改呼叫目標乙太網晶片驅動
    │   └── DM9051.c                沿用 DM9051 晶片則複製＋SPI 走 at_port；換晶片則整份替換
    └── at_net_shim.h               備用方案專用：僅目標平台無法共存 uIP 時才加入此檔
```

**分類原則**：

| 目錄 | 對應圖例 | 是否要改 | 說明 |
|------|----------|----------|------|
| `app/` | 🟢 | **不改邏輯**，只改 include | AT_Command / etherbridge / webserver，三者共用同一份 `at_type`/`at_show`，維持同層方便交叉參照 |
| `net/uip/` | 🔵 | **不改邏輯** | 拆成 `core`（協定狀態機）/ `apps`（DHCP/DNS/psock 等輔助）/ `include`，方便日後升級 uIP 版本時只比對這三個子目錄 |
| `port/` | 🟡 | **要改** | 每個新平台唯一需要動手寫的地方；`net_if/` 獨立出來是因為它同時涉及 uIP 膠合與晶片驅動，方便日後換晶片時只動這個子目錄 |

> 這個結構讓「複製就好、不用看」（`app/`、`net/uip/`）與「這個平台一定要重寫」（`port/`）在目錄層級一眼區分，之後升級 uIP 版本或換 MCU 平台時，改動範圍清楚侷限在對應目錄，不會誤觸不該動的程式碼。
