# porting_manifest.md

> 本檔由 `Project.uvprojx` 實際列檔 + `doc/porting_v01/README.md` 策略翻譯而來。
> 目的：把 `SDK/DM9051/App/` + uIP stock + platform HAL 差異點，整理成可執行的 porting manifest。

---

## 1. Source Inventory（來自 Project.uvprojx Target #1）

`Project.uvprojx` 只有 Target #1 **`STM32100E-EVAL`**（device=`AT32F413C8T7`）有效，其餘 5 個 target 為舊 STM32 殘留設定，不參與 porting。

### 1.1 Build macro / include

- Define: `AT32F413Cx_MD, USE_STDPERIPH_DRIVER, AT_START_F413_V1_0`
- IncludePath:
  - `..\DM9051\include`
  - `..\Libraries\AT32F4xx_StdPeriph_Driver\inc`
  - `..\User`
  - `..\DM9051\App\webserver`
  - `..\DM9051\App\etherbridge`
  - `..\DM9051\App\AT_Command`
  - `..\bsp`
  - `..\bsp\inc`
  - `..\libraries_nmdk\cmsis\cm4\core_support`
  - `..\Libraries\CMSIS\CM4\DeviceSupport`
  - `..\Libraries\AT32F4xx_StdPeriph_Driver\inc`

### 1.2 User Group

| 來源 | 角色 | 分類 |
|------|------|------|
| `app_call.c` | uIP `UIP_APPCALL` 分派 | 🔵 app/uIP |
| `main.c` | 進入點、主迴圈整合 | 🟡 參考樣本 |
| `windows_app.c` | 雜項初始化 | ⚪ 參考/可省略 |
| `system_at32f4xx.c` | 系統時脈、平台初始化 | ⚪ 平台 HAL 保留 |
| `at32f4xx_it.c` | 中斷向量表 | ⚪ 平台保留 |
| `at32_board_uart.c` | UART DMA / FIFO HAL | 🟡 -> `at_port` |
| `at32f4xx_assert.c` | assert | ⚪ 留作參考 |
| `spi_config.c` | SPI 設定 | 🟡 -> DM9051 SPI glue |

### 1.3 DM9051 Group（uIP core + glue）

| 來源 | 角色 | 分類 |
|------|------|------|
| `uip.c` | TCP/IP 核心 | 🔵 net/uip/core |
| `uip_arp.c` | ARP | 🔵 net/uip/core |
| `uip_init.c` | uIP init | 🔵 net/uip/core |
| `uip_keepalive.c` | keepalive | 🔵 net/uip/core |
| `uip_timer.c` | timer | 🔵 net/uip/core |
| `uip-fw.c` | firewall hook（未使用但編譯） | 🔵 net/uip/core |
| `uiplib.c` | IP helper | 🔵 net/uip/core |
| `uip-neighbor.c` | neighbor | 🔵 net/uip/core |
| `uip-split.c` | split | 🔵 net/uip/core |
| `dhcpc.c` | DHCP client | 🔵 net/uip/apps |
| `resolv.c` | DNS resolver | 🔵 net/uip/apps |
| `psock.c` | protosocket | 🔵 net/uip/apps |
| `memb.c` | mem block pool | 🔵 net/uip/apps |
| `trans_level.c` | 傳輸層輔助 | 🔵 net/uip/apps |
| `clock-arch.c` | tick 來源（`g_RunTime`） | 🟡 port/clock-arch.c |
| `tapdev.c` | NIC glue（DM9051 RX/TX） | 🟡 port/net_if/tapdev.c |
| `DM9051.c` | DM9051 SPI MAC/PHY | ⚪ 保留驅動邏輯，SPI 改走 at_port |

> 注意：原 `TCP_App/` 與 `App/` 為平行實作，以專案編譯的 `App/` 為移植基準。

### 1.4 AT_Command App

