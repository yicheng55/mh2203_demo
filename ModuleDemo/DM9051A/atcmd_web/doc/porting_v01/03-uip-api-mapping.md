# 03 — uIP 移植規格（主策略）與 uIP API 對照表（備用方案）

> **v1.1 策略更新：uIP 整包複製到新專案，不再嘗試改接 lwIP / socket 等目標網路堆疊。**
> 原因：uIP 是單緩衝、事件回呼模型（`uip_appdata`、`UIP_APPCALL`），和 lwIP/socket 的事件模型差異太大；AT_Command、`etherbridge`、`httpd`（webserver）三個模組都直接依賴這個模型。與其把三個模組全部改寫並冒回歸風險，不如把 uIP 原始碼原封不動搬過去——**AT_Command 一行都不用改網路呼叫**。

---

## 0. 主策略：uIP 整包複製，只換兩個硬體膠合檔

複製 `SDK/DM9051/` 下所有 uIP 核心檔（`uip.c`, `uip_arp.c`, `uip_init.c`, `uip_timer.c`, `uip_keepalive.c`, `uip-split.c`, `uiplib.c`, `uip-fw.c`, `uip-neighbor.c`, `dhcpc.c`, `resolv.c`, `psock.c`, `memb.c`, `trans_level.c`）到新專案，**邏輯完全不改**。真正需要因平台而重寫的只有：

| 檔案 | 現況（AT32F413） | 移植動作 |
|------|-------------------|----------|
| [`clock-arch.c`](../../SDK/DM9051/clock-arch.c) | `clock_time()` 回傳全域 `g_RunTime`（由 systick ISR 遞增） | 改讀目標平台的 tick 來源（RTOS tick / systick 計數器），維持 `clock_time_t` 語意 |
| [`tapdev.c`](../../SDK/DM9051/tapdev.c) | `tapdev_init/read/send` 直接呼叫 `DM9051_Init()` / `DM9051_RX()` / `DM9051_TX()` | 若沿用 DM9051 晶片：改走 `at_port` 的 SPI 介面即可；若換乙太網晶片：改呼叫新驅動的 init/收/發 API |
| [`DM9051.c`](../../SDK/DM9051/DM9051.c) | SPI1 + AT32 GPIO/EXTI | 沿用晶片則複製＋SPI 呼叫改走 `at_port`；換晶片則整份換掉 |

**驗收**：`uip_input()` / `uip_periodic()` 的呼叫時機、`UIP_APPCALL` 分派（`app_call.c`）與原韌體一致；只有 tick 來源與收發封包的底層路徑不同。

**風險**：
- uIP 假設 C89/C99、`u8_t/u16_t` 型別大小與位元組序（`BYTE_ORDER`）需與 `uip-conf.h` 一致，換平台時確認 endianness 設定正確（AT32F413 為小端）。
- `clock_time_t` 溢位/精度需與 `uip_timer.c` 的逾時判斷相容。

> 只有當目標平台**強制**要求使用其他網路堆疊（例如 RTOS 已內建 lwIP 且無法並存 uIP）時，才需要走下面的**備用方案**：改寫 `at_net_shim.h`，把 AT_Command 用到的 uIP 呼叫對接到 lwIP/socket。

---

## 備用方案：uIP → lwIP / socket API 對照表

> 對應範本：[`templates/at_net_shim.h`](templates/at_net_shim.h)
> 原則：AT_Command 用到的 uIP 呼叫，分兩類處理：
> - **A 類｜純資料巨集** — 只做位元組運算，與堆疊無關 → **原樣保留**（shim 內重定義即可）。
> - **B 類｜事件/連線 API** — 與堆疊事件模型強耦合 → **必須 shim**，對接目標堆疊（lwIP / socket）。

下表「行號」為 AT_Command 內實際出現處（無則留空表示宣告於 uip 標頭、本模組間接相關）。

---

## 1. 型別對應（最重要）

