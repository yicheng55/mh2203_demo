# SOUL.md — DM9051 Portable Driver 專案靈魂檔

> 「驅動程式不只是溝通硬體的橋樑，更是系統架構的基石。」—— 專案核心理念

---

## 專案身分

| 屬性 | 說明 |
|------|------|
| **專案名** | DM9051 Portable Driver (Staged Refactor) |
| **目標晶片** | Davicom DM9051A SPI Ethernet 控制器 |
| **目標 MCU** | AT32F403A / AT32F407 (Cortex-M0, MH2030A 開發板) |
| **網路協議棧** | uIP 1.0 (主要) / lwIP (次要支援) |
| **開發環境** | Keil MDK-ARM, Windows + MSYS2 bash |
| **專案階段** | 增量式重構與驗證 (Staged Migration) |

---

## 核心哲學：分層與介面

### 四層架構 (嚴格單向依賴)

```
Application / uIP / lwIP
        │
Network Stack Adapter (adapters/)
        │
DM9051 Core Driver (core/)
        │
DM9051 HAL vtable (hal/inc/)
        │
MCU Port Layer (ports/mh2030a/)
        │
SPI / GPIO / IRQ / Delay
```

**鐵律**：上層只能依賴下層，下層**絕不**包含上層標頭檔。
- `core/` 純淨、無堆疊相依、無平台相依
- `hal/inc/` 只定義 vtable 介面，零實作
- `ports/` 擁有 MCU 具體實作 (SPI、GPIO、EXTI、Delay)
- `adapters/` 擁有堆疊整合邏輯，**禁用**直接 include MCU SPI/GPIO 標頭
- `examples/` 只 include adapter 與 board config，不觸核心內部

### Interface-First (介面先行)

- **HAL Contract** (`docs/plan/HAL_CONTRACT.md`) 定義每個操作的語意、錯誤碼、前置/後置條件
- **API Boundary** (`docs/plan/API_BOUNDARY.md`) 明確列出各層可/不可 include 的標頭
- **State Model** (`docs/plan/STATE_MODEL.md`) 定義 `dm9051_device_t` 運行時狀態機

---

## 關鍵設計決策

### 1. Context-based API (核心現代化)

舊版：全域狀態、單一實例、硬編碼 MH2030A
新版：`dm9051_device_t` 實例化、多實例支援、平台中立

```c
// 單一實例，可攜帶到任何平台
dm9051_device_t dev;
dm9051_config_t cfg;
dm9051_hal_t hal;          // vtable + ctx
dm9051_core_open(&dev, &cfg, &hal);
```

### 2. HAL vtable 而非平頭函式

```c
typedef struct dm9051_hal_ops {
    int (*read_reg)(void *ctx, uint8_t reg, uint8_t *val);
    int (*write_reg)(void *ctx, uint8_t reg, uint8_t val);
    int (*read_mem)(void *ctx, uint8_t *buf, uint16_t len);   // MRCMD
    int (*write_mem)(void *ctx, const uint8_t *buf, uint16_t len); // MWCMD
    void (*reset)(void *ctx);
    void (*delay_ms)(uint32_t ms);
    void (*delay_us)(uint32_t us);
    void (*irq_enable)(void *ctx);
    void (*irq_disable)(void *ctx);
    uint32_t (*enter_critical)(void *ctx);
    void (*exit_critical)(void *ctx, uint32_t state);
} dm9051_hal_ops_t;
```

- **強制綁定集**：`read_reg`、`write_reg`、`read_mem`、`write_mem`、`delay_ms`、`delay_us`
- **選配**：`reset`、IRQ hooks、critical section hooks
- 錯誤碼語意化：`OK` / `ERR` / `ERR_TIMEOUT` / `ERR_PARAM` / `ERR_NOT_READY`

### 3. Staged Migration (增量遷移)

**不大變更重寫**，而是並存：
- Legacy API (`dm9051_conf()`、`dm9051_init()`、`dm9051_rx()`、`dm9051_tx()`) 保留於 `dm9051_core.h`
- 新 API (`dm9051_core_open()`、`dm9051_core_receive_ex()`...) 並行
- Keil 生產專案 (`DM9051A.uvprojx`) **完全不動**
- Staged 驗證專案 (`DM9051A_uip.uvprojx`) 獨立引用 staged 檔案
- 只有通過驗證且完成 GitNexus 影響分析，才切換目標

### 4. 雙堆疊適配器策略

| 適配器 | 設計重點 |
|--------|----------|
| **uIP** (`adapters/uip/`) | 極簡、單緩衝 `uip_buf`、ARP 整合、週期性輪詢 `dm9051_uip_stack_poll()` |
| **lwIP** (`adapters/lwip/`) | `struct netif` 標準介面、pbuf chain → 連續 `tx_buf` 複製、PBUF_POOL RX、`ethernet_input` 回呼 |

兩者**共用同一組 core + HAL + port**，唯一差異在 adapter 層。

### 5. MH2030A Port 特色

- **SPI1 Polling** (預設、最穩) — `dm9051_hal_mh2030a_spi1.c`
- **SPI1 DMA FIFO** (選配) — `dm9051_hal_mh2030a_spi1_dma.c`，暫時 `ERR_NOT_READY`
- **PF6/EXTI Line 6 IRQ** (選配) — `dm9051_hal_mh2030a_int.c`，事件推回 core
- **Build-time feature flags** (`dm9051_hal_mh2030a_spi1.h`)：
  - `DM9051_MH2030A_USE_DMA` / `ENABLE_DMA`
  - `DM9051_MH2030A_USE_IRQ` / `ENABLE_IRQ`
  - `DM9051_MH2030A_OWN_EXTI4_15_HANDLER` (應用層自管 IRQ Handler)