| 來源 | 角色 | 分類 |
|------|------|------|
| `atcommand.c` | AT parser 主體、`at_type`、`uip_appdata` 存取 | 🟢 app/at_command |
| `at_base_cmd.c` | 基礎指令 + reset | 🟢 app/at_command |
| `at_tcpip_cmd.c` | TCP/IP AT 指令 | 🟢 app/at_command |
| `dataflash.c` | Flash 參數讀寫 | 🟢 app/at_command |
| `atcommand.h` | struct / 巨集 / 原型 | 🟢 app/at_command |
| `dataflash.h` | test pattern | 🟢 app/at_command |

### 1.5 etherbridge App

| 來源 | 角色 | 分類 |
|------|------|------|
| `etherbridge.c` | UART <-> Eth bridge | 🟢 app/etherbridge |
| `etherbridge.h` | 原型/型別 | 🟢 app/etherbridge |

### 1.6 webserver App

| 來源 | 角色 | 分類 |
|------|------|------|
| `httpd.c` | HTTP daemon (psock / protothread) | 🟢 app/webserver |
| `httpd-cgi.c` | 13 個客製 CGI handler | 🟢 app/webserver |
| `httpd-fs.c` | ROM fs 讀取 | 🟢 app/webserver |
| `httpd-fsdata.c` | fsdata（彙集靜態資源） | 🟢 app/webserver |
| `http-strings.c` | HTTP 字串 | 🟢 app/webserver |
| `webopts.h` | 編譯選項 | 🟢 app/webserver |
| `httpd.h` / `httpd-cgi.h` / `httpd-fs.h` / `httpd-fsdata.h` / `http-strings.h` | 標頭 | 🟢 app/webserver |
| `httpd-fs/style.css`, `style2.css` | 靜態 CSS | 🟢 app/webserver/httpd-fs/ |

### 1.7 uIP include（stock header）

| 來源 | 目標 | 備註 |
|------|------|------|
| `SDK/DM9051/include/uip.h` | `net/uip/include/uip.h` | |
| `SDK/DM9051/include/uip-conf.h` | `net/uip/include/uip-conf.h` | |
| `SDK/DM9051/include/uip_arch.h` | `net/uip/include/uip_arch.h` | |
| `SDK/DM9051/include/uipopt.h` | `net/uip/include/uipopt.h` | |
| `SDK/DM9051/include/uip_arp.h` | `net/uip/include/uip_arp.h` | |
| `SDK/DM9051/include/uip_init.h` | `net/uip/include/uip_init.h` | |
| `SDK/DM9051/include/uip_timer.h` | `net/uip/include/uip_timer.h` | |
| `SDK/DM9051/include/uip_keepalive.h` | `net/uip/include/uip_keepalive.h` | |
| `SDK/DM9051/include/uip-split.h` | `net/uip/include/uip-split.h` | |
| `SDK/DM9051/include/uip-fw.h` | `net/uip/include/uip-fw.h` | |
| `SDK/DM9051/include/uip-neighbor.h` | `net/uip/include/uip-neighbor.h` | |
| `SDK/DM9051/include/tapdev.h` | `net/uip/include/tapdev.h` | |
| `SDK/DM9051/include/trans_level.h` | `net/uip/include/trans_level.h` | |
| `SDK/DM9051/include/trans_level2.h` | `net/uip/include/trans_level2.h` | |
| `SDK/DM9051/include/resolv.h` | `net/uip/include/resolv.h` | |
| `SDK/DM9051/include/psock.h` | `net/uip/include/psock.h` | |
| `SDK/DM9051/include/memb.h` | `net/uip/include/memb.h` | |
| `SDK/DM9051/include/dhcpc.h` | `net/uip/include/dhcpc.h` | |
| `SDK/DM9051/include/clock.h` | `net/uip/include/clock.h` | |
| `SDK/DM9051/include/clock-arch.h` | `net/uip/include/clock-arch.h` | |
| `SDK/DM9051/include/includes.h` | `net/uip/include/includes.h` | |
| `SDK/DM9051/include/pt.h` | `net/uip/include/pt.h` | |
| `SDK/DM9051/include/lc.h` | `net/uip/include/lc.h` | |
| `SDK/DM9051/include/lc-switch.h` | `net/uip/include/lc-switch.h` | |
| `SDK/DM9051/include/lc-addrlabels.h` | `net/uip/include/lc-addrlabels.h` | |
| `SDK/DM9051/include/uiplib.h` | `net/uip/include/uiplib.h` | |
| `SDK/DM9051/include/DM9051.h` | `net/uip/include/DM9051.h`（僅參考） | 驅動 header |