| uIP 型別 | 定義 | lwIP 對應 | socket 對應 | 移植做法 |
|----------|------|-----------|-------------|----------|
| `uip_ipaddr_t` | `u16_t[2]`（IPv4=32bit） | `ip4_addr_t` | `struct in_addr` / `uint32_t` | shim 內 `typedef`；保留 `uip_ipaddr*` 巨集存取 |
| `struct uip_conn` | TCP 連線狀態 | `struct tcp_pcb` | fd（OS 管理） | B 類，shim 包裝 |
| `struct uip_udp_conn` | UDP 連線狀態 | `struct udp_pcb` | fd | B 類，shim 包裝 |
| `u16_t` 埠（network order） | — | `u16_t` | `uint16_t` | 一致，注意 byte order |

`struct at_funcation`（[atcommand.h:68](../../SDK/DM9051/App/AT_Command/atcommand.h)）含多個 `uip_ipaddr_t` 欄位（`hostip/hostmask/hostgw/tcp_raddr/udp_raddr/dns_saddr/srvname_saddr`）。**只要 shim 提供 `uip_ipaddr_t` 型別與存取巨集，這個結構不用改。**

---

## 2. A 類｜純資料巨集（原樣保留）

這些只是把 4 個 byte 拼成/拆出 32-bit IP，**與網路堆疊無關**，shim 直接保留或重定義：

| API | AT_Command 用處 | 語意 |
|-----|------------------|------|
| `uip_ipaddr(addr,a,b,c,d)` | at_tcpip_cmd.c:186,235,274 | 由 4 byte 組 IP |
| `uip_ipaddr1..4(addr)` | 多處（at_base/at_tcpip/atcommand） | 取第 1~4 byte |
| `uip_ipaddr_copy(d,s)` | at_base_cmd.c:361 | 複製 IP |
| `uip_ipaddr_cmp / maskcmp / mask` | （結構比較） | 比較/遮罩 |
| `HTONS(n)` | at_tcpip_cmd.c:187,191,236,240,276,298 | host→network 16-bit |

> 移植：shim 內保留這些巨集定義（可直接抄 uip.h 的純運算版本），AT_Command 程式碼一字不改。

---

## 3. B 類｜需 shim 的事件/連線 API

### 3.1 IP / 介面設定

| uIP API | AT_Command | lwIP | socket |
|---------|-----------|------|--------|
| `uip_sethostaddr(a)` | atcommand.c:91 | `netif_set_ipaddr(&netif,a)` | 設定本機 IP（OS 層） |
| `uip_setnetmask(a)` | atcommand.c:92, at_tcpip_cmd.c:344 | `netif_set_netmask(&netif,a)` | — |
| `uip_setdraddr(a)` | atcommand.c:93, at_tcpip_cmd.c:364 | `netif_set_gw(&netif,a)` | — |
| `uip_gethostaddr(&x)` | at_tcpip_cmd.c:243,300 | 讀 `netif.ip_addr` | — |

> shim：`#define uip_sethostaddr(a) atshim_set_ip(a)`，在目標平台實作 `atshim_set_ip` 呼叫 `netif_*`。

### 3.2 TCP

| uIP API | AT_Command | lwIP | socket |
|---------|-----------|------|--------|
| `tcp_init()` | at_tcpip_cmd.c:265,297 | （lwip_init 內含） | — |
| `uip_listen(HTONS(port))` | at_tcpip_cmd.c:298 | `tcp_bind`+`tcp_listen` | `socket`+`bind`+`listen` |
| `uip_connect(&ip,HTONS(port))` | at_tcpip_cmd.c:276 | `tcp_connect` | `socket`+`connect` |
| `uip_close / uip_abort` | （連線收尾） | `tcp_close`/`tcp_abort` | `close` |

### 3.3 UDP

| uIP API | AT_Command | lwIP | socket |
|---------|-----------|------|--------|
| `uip_udp_new(&ip,HTONS(port))` | at_tcpip_cmd.c:187,236 | `udp_new`+`udp_connect` | `socket(SOCK_DGRAM)` |
| `uip_udp_bind(conn,HTONS(port))` | at_tcpip_cmd.c:191,240 | `udp_bind` | `bind` |
| `uip_udp_send(len)` / `uip_send` | （經 appdata） | `udp_send(pbuf)` | `sendto`/`send` |