---

## 專案靈魂：為什麼這樣做？

### 1. 可移植性 > 便利性

嵌入式驅動最怕「寫死在某塊板子上」。這專案從 Day 1 就強迫分離：
- 核心不認得 AT32、不認得 Keil、不認得 uIP/lwIP
- 換 MCU 只寫 `ports/<new_mcu>/`，核心完全不動
- 換堆疊只寫 `adapters/<new_stack>/`，核心完全不動

### 2. 可驗證性 > 假設正確

- 每層都有 **介面文件** (HAL_CONTRACT, API_BOUNDARY, STATE_MODEL)
- Staged 專案可獨立編譯、獨立燒錄、獨立測試
- Smoke test (`main_uip_mh2030a_smoke.c`) 只做：開機 → Chip ID → Link → 收發一包
- 複雜功能 (DMA、IRQ、lwIP) 逐層啟用，各自有獨立驗證入口

### 3. 相容性 > 完美主義

- Legacy API 保留是為了**不打斷量產專案**
- 符號重命名需經 GitNexus 影響分析
- 文件先行：`docs/plan/` 先寫清楚再動碼

### 4. 顯式 > 隱式

- 所有狀態在 `dm9051_device_t.runtime` 可見
- 錯誤碼回傳，不靠全域 `errno`
- Feature flag 全在標頭檔可見，不藏在 `.c` 裡
- Build config 以 `#define` 控制，無魔法連結時期切換

---

## 關鍵檔案地圖 (大腦索引)

| 類別 | 檔案 | 角色 |
|------|------|------|
| **核心介面** | `core/inc/dm9051_core.h` | 新舊 API 完整宣告 |
| **核心型別** | `core/inc/dm9051_types.h` | `dm9051_device_t`、config、error codes |
| **核心暫存器** | `core/inc/dm9051_regs.h` | DM9051A 所有暫存器定義 |
| **核心實作** | `core/src/dm9051_core.c` | 開關機、收發、PHY、中斷狀態 |
| **HAL 介面** | `hal/inc/dm9051_hal.h` | vtable 定義、錯誤碼 |
| **MH2030A Polling** | `ports/mh2030a/dm9051_hal_mh2030a_spi1.c/h` | SPI1 polling 實作 |
| **MH2030A DMA** | `ports/mh2030a/dm9051_hal_mh2030a_spi1_dma.c/h` | DMA FIFO (進行中) |
| **MH2030A IRQ** | `ports/mh2030a/dm9051_hal_mh2030a_int.c/h` | PF6/EXTI6 事件轉發 |
| **uIP Adapter** | `adapters/uip/dm9051_uip.c` | 核心→uIP 橋接 |
| **uIP Stack Loop** | `adapters/uip/dm9051_uip_stack.c` | 週期性輪詢、ARP、TCP/UDP timer |
| **lwIP Adapter** | `adapters/lwip/dm9051_lwip.c` | `netif` 標準實作、pbuf 處理 |
| **Smoke Test** | `examples/uip_mh2030a_demo/main_uip_mh2030a_smoke.c` | 最小驗證入口 |
| **移植指南** | `docs/PORTING_GUIDE.md` | 新 MCU 移植 6 步驟 |
| **規劃文檔** | `docs/plan/*.md` | 架構決策不可變記錄 |

---

## 開發工作流 (Soul-level)

```
1. 閱讀介面 → 2. 寫測試/驗證入口 → 3. 實作最小可行 → 4. 在 staged 專案跑通
                                                    ↓
5. 文件同步更新 (HAL_CONTRACT / STATE_MODEL / API_BOUNDARY)
                                                    ↓
6. 只有通過所有 smoke + integration test → 才考慮切換生產目標
```

**禁止**：
- 直接修改 `drivers/dm9051_edriver_v1.6.1a_beta/` (生產基線)
- 在 core/hal 裡 include uIP/lwIP/MH2030A 標頭
- 無文件就改 API 簽名
- 魔法 `#ifdef` 散落在各層 (集中在 port layer 的 feature header)

---

## 給未來維護者的話

> 這專案不是為了「重構而重構」。它是為了**下一顆 MCU、下一個堆疊、下一個專案**不再從頭寫起。
>
> 當你在 `ports/` 裡加新 MCU 時，如果發現 core 需要改 — 代表介面不夠完整，請先補介面。
> 當你在 `adapters/` 裡加新堆疊時，如果發現 core 需要改 — 代表核心職責外洩，請修正分層。
>
> **分層是自由的代價，介面是自由的保證。**

---

## 版本歷程 (關鍵里程碑)

| Commit | 日期 | 里程碑 |
|--------|------|--------|
| `314ddc2` | 2026-06-17 | 嵌入式系統工程師角色與指南文件化 |
| `7dc4010` | 2026-06-17 | DM9051 uIP Adapter 分析 + SVG 分層圖 |
| `e5b31a1` | 2026-06-17 | 程式碼結構重組、可讀性與可維護性提升 |

---

*SOUL.md 應隨架構決策演進而更新。若專案靈魂變了，這檔案才需大改寫。*