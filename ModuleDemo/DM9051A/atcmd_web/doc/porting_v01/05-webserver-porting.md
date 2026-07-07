# 05 — Webserver 移植分析

> 檔案位置：
> - `SDK/DM9051/App/webserver/`（主要版本，整合於 `Project.uvprojx`）
> - `SDK/DM9051/TCP_App/webserver/`（TCP_App 對應版本，內容相同）
>
> 移植重要性：Webserver 提供設備的 **HTTP 網頁設定介面**，讓使用者透過瀏覽器設定 IP/role/DHCP/DNS/port/keepalive 等所有 AT 參數。這些設定直接對應 `at_type`/`at_show` 結構，與 AT_Command 模組共用同一份資料，因此移植時必須一起處理。

> **v1.1 策略更新：webserver 隨 `SDK/DM9051/App/` 整包複製，且因 uIP 網路堆疊整包保留（見 [`03`](03-uip-api-mapping.md)），`httpd.c` 的 `psock`/`protothread` HTTP 引擎也不需要換成其他 HTTP server。** 本文件下方「換成目標 HTTP server」的段落改列為**備用方案**（僅在目標平台無法共存 uIP 時才需要）。

---

## 1. 檔案分類與移植難度

| 檔案 | 角色 | uIP 相依程度 | AT 相依程度 | 移植策略 |
|------|------|-------------|-------------|----------|
| `httpd.c` | HTTP daemon 主體 | **高**（psock/protothread） | 低（`#include "atcommand.h"`） | 直接複製（因 uIP 的 psock/protothread 整包保留） |
| `httpd-cgi.c` | **客製化 CGI 設定頁** | 高（`uip_appdata`, `PSOCK_*`, `uip_conns`） | **高**（大量 at_show/at_type） | 直接複製，核心業務邏輯不改 |
| `httpd-fs.c` / `httpd-fsdata.c` | ROM 靜態檔案系統 | 低（自管 buffer） | 無 | 可幾乎原樣搬移 |
| `http-strings.c` / `.h` | HTTP 字串常數 | 無 | 無 | 直接複製 |
| `httpd.h` | 結構定義 | 中（psock, pt） | 無 | 直接複製 |
| `httpd-cgi.h` | CGI 框架巨集 | 中（PSOCK 相關） | 無 | 直接複製 |
| `webopts.h` | 編譯選項（`WEB_BAUD_UPDATE`） | 無 | 無 | 直接複製，按需啟用 |
| `httpd-fs/style*.css` | 靜態 CSS | 無 | 無 | 直接複製 |

> 上表為**主策略**（uIP 整包保留）下的結論。若目標平台真的無法共存 uIP，才需要走第 5 節「層 2」的備用方案（換 HTTP 引擎）。

---

## 2. 客製化修改點（已知與原始 uIP 的差異）

| 檔案 | 修改 | 位置 |
|------|------|------|
| `httpd.h` | `inputbuf` 從 256 擴大至 **300 bytes** | [httpd.h:49](../../SDK/DM9051/App/webserver/httpd.h) (joseph 20240604) |
| `httpd-cgi.c` | 加入 13 個完整 AT 設定 CGI handler | 見下節 |
| `webopts.h` | 新增 `WEB_BAUD_UPDATE` 條件編譯選項 | webopts.h:4 |

---

## 3. 客製化 CGI Handler 清單（`httpd-cgi.c`）

所有 handler 使用 `PSOCK_GENERATOR_SEND` 模式，對 `uip_appdata` 寫入 HTML 片段後由 uIP 送出。

| CGI 名稱 | Script tag | 功能 | 存取的 at_show / at_type 欄位 |
|----------|-----------|------|-------------------------------|
| `current` | `current-stats` | 顯示目前執行態（role/DHCP/DNS/MAC/IP/mask/gw/DNS狀態/連線狀態） | role, dhcpc_mode, dns_mode, hostip/mask/gw，另讀 `uip_ethaddr`/`uip_hostaddr`/`uip_netmask`/`uip_draddr` |
| `parameter1` | `parameter1-stats` | 顯示設定頁1（role/DHCP/keepalive/DNS模式） | role, dhcpc_mode, keepalive_mode, dns_mode |
| `parameter2` | `parameter2-stats` | 顯示設定頁2（MAC/IP/mask/gw/tcplport/tcpconn/udplport/udpconn/baud） | 幾乎所有欄位 + eeprom_show.macaddr |
| `hipconfig` | `hipconfig-stats` | 設定表單：本機 IP 輸入框 | hostip |
| `mipconfig` | `mipconfig-stats` | 設定表單：Netmask 輸入框 | hostmask |
| `gipconfig` | `gipconfig-stats` | 設定表單：Gateway 輸入框 | hostgw |
| `tcps_lport` | `tcps_lport-stats` | 設定表單：TCP/UDP Listen Port | t_lport / u_lport（依 role） |
| `tcpc_raddr` | `tcpc_raddr-stats` | 設定表單：遠端 IP:Port | tcp_raddr/rport 或 udp_raddr/rport（依 role） |
| `DNS_Mode` | `DNS_Mode-stats` | 設定表單：DNS 開關 select | dns_mode |
| `dnsipconfig` | `dnsipconfig-stats` | 設定表單：DNS Server IP | dns_saddr |
| `DNS_srvname` | `DNS_srvname-stats` | 設定表單：DNS Server Name + (可選 Baud) | dns_srvname（`WEB_BAUD_UPDATE` 時也含 baud） |
| `role_mode` | `role_mode-stats` | 設定表單：Role select dropdown | role（+ `reconn_addr` 外部變數） |
| `ipconfig_page1` | `ipconfig_page1-stats` | 設定表單：DHCP/Static IP select | dhcpc_mode |