---

## 2. Porting 總覽：不要改什麼、要改什麼

| 目錄 | 要不要改 | 說明 |
|------|----------|------|
| `app/` | ❌ 核心邏輯不改 | 只做 include 調整（`at_port.h`、`compat/uip_mh2203/uip_mh2203.h`） |
| `net/uip/` | ❌ 核心邏輯不改 | uIP stock 整包複製 |
| `port/` | ✅ 新平台重寫 | UART/Flash/Reset/Timer/NIC |
| `compat/uip_mh2203/` | ✅ API 橋接 | 不一致 API / 巨集相容 |
| `doc/porting_v01/` | ✅ 僅參考 | 不編譯 |
| `ref/at32/` | ✅ 僅參考 | 供新系統加入使用，不改動 |

---

## 3. 完整檔案對照

> Action:
> - **COPY** = 原樣複製到新專案
> - **REF** = 保留在原處做為參考，新專案自己重寫或另存
> - **COPY + ADAPT** = 複製後僅改 include / call site，不更動內部邏輯
> - **COPY + REWRITE** = 複製骨架，必須改寫底層硬體呼叫

| 來源 | 目標 | Action |
|------|------|--------|
| `SDK/User/app_call.c` | `app/at_command/app_call.c` | **COPY** |
| `SDK/User/main.c` | `app/main.c`（參考） | **REF**（不 compile，做整合樣本） |
| `SDK/User/windows_app.c` | `ref/at32/User/windows_app.c` | **REF** |
| `SDK/User/system_at32f4xx.c` | `ref/at32/User/system_at32f4xx.c` | **REF** |
| `SDK/User/at32f4xx_it.c` | `ref/at32/User/at32f4xx_it.c` | **REF** |
| `SDK/User/at32_board_uart.c` | `ref/at32/User/at32_board_uart.c` | **REF** |
| `SDK/User/at32f4xx_assert.c` | `ref/at32/User/at32f4xx_assert.c` | **REF** |
| `SDK/User/spi_config.c` | `ref/at32/User/spi_config.c` | **REF** |
| `SDK/DM9051/clock-arch.c` | `port/clock-arch.c` | **COPY + REWRITE** (tick -> new timer) |
| `SDK/DM9051/dhcpc.c` | `net/uip/apps/dhcpc.c` | **COPY** |
| `SDK/DM9051/DM9051.c` | `ref/at32/net/DM9051.c` | **REF**（驅動邏輯保留，SPI/GPIO/EXTI 行號留給新板參考） |
| `SDK/DM9051/memb.c` | `net/uip/apps/memb.c` | **COPY** |
| `SDK/DM9051/psock.c` | `net/uip/apps/psock.c` | **COPY** |
| `SDK/DM9051/resolv.c` | `net/uip/apps/resolv.c` | **COPY** |
| `SDK/DM9051/tapdev.c` | `port/net_if/tapdev.c` | **COPY + REWRITE** (NIC init/RX/TX) |
| `SDK/DM9051/uip.c` | `net/uip/core/uip.c` | **COPY** |
| `SDK/DM9051/uip_arp.c` | `net/uip/core/uip_arp.c` | **COPY** |
| `SDK/DM9051/uip_init.c` | `net/uip/core/uip_init.c` | **COPY** |
| `SDK/DM9051/uip_keepalive.c` | `net/uip/core/uip_keepalive.c` | **COPY** |
| `SDK/DM9051/uip_timer.c` | `net/uip/core/uip_timer.c` | **COPY** |
| `SDK/DM9051/uip-fw.c` | `net/uip/core/uip-fw.c` | **COPY** |
| `SDK/DM9051/uiplib.c` | `net/uip/core/uiplib.c` | **COPY** |
| `SDK/DM9051/uip-neighbor.c` | `net/uip/core/uip-neighbor.c` | **COPY** |
| `SDK/DM9051/uip-split.c` | `net/uip/core/uip-split.c` | **COPY** |
| `SDK/DM9051/trans_level.c` | `net/uip/apps/trans_level.c` | **COPY** |
| `SDK/DM9051/App/AT_Command/atcommand.c` | `app/at_command/atcommand.c` | **COPY + ADAPT** (add include `at_port.h`) |
| `SDK/DM9051/App/AT_Command/at_base_cmd.c` | `app/at_command/at_base_cmd.c` | **COPY + ADAPT** (reset via compat) |
| `SDK/DM9051/App/AT_Command/at_tcpip_cmd.c` | `app/at_command/at_tcpip_cmd.c` | **COPY + ADAPT** (uart via compat) |
| `SDK/DM9051/App/AT_Command/dataflash.c` | `app/at_command/dataflash.c` | **COPY + ADAPT** (flash via compat) |
| `SDK/DM9051/App/AT_Command/atcommand.h` | `app/at_command/atcommand.h` | **COPY** |
| `SDK/DM9051/App/AT_Command/dataflash.h` | `app/at_command/dataflash.h` | **COPY** |
| `SDK/DM9051/App/etherbridge/etherbridge.c` | `app/etherbridge/etherbridge.c` | **COPY + ADAPT** (uart dma fn -> compat) |
| `SDK/DM9051/App/etherbridge/etherbridge.h` | `app/etherbridge/etherbridge.h` | **COPY** |
| `SDK/DM9051/App/webserver/httpd.c` | `app/webserver/httpd.c` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-cgi.c` | `app/webserver/httpd-cgi.c` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-fs.c` | `app/webserver/httpd-fs.c` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-fsdata.c` | `app/webserver/httpd-fsdata.c` | **COPY** |
| `SDK/DM9051/App/webserver/http-strings.c` | `app/webserver/http-strings.c` | **COPY** |
| `SDK/DM9051/App/webserver/webopts.h` | `app/webserver/webopts.h` | **COPY** |
| `SDK/DM9051/App/webserver/httpd.h` | `app/webserver/httpd.h` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-cgi.h` | `app/webserver/httpd-cgi.h` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-fs.h` | `app/webserver/httpd-fs.h` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-fsdata.h` | `app/webserver/httpd-fsdata.h` | **COPY** |
| `SDK/DM9051/App/webserver/http-strings.h` | `app/webserver/http-strings.h` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-fs/style.css` | `app/webserver/httpd-fs/style.css` | **COPY** |
| `SDK/DM9051/App/webserver/httpd-fs/style2.css` | `app/webserver/httpd-fs/style2.css` | **COPY** |
| `SDK/DM9051/include/*.h` | `net/uip/include/*.h` | **COPY** |
| `Libraries/AT32F4xx_StdPeriph_Driver/src/*.c` | `ref/at32/Libraries/AT32F4xx_StdPeriph_Driver/src/` | **REF** |
| `Libraries/CMSIS/.../startup_at32f413cx_md.s` | `ref/at32/Libraries/CMSIS/...` | **REF** |
| `Libraries/CMSIS/.../at32f413.h` | `ref/at32/Libraries/CMSIS/...` | **REF** |
| `bsp/inc/*.h` + `bsp/src/*.c` | `ref/at32/bsp/` | **REF** |
| `SDK/DM9051/include/DM9051.h` | `ref/at32/net/DM9051.h` | **REF** |

### 3.1 Web 衍生物件

以下僅供 build script 參考，不進入 app source 直接 port：

| 來源 | 說明 |
|------|------|
| `SDK/DM9051/App/webserver/makefsdata` | 產生 `httpd-fsdata.c/h` 腳本 |
| `SDK/DM9051/App/webserver/makestrings` | http string 產生腳本 |
| `SDK/DM9051/App/webserver/Makefile.webserver` | webserver gen makefile |

### 3.2 TCP_App 平行目錄

| 來源 | 說明 |
|------|------|
| `SDK/DM9051/TCP_App/webserver/*` | 功能同 App/webserver，本 manifest 以 `App/webserver` 為準 |
| `SDK/DM9051/TCP_App/etherbridge/*` | 功能同 App/etherbridge |
| `SDK/DM9051/TCP_App/AT_Command/*` | 功能同 App/AT_Command |

---

## 4. `compat/uip_mh2203/` 建議內容

> 檔案名稱與模組劃分遵從「不改既有檔名/函式名」原則。

| 檔案 | 目的 |
|------|------|
| `uip_mh2203.h` | 統一 include / platform feature macro |
| `uip_mh2203_tick.c` | `clock_time()` -> new platform timer |
| `uip_mh2203_uart.c` | UART send / recv / FIFO / DMA style -> new UART HAL |
| `uip_mh2203_flash.c` | dataflash write/read API -> new Flash HAL |
| `uip_mh2203_spi.c` | DM9051 SPI read/write / CS / GPIO -> at_port SPI |
| `uip_mh2203_reset.c` | `__asm` reset / IAP reset -> new platform reset |
| `uip_mh2203_endian.c` | `BYTE_ORDER` / packing / align helper（若有差異） |

### 4.1 建議相容 stub 形狀

ทุก compat 檔案**只提供 symbol mapping / wrapper**，不修改原有 app 邏輯。

```c
/* uip_mh2203_tick.c */
#include "clock-arch.h"
clock_time_t clock_time(void) {
    return at_port_get_tick_count();
}