### 3.4 資料存取（事件模型差異最大）

| uIP | AT_Command | 差異 |
|-----|-----------|------|
| `uip_appdata`（全域指標） | atcommand.c:310,315,663,673（`sprintf(uip_appdata,…)`） | uIP 用單一全域 buffer；lwIP 用 `pbuf` 鏈、socket 用應用 buffer |
| `uip_len` / `uip_datalen()` | — | 收到長度 |

> **這是 shim 最關鍵處**：AT_Command 在送回應時直接 `sprintf` 進 `uip_appdata`。移植時提供 `atshim_appbuf()` 回傳一塊應用層 TX buffer 指標，並用 `atshim_send(len)` 真正送出（lwIP 包成 pbuf / socket 直接 send）。把原碼的 `sprintf(uip_appdata,…)` + 送出，改成 `sprintf(atshim_appbuf(),…)` + `atshim_send(...)`。

### 3.5 DHCP / DNS / ARP

| uIP API | AT_Command 關聯 | lwIP | socket |
|---------|------------------|------|--------|
| `dhcpc_init / dhcpc_request` | at_base_cmd.c:274-282（`dhcpc_mode` 旗標）, atcommand.c:47,203,412 | `dhcp_start(&netif)` | OS dhclient |
| `resolv_conf / resolv_query / resolv_lookup` | at_tcpip_cmd.c:122-127 | lwIP DNS（`dns_gethostbyname`） | `getaddrinfo` |
| `uip_arp_*` | （uIP 內部） | lwIP 自管 | OS 自管 |

> DHCP/DNS 在 AT_Command 多半是**設定旗標 + 顯示**，真正動作在 uIP 層。移植時把「啟用 DHCP/DNS」對接成目標堆疊的對應呼叫即可。

---

## 4. 事件模型轉換要點

uIP 是**單緩衝、事件回呼**（`app_call.c` 的 `UIP_APPCALL` 被 uIP 在 connected/newdata/acked/poll 等事件時呼叫）。移植時：

| uIP 事件 | lwIP | socket |
|----------|------|--------|
| `uip_connected()` | `tcp_connected` callback | `connect` 返回/`accept` |
| `uip_newdata()` + `uip_appdata` | `tcp_recv` callback(pbuf) | `recv` |
| `uip_acked()` | `tcp_sent` callback | （TCP 自管） |
| `uip_poll()` | `tcp_poll` callback | select/poll 逾時 |
| `uip_closed/aborted/timedout` | `tcp_err`/close callback | `recv`==0/錯誤 |

> AT_Command 本身對這些事件巨集的直接使用很少（主要在 `app_call.c`/`etherbridge.c`）。若連 etherbridge 一起移植，需把 `app_call` 的事件分派改寫成目標堆疊的 callback 風格。**只移植 AT 命令層的話，重點是 3.1~3.4。**

---

## 5. shim 收斂表

| AT_Command 內呼叫 | shim 提供 | 目標平台實作 |
|-------------------|-----------|--------------|
| `uip_ipaddr_t` + `uip_ipaddr*` / `HTONS` | 型別 + A 類巨集（保留） | 無需改 |
| `uip_sethostaddr/setnetmask/setdraddr/gethostaddr` | `atshim_set_ip/mask/gw/get_ip` | `netif_*` |
| `uip_listen/uip_connect` | `atshim_tcp_listen/connect` | `tcp_*` / socket |
| `uip_udp_new/uip_udp_bind` | `atshim_udp_new/bind` | `udp_*` / socket |
| `sprintf(uip_appdata,…)` + 送出 | `atshim_appbuf()` + `atshim_send()` | pbuf / send |
| `dhcpc_* / resolv_*` | `atshim_dhcp_*` / `atshim_dns_*` | `dhcp_start` / DNS |

> 以上為**備用方案**表格，僅在無法保留 uIP 時才需要。預設請走本檔開頭「0. 主策略」：複製 uIP，只改 `clock-arch.c` / `tapdev.c`。
>
> 下一份：[`04-ai-porting-checklist.md`](04-ai-porting-checklist.md) — 照表動手。