### 外部相依（CGI 中直接使用的 uIP 全域）

| 變數 | 用途 | 來源 |
|------|------|------|
| `uip_appdata` | 寫入 HTML 輸出 buffer | uip.h |
| `UIP_APPDATA_SIZE` | buffer 最大長度限制 | uip.h / uip-conf.h |
| `uip_conns[]` | 判斷連線是否 CLOSED | uip.h |
| `uip_ethaddr` | 顯示 MAC 位址（current-stats） | uip.h |
| `uip_hostaddr` / `uip_netmask` / `uip_draddr` | 顯示目前 IP/mask/gw（current-stats） | uip.h |
| `dhs_staus_msg` | DNS 狀態訊息字串 | etherbridge/trans_level |
| `reconn_addr` | 連線狀態字串 | etherbridge |

---

## 4. HTTP 頁面提交路徑（Save 按鈕流程）

Web 設定頁按「Save」→ HTTP POST → `httpd.c` 的 `handle_input()` 解析 → 呼叫 `eth_rcv_strtok_process()` + `eth_rcv_atcmd_process()` ([atcommand.c:110,175](../../SDK/DM9051/App/AT_Command/atcommand.c)) 套用設定 → 呼叫 `Write_AT_Show_DataFlash()` 存到 Flash。

> **這條路徑讓 Web 設定直接觸發 AT_Command 的儲存機制**，移植時要確保兩者連動。

---

## 5. 移植策略（主策略：整包複製）

### 層 1：靜態資源層（最簡單，可直接搬）

直接複製：
- `httpd-fs.c` / `httpd-fsdata.c`（ROM 虛擬 FS）
- `http-strings.c` / `.h`（HTTP header 字串常數）
- `httpd-fs/style*.css`（內嵌進 fsdata）
- `webopts.h`（選項巨集）

### 層 2：HTTP 引擎層（因 uIP 保留，直接複製 `httpd.c`）

原 `httpd.c` 使用 **uIP psock/protothread** 驅動 HTTP。既然 uIP 整包複製到新專案（見 [`03`](03-uip-api-mapping.md)），`httpd.c` **不需要換框架，直接複製即可**，`UIP_APPCALL` 分派方式與原韌體一致（`app_call.c` 呼叫 `httpd_appcall()`）。

> **備用方案**（僅目標平台無法共存 uIP 時才需要）：改用目標平台的 HTTP 框架，例如 lwIP 內建 `httpd`、FreeRTOS+TCP HTTP Server、Mongoose、libmicrohttpd 等，並把 `httpd-cgi.c` 的 handler 改成對應框架的 CGI callback 格式。

### 層 3：CGI 業務邏輯層（核心，直接複製）

`httpd-cgi.c` 內的 `generate_*_stats` 函式是**真正的業務邏輯**（AT 設定讀取與顯示）。因 uIP 保留，這層**不需要改寫**：

- `sprintf(uip_appdata, ...)`、`uip_hostaddr/netmask/draddr`、`uip_ethaddr` 等呼叫全部原樣可用。
- 對 `at_show`/`at_type`/`eeprom_show`/`eeprom_type` 的直接存取不變。
- POST 解析（Save 流程）沿用 `eth_rcv_strtok_process` / `eth_rcv_atcmd_process`。

> 若走備用方案（換 HTTP 引擎），才需要：把 `generate_*_stats(arg)` 改成目標框架 handler 函式、把 `sprintf(uip_appdata,...)` 改成目標框架的 response buffer、把 `uip_hostaddr/netmask/draddr` 改成 `atshim_get_ip/mask/gw`（配合 `at_net_shim.h`）、把 `uip_ethaddr` 改成目標平台的 MAC 取得函式。

---

## 6. webserver 移植追加至 04 檢查清單

下列為額外的 Step 7（接在 AT_Command Step 6 之後）：

### Step 7 — Webserver 移植

- [ ] 複製 `SDK/DM9051/App/webserver/` 整個目錄到新專案的 `app/webserver/`（見 [README 第 4 節](README.md#4-建議的新專案目錄結構v11-新增) 目錄結構）：`httpd-fs.c` / `httpd-fsdata.c` / `http-strings.c` / css / `webopts.h` / `httpd.c` / `httpd-cgi.c` / `httpd.h` / `httpd-cgi.h`，**核心邏輯不改**
- [ ] 確認 `app_call.c`（或其等價的 `UIP_APPCALL` 分派）正確呼叫 `httpd_appcall()`
- [ ] 確認 POST/Save 流程觸發 `eth_rcv_atcmd_process` + `Write_AT_Show_DataFlash`
- [ ] 在主迴圈呼叫 `httpd_init()`
- [ ]（僅無法共存 uIP 時）走備用方案：選定目標 HTTP 框架、把 13 個 `generate_*_stats` 改寫成該框架的 handler

**驗收**：瀏覽器開啟設定頁，可看到目前 IP/role/DHCP 等設定；修改後按 Save，重開後設定保留。

---

## 7. App/ vs TCP_App/ 差異

`SDK/DM9051/TCP_App/webserver/` 與 `App/webserver/` 目前內容相同（含 `http-strings` 工具腳本）。
移植時以 **`App/webserver/`** 為基準（此為 `Project.uvprojx` 引用的版本）；`TCP_App/` 視需求對齊。