/* uip_mh2203_uart.c */
#include "at_port.h"
void uart_send_byte(uint8_t c) {
    at_port_uart_write(c);
}
```

---

## 5. 保留原處、供新系統加入使用的硬體參考

> 這些檔案**不搬**到 `app/` / `net/uip/`，留在原 `SDK/` 做為新板移植的參考範本。

- `User/system_at32f4xx.c`
- `User/at32f4xx_it.c`
- `User/at32_board_uart.c`
- `User/spi_config.c`
- `Libraries/AT32F4xx_StdPeriph_Driver/src/*.c`
- `Libraries/CMSIS/CM4/DeviceSupport/startup/mdk/startup_at32f413cx_md.s`
- `Libraries/CMSIS/CM4/DeviceSupport/include/at32f413.h` etc.
- `bsp/inc/*.h` + `bsp/src/*.c`
- `SDK/DM9051/DM9051.c` — 驅動邏輯保留，但 SPI/GPIO/EXTI 行號要換成新平台 HAL
- `SDK/DM9051/include/DM9051.h`

---

## 6. 建議的目錄骨架

```
new_project/
├── app/
│   ├── at_command/
│   ├── etherbridge/
│   └── webserver/
│       └── httpd-fs/
├── net/
│   └── uip/
│       ├── core/
│       ├── apps/
│       └── include/
├── port/
│   ├── at_port.h
│   ├── at_port.c
│   ├── clock-arch.c
│   ├── at_net_shim.h
│   └── net_if/
│       ├── tapdev.c
│       └── DM9051.c
└── compat/
    └── uip_mh2203/
        ├── uip_mh2203.h
        ├── uip_mh2203_tick.c
        ├── uip_mh2203_uart.c
        ├── uip_mh2203_flash.c
        ├── uip_mh2203_spi.c
        ├── uip_mh2203_reset.c
        └── uip_mh2203_endian.c
```

---

## 7. 移植文件缺口

| 需求文件 | 目前狀態 | 建議 |
|------|------|------|
| `01-at-command-dependencies.md` | 不存在 (only 00/03/05-ish docs) | 接著產出 |
| `02-at-port-hal-spec.md` | 不存在 | 接著產出 |
| `04-ai-porting-checklist.md` | 不存在 | 接著產出 |
| `05-webserver-porting.md` | 不存在 | 接著產出 |

---

## 8. 快速下一步

1. 建立新專案骨架 `new_project/`（上述 7）。
2. 僅複製 `app/` + `net/uip/`，先編譯過。
3. 實作 `port/at_port.c`、`port/clock-arch.c`、`port/net_if/tapdev.c` stubs。
4. 若 DM9051 保留，先完成 `port/net_if/DM9051.c`。
5. 需要時再補 `compat/uip_mh2203/` wrapper，**不動 app 內部邏輯**。
