# Git Commit Message — 專案規範（嵌入式 C / MCU）

## 標準格式

```
[Component] Subject

Detailed changes:
- Change 1
- Change 2

Overall impact and purpose:
Why this change was made at a higher level.
```

- `[Component]` 與 `Subject` 為**必填**，兩者之間有一個空格。
- `Detailed changes:` 段落為選填，列出逐項變更（bullet points）。
- `Overall impact and purpose:` 為選填，說明改動的總體目的與影響。
- 若改動極小（單一檔案 < 10 行變更、純註解修正、格式調整），可僅寫 Subject。

---

## Component 類型（只能選一個）

| Component  | 適用範圍                                                             |
|------------|----------------------------------------------------------------------|
| `DM9051`   | DM9051 Core Driver 核心邏輯變更（`core/`）                           |
| `HAL`      | HAL Interface 或 vtable 結構變更（`hal/`）                           |
| `PORT`     | 平台移植層變更 — SPI、GPIO、IRQ、DMA、delay（`ports/`）              |
| `uIP`      | uIP 1.0 adapter 變更（`adapters/uip/`）                              |
| `lwIP`     | lwIP 2.1.2 adapter 變更（`adapters/lwip/`）                          |
| `APP`      | 應用層範例變更（`apps/`）                                            |
| `DOC`      | 文件變更（markdown、HTML 文件）                                      |
| `BUILD`    | Keil 專案配置（`.uvprojx`、`.uvoptx`）、建置腳本、編譯定義          |
| `TOOL`     | 開發工具／腳本（PowerShell scripts、驗證工具）                       |
| `LIBS`     | MCU peripheral library 或 CMSIS 變更（`Libraries/`）                 |
| `SKILL`    | Agent skill 或 workflow 檔案變更（`.agents/`、`.claude/`）           |
| `MERGE`    | 合併版控、修正衝突                                                   |

### Component 選擇規則

- **只能選一個**，不可填複數 Component。
- 變更涉及跨層（如同時改 `core/` 與 `hal/`），以主要改動所在層為準。
- 若同時改動 driver + adapter，以底層為主（`DM9051` > `uIP` / `lwIP`）。
- 單純改文件 → `DOC`；單純改 Keil 專案 → `BUILD`。

---

## Subject 標題規則

- 以英文撰寫（與專案既有 commit 風格一致）。
- **不超過 50 字元**，句末不加句號。
- 使用**祈使句**（如 "Fix...", "Add...", "Remove...", "Refactor..."）。
- 描述**實際改動內容**，而非問題現象。

```
# Good
[DM9051] Fix SPI DMA timeout on large transfers
[HAL] Add dm9051_hal_bind() entry point for port registration
[uIP] Refactor to uip_ethernetif struct with link_poll API

# Bad
[DM9051] Fix bug
[DM9051] Driver changes
[DOC] Update
```

---

## Body 本文規則（選填）

- 標題不足以完整描述時再填。格式如下：

```
Detailed changes:
- Add SPI chip select toggle in dm9051_hal_read_reg()
- Fix byte order in multi-byte register access
- Update DM9051A_DEVICE_ID check to include revision mask

Overall impact and purpose:
Fixes incorrect register reads on platforms with
reverse byte ordering. Verified with logic analyzer
on MH2030A + DM9051A hardware.
```

- `Detailed changes:` 使用 `- ` bullet point 逐項列出。
- `Overall impact and purpose:` 說明為何這樣改、影響範圍、驗證方式。
- 每行不超過 72 字元。

### 適合寫 Body 的情況

| 情況 | 範例 |
|------|------|
| SPI timing 調整 | `[PORT] Adjust SPI clock prescaler for 20 MHz operation` |
| Register bit 修正 | `[DM9051] Fix RCR bit 3 mask in promiscuous mode check` |
| 多層連動改動 | `[HAL] Split spi_read into polling and DMA variants` |
| 破壞性 API 變更 | `[uIP] Change dm9051_uip_stack_init() signature` |
| 記憶體使用變更 | `[DM9051] Reduce RX buffer pool from 4 to 2 KB` |

---

## 完整範例

### 範例 1：簡短（單一檔案小改動）

```
[DM9051] Fix RCR promiscuous mode bit mask
```

### 範例 2：一般（含 Body）

```
[uIP] Refactor to uip_ethernetif struct with link_poll API

Detailed changes:
- Introduce struct uip_ethernetif consolidating dev/hal/rx_buf/tx_buf
- Add dm9051_uip_link_poll() for lightweight link status polling
- Refactor dm9051_uip_stack_init() signature to accept uip_ethernetif
- Move dm9051_uip_attach() inside dm9051_uip_stack_init()
- Remove redundant dm9051_uip_mh2030a_smoke_receive/send wrappers
- Simplify main demo: delete link detection boilerplate, use link_poll
- Export dm9051_uip_mh2030a_smoke_eth() accessor

Overall impact and purpose:
Aligns uIP adapter API with lwIP ethernetif pattern, reduces
application boilerplate, and consolidates device state into a
single structure for cleaner porting.
```

### 範例 3：文件變更

```
[DOC] Add lwIP adapter architecture analysis

Detailed changes:
- Document HAL vtable binding mechanism
- Map RX/TX data flow through netif layer
- Describe error handling strategy for each path
- Provide diagrams for packet flow and buffer management
- Summarize key design decisions and trade-offs

Overall impact and purpose:
Provides a comprehensive reference for future DM9051
adapter implementations and porting efforts.
```

### 範例 4：平台移植

```
[PORT] Add MH2030A SPI1 DMA transfer implementation

Detailed changes:
- Implement dm9051_hal_read_mem_dma() using SPI1 DMA channel
- Implement dm9051_hal_write_mem_dma() using SPI1 DMA channel
- Add DMA completion callback with interrupt flag handling
- Configure DMA transfer size to match DM9051 FIFO granularity

Overall impact and purpose:
Enables zero-copy RX/TX for DM9051 on MH2030A,
reducing CPU load during network throughput tests.
```

### 範例 5：建置配置

```
[BUILD] Add DM9051A_SPI_DMA target with DMA preprocessor defines

Detailed changes:
- Clone DM9051A target as DM9051A_SPI_DMA
- Add MH2030A_DM9051_SPI_DMA preprocessor define
- Include dm9051_hal_mh2030a_spi1_dma.c in source group
- Exclude polling-only source files from DMA target build

Overall impact and purpose:
Enables separate build configuration for DMA mode,
avoiding compile-time ifdef branching in shared source files.
```

---

## 撰寫流程

1. **判斷 Component**：依改動所在目錄選擇唯一的 Component（見上表）。
2. **寫 Subject**：`[Component] ` prefix + 50 字內英文祈使句、不加句號。
3. **判斷是否需要 Body**：小改動免 Body；跨層、破壞性、或多項變更請補 `Detailed changes:` 與 `Overall impact and purpose:`。
4. **檢查空行**：Subject 與 Body 之間留一個空行（若 Body 存在）。

---

## 邊界情況

| 情況 | 處理方式 |
|------|----------|
| 只改 Keil `.uvprojx` | Component 使用 `BUILD` |
| 只改 `AGENTS.md` 或 skill 檔案 | Component 使用 `SKILL` |
| 同時改 `core/` + `adapters/` | Component 以底層為準，Body 說明跨層影響 |
| 無法判斷 Component | 使用主目錄層級最高的變更所在層 |
| 破壞性變更 | Body 需說明 migration path 或 backward compat |
