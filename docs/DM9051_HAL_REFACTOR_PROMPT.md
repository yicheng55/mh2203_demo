# DM9051 HAL Refactor Architecture Review Prompt

請分析以下 3 個目錄，目標是評估並設計可重用的 DM9051 驅動架構：

1. `D:\prg\davicom\mh2xxxx_proj\mh2030_demo\drivers`
2. `D:\prg\davicom\mh2xxxx_proj\mh2030_demo\ModuleDemo\DM9051A\port`
3. `D:\prg\davicom\mh2xxxx_proj\mh2030_demo\ModuleDemo\DM9051A\USER`

請先不要修改程式碼，先完成架構分析、code review 與重構建議。

## Review Goal

目前希望優化目錄與檔案位置，建立可共用的 HAL 驅動層，讓底層 DM9051 驅動可以同時被目前的 uIP 上層與未來加入的 lwIP 上層共用。

目標是形成以下分層：

```text
USER / uIP / lwIP
        |
network stack adapter
        |
DM9051 core driver
        |
DM9051 HAL interface
        |
MH2030A SPI / GPIO / IRQ / delay
```

## Target Architecture

### DM9051 Core Driver

- 放置與 DM9051 晶片暫存器、封包收發、PHY/MAC 控制相關的共用邏輯。
- 不直接依賴 uIP。
- 不直接依賴 lwIP。
- 不直接依賴 Keil project。
- 不直接依賴 MH2030A 平台細節。
- 不直接操作 MCU SPI/GPIO/IRQ/delay API。

### HAL / Bus Driver Layer

- 抽象 SPI read/write。
- 抽象 GPIO reset。
- 抽象 delay。
- 抽象 interrupt enable/disable。
- 抽象 critical section。
- 平台相關程式集中放在 HAL 層。
- 讓 DM9051 core driver 可在不同 MCU、不同 RTOS 或裸機環境共用。

### Network Stack Adapter

- uIP adapter：負責把 DM9051 rx/tx API 接到 uIP。
- lwIP adapter：未來可新增，使用同一套 DM9051 core driver 與 HAL interface。
- 上層 network stack 不直接操作 DM9051 register。
- 上層 network stack 不直接操作 SPI。

## Analysis Tasks

請分析目前程式碼中：

1. 哪些檔案屬於 DM9051 core driver。
2. 哪些檔案屬於 MH2030A 平台 HAL。
3. 哪些檔案屬於 uIP adapter。
4. 哪些檔案屬於 USER application / demo layer。
5. 哪些程式目前耦合過深，會阻礙未來加入 lwIP。
6. 哪些 API 應該抽象出來。
7. 哪些 include path、Keil project 設定或編譯單元可能需要調整。
8. 哪些初始化順序與中斷流程需要保留或重新整理。

## Suggested HAL Interface Candidates

請評估是否需要抽象出以下 API，並根據目前程式碼提出更合適的命名與參數：

```c
int dm9051_hal_spi_read(void *ctx, uint8_t reg, uint8_t *buf, uint16_t len);
int dm9051_hal_spi_write(void *ctx, uint8_t reg, const uint8_t *buf, uint16_t len);
int dm9051_hal_spi_read_mem(void *ctx, uint8_t *buf, uint16_t len);
int dm9051_hal_spi_write_mem(void *ctx, const uint8_t *buf, uint16_t len);

void dm9051_hal_reset(void *ctx);
void dm9051_hal_delay_ms(uint32_t ms);
void dm9051_hal_delay_us(uint32_t us);

void dm9051_hal_irq_enable(void *ctx);
void dm9051_hal_irq_disable(void *ctx);

void dm9051_hal_enter_critical(void *ctx);
void dm9051_hal_exit_critical(void *ctx);
```

也請評估 DM9051 core driver 對上層 network adapter 是否需要類似以下 API：

```c
int dm9051_open(struct dm9051_device *dev);
int dm9051_close(struct dm9051_device *dev);
int dm9051_poll(struct dm9051_device *dev);
int dm9051_rx(struct dm9051_device *dev, uint8_t *buf, uint16_t buf_len, uint16_t *rx_len);
int dm9051_tx(struct dm9051_device *dev, const uint8_t *buf, uint16_t len);
int dm9051_get_link(struct dm9051_device *dev);
```

## Suggested Directory Structure

請評估是否適合重構成以下目錄結構，並指出需要調整的地方：

```text
drivers/
  dm9051/
    core/
      dm9051.c
      dm9051.h
      dm9051_regs.h
      dm9051_phy.c
      dm9051_phy.h
    include/
      dm9051_hal.h
      dm9051_netif.h

ModuleDemo/DM9051A/port/
  mh2030a/
    dm9051_hal_mh2030a_spi1.c
    dm9051_hal_mh2030a_spi1.h
  uip/
    dm9051_uip_adapter.c
    dm9051_uip_adapter.h
  lwip/
    dm9051_lwip_adapter.c
    dm9051_lwip_adapter.h
```

## Code Review Focus

請優先檢查：

- 可能的 bug。
- 初始化順序問題。
- SPI / DM9051 / uIP / 中斷 / DMA 相關風險。
- buffer overflow。
- 越界存取。
- NULL pointer。
- race condition。
- ISR 與主迴圈共享資料問題。
- critical section 是否足夠。
- magic number。
- 重複程式碼。
- 可維護性問題。
- compiler warning 風險。
- Keil / embedded C 常見移植問題。
- 未來接入 lwIP 時可能造成阻礙的耦合點。

## Required Output

請用 code review + architecture review 的格式回答。

輸出必須包含：

1. 目前架構摘要。
2. `drivers`、`port`、`USER` 三個目錄各自的角色。
3. 主要呼叫關係與資料流。
4. 建議的新目錄結構。
5. 每個建議目錄/檔案的責任。
6. 目前檔案搬移或拆分建議。
7. 建議的 HAL interface header。
8. uIP adapter 與未來 lwIP adapter 的分層方式。
9. Findings first 的 code review 問題清單，依嚴重程度排序。
10. 每個 finding 必須包含：
    - 嚴重度：Critical / High / Medium / Low
    - 檔案與行號
    - 問題說明
    - 可能造成的後果
    - 建議修正方式
11. 重構風險清單。
12. 建議分階段修改計畫。
13. 哪些項目可以低風險先做。
14. 哪些項目需要先做 impact analysis 或人工確認。

## GitNexus Rules

此專案已由 GitNexus 索引。請遵守以下規則：

- 修改任何 function/class/method 前，必須先執行 impact analysis。
- impact analysis 請使用 upstream 方向，確認直接呼叫者、受影響流程與風險等級。
- 若 impact analysis 顯示 HIGH 或 CRITICAL risk，必須先警告使用者並等待確認。
- 不要用單純 find-and-replace 重新命名 symbol；需要使用理解 call graph 的 rename 流程。
- 修改完成後，必須執行 detect_changes，確認影響範圍只包含預期的 symbols 與 execution flows。
- 若只是做分析與 review，不要修改檔案。

## Important Constraint

第一階段請只做「架構分析、code review 與分階段重構計畫」，不要直接搬檔、改 API 或修改 Keil project。

等使用者確認第一階段方案後，再開始執行實際重構。
