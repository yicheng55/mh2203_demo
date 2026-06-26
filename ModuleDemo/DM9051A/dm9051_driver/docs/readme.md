# DM9051A 驅動程式使用手冊 (ModuleDemo)

## 1. 簡介 (Introduction)

本文件說明 `ModuleDemo/DM9051A/dm9051_driver` 目錄下的 DM9051A SPI 乙太網路驅動程式結構。此驅動以 mh2030 / MH2030A MCU 為主要平台，並以分層方式整理 DM9051A 晶片核心邏輯、硬體抽象層、平台移植層與 TCP/IP stack 適配層。

目前此目錄屬於 staged driver layout，目標是讓 DM9051A driver 從原本較偏平台綁定的結構，逐步整理為可移植、可維護、可擴充的驅動架構。

**相關文件：**

| 文件 | 說明 |
|------|------|
| `docs/plan/HAL_CONTRACT.md` | HAL 操作完整語意合約 |
| `docs/plan/API_BOUNDARY.md` | 分層依賴規則 |
| `docs/plan/STATE_MODEL.md` | 狀態模型 (`dm9051_device_t`) |
| `docs/plan/CORE_API_PLAN.md` | Core API 設計與 legacy 相容 |
| `docs/plan/BUILD_SELECTION.md` | Keil target 編譯選擇分析 |
| `docs/plan/CORE_SPLIT_MAP.md` | 函式拆分對應表 |
| `docs/plan/ADAPTER_STAGING.md` | 轉接器暫存行為 |
| `docs/PORTING_GUIDE.md` | 移植到新 MCU 平台的逐步指南 |

## 2. 檔案結構說明 (Directory Structure)

整體分層如下：

```text
Application / uIP / lwIP
        |
Network Stack Adapter
        |
DM9051 Core Driver
        |
DM9051 HAL Interface
        |
MH2030A SPI / GPIO / IRQ / Delay
```

### 2.1 主要檔案結構

| 檔案 / 路徑 | 核心功能 | 在驅動中的角色 |
|---|---|---|
| `core/inc/dm9051_core.h` | 宣告 DM9051 核心 API，例如初始化、收包、送包、PHY 讀寫、中斷事件處理等。 | 核心邏輯層對外介面 |
| `core/src/dm9051_core.c` | 實作 DM9051 核心流程，包含裝置探測、MAC 設定、RX/TX FIFO 操作、PHY 存取、IRQ 狀態管理。 | 核心邏輯層 |
| `core/inc/dm9051_regs.h` | 定義 DM9051 暫存器位址、bit mask、Vendor/Product ID、RX/TX 控制常數。 | 晶片暫存器定義層 |
| `core/inc/dm9051_types.h` | 定義 driver 共用型別，例如 `dm9051_config_t`、`dm9051_device_t`、狀態碼、MAC/IP 設定。 | 共用資料型別層 |
| `core/src/dm9051_debug.c` | 保留診斷、debug 輸出與輔助檢查邏輯。 | 診斷輔助層 |
| `hal/inc/dm9051_hal.h` | 定義平台無關 HAL vtable，例如 `read_reg`、`write_reg`、`read_mem`、`write_mem`、delay、IRQ 控制。 | 硬體抽象介面層 |
| `ports/mh2030a/dm9051_hal_mh2030a_spi1.c` | 將 MH2030A 的 SPI1 polling 傳輸、CS 控制、reset GPIO、delay 綁定到 DM9051 HAL。 | MH2030A 平台移植層 |
| `ports/mh2030a/dm9051_hal_mh2030a_spi1.h` | 宣告 MH2030A SPI1 HAL binding API 與設定結構。 | 平台移植層對外介面 |
| `ports/mh2030a/dm9051_hal_mh2030a_spi1_priv.h` | 放置 SPI1 與 pin mapping 的私有共用定義。 | 平台私有定義 |
| `ports/mh2030a/dm9051_hal_mh2030a_spi1_dma.c` | 提供 SPI1 DMA 版本 FIFO 傳輸實作。 | 平台 DMA 傳輸層 |
| `ports/mh2030a/dm9051_hal_mh2030a_spi1_dma.h` | 宣告 SPI1 DMA 相關介面。 | 平台 DMA 介面 |
| `ports/mh2030a/dm9051_hal_mh2030a_int.c` | 實作 DM9051 INT 腳位對應的 EXTI/NVIC 中斷設定與 handler 轉接。 | 平台中斷處理層 |
| `ports/mh2030a/dm9051_hal_mh2030a_int.h` | 宣告 MH2030A 中斷初始化與 handler API。 | 平台中斷介面 |
| `ports/mh2030a/mh2030a_platform.h` | 集中包含 MH2030A 平台相關 header，例如 `mh20xx.h`、delay 等。 | 平台相容封裝層 |
| `ports/mh2030a/mh2030a_board.c` | 實作 board bring-up，例如 clock、debug UART、printf retarget。 | 板級支援層 |
| `ports/mh2030a/mh2030a_board.h` | 宣告 board 初始化相關 API。 | 板級支援介面 |
| `ports/mh2030a/delay.c` | 提供 MH2030A 使用的 delay 實作。 | 時序支援層 |
| `ports/mh2030a/delay.h` | 宣告 delay API。 | 時序支援介面 |
| `ports/mh2030a/mh2030a_uip_clock.c` | 提供 uIP 使用的系統 clock/timer 支援。 | uIP 平台時間層 |
| `ports/mh2030a/mh2030a_uip_clock.h` | 宣告 uIP clock 相關 API。 | uIP 平台時間介面 |
| `ports/mh2030a/mh20xx_it.c` | 放置 MH2030A 中斷向量或中斷服務函式整合。 | MCU 中斷整合層 |
| `adapters/lwip/lwipopts.h` | lwIP 編譯與功能設定。 | lwIP 組態層 |
| `adapters/uip/dm9051_uip.c` | 將 DM9051 core API 包裝成 uIP 可使用的輸入、輸出、poll 與 interrupt flow。 | uIP 適配層 |
| `adapters/uip/dm9051_uip.h` | 宣告 uIP adapter API，例如 `dm9051_uip_init()`、`dm9051_uip_poll()`。 | uIP 適配介面 |
| `adapters/uip/dm9051_uip_stack.c` | 整合 uIP stack 初始化與主迴圈 poll 流程。 | uIP stack 整合層 |
| `adapters/uip/dm9051_uip_stack.h` | 宣告 uIP stack 整合 API。 | uIP stack 介面 |
| `examples/lwip_mh2030a_demo/main_dm9051_lwip_example.c` | 示範 MH2030A + DM9051 + lwIP 的初始化與執行方式。 | lwIP 範例應用 |
| `examples/uip_mh2030a_demo/main_uip_mh2030a_demo.c` | 示範 MH2030A + DM9051 + uIP 的主要 demo 流程。 | uIP 範例應用 |
| `examples/uip_mh2030a_demo/main_uip_mh2030a_smoke.c` | 用於基本 smoke test，驗證 staged driver 可完成初始化與 chip ID 探測。 | 驅動驗證範例 |
| `examples/uip_mh2030a_demo/dm9051_uip_mh2030a_smoke.c` | 綁定 MH2030A HAL/device pair，執行 DM9051 core open 測試。 | uIP smoke glue |
| `examples/uip_mh2030a_demo/dm9051_uip_mh2030a_smoke.h` | 宣告 smoke test 支援 API。 | 測試支援介面 |
| `Makefile` | 提供 staged driver 的建置規則或檔案選擇參考。 | 建置輔助 |
| `README.md` | 說明目前 staged driver 的分層、來源對應與重構狀態。 | 專案說明文件 |

### 2.2 與傳統三檔式驅動的對應

| 傳統檔案概念 | 目前分層後的對應檔案 | 說明 |
|---|---|---|
| `dm9051_driver.c` | `core/src/dm9051_core.c` | 原本集中式的初始化、收包、送包、PHY 與中斷狀態邏輯，已規劃集中在 core driver。 |
| `dm9051_driver.h` | `core/inc/dm9051_core.h`、`core/inc/dm9051_regs.h`、`core/inc/dm9051_types.h` | API 宣告、暫存器定義與共用型別被拆分，降低 header 耦合。 |
| `dm9051_env.c` | `hal/inc/dm9051_hal.h`、`ports/mh2030a/*` | SPI、GPIO、IRQ、delay 等硬體相關操作改由 HAL contract 與 MH2030A port 實作。 |

## 3. 硬體接線與配置 (Hardware Configuration)

目前 MH2030A port 的 DM9051A 腳位配置如下：

| Signal | Pin | 說明 |
|---|---|---|
| CS | PA15 | SPI chip select |
| SCK | PB3 | SPI clock |
| MISO | PB4 | SPI master input |
| MOSI | PB5 | SPI master output |
| INT | PF6 / EXTI line 6 | DM9051A 中斷輸入腳 |
| RST | PF7 | DM9051A reset 控制腳 |

MH2030A 平台相關 SPI、GPIO、EXTI、delay 實作集中在 `ports/mh2030a/`，核心層不直接包含 MCU vendor header。

### 3.1 Keil `.uvprojx` 專案檔與 Build Target

本專案使用 Keil MDK v5 (ARMCC5, V5.06 update 7) 開發，專案檔位於：
`ModuleDemo/DM9051A/USER/DM9051A.uvprojx`

#### 5 個 Build Target 總覽

| TargetName | 輸出目錄 | 輸出檔名 | C 編譯器巨集 | 功能 |
|-----------|----------|---------|-------------|------|
| `DM9051A` | `..\OBJ\` | `DM9051A` | `USE_STDPERIPH_DRIVER` | 基礎 SPI Probe，驗證 VID/PID/CHIPR |
| `DM9051A_SPI_DMA` | `..\OBJ\` | `DM9051A_DMA` | `USE_STDPERIPH_DRIVER` | SPI DMA Probe |
| `MH2030A_DM9051_uIP` | `..\OBJ_UIP\` | `MH2030A_DM9051_uIP` | `USE_STDPERIPH_DRIVER`, `MH2030A_UIP_PORT` | uIP Polling 網路棧 + Web Server |
| `MH2030A_DM9051_uIP_dma` | `..\OBJ_UIP\` | `MH2030A_DM9051_uIP_dma` | `USE_STDPERIPH_DRIVER`, `MH2030A_UIP_PORT`, `MH2030A_DM9051_SPI_DMA` | uIP + SPI DMA |
| `MH2030A_DM9051_uIP_int` | `..\OBJ_UIP_INT\` | `MH2030A_DM9051_uIP_int` | `USE_STDPERIPH_DRIVER`, `MH2030A_UIP_PORT`, `DMPLUG_INT` | uIP + PF6/EXTI6 Interrupt |

#### `.uvprojx` XML 結構要點

專案檔為 XML 格式，主要區段：

```xml
<Project>
  <Targets>
    <Target>
      <TargetName>DM9051A</TargetName>
      <TargetOption>
        <TargetCommonOption>            <!-- 輸出目錄、Device、CPU、記憶體 -->
          <OutputDirectory>..\OBJ\</OutputDirectory>
          <OutputName>DM9051A</OutputName>
          <Cpu>IROM(0x0,0x40000) IRAM(0x20000000,0x20000)</Cpu>
        </TargetCommonOption>
        <TargetArmAds>
          <Cads>                        <!-- C 編譯器設定 -->
            <VariousControls>
              <Define>USE_STDPERIPH_DRIVER</Define>
              <IncludePath>..\bsp;..\USER;..\..\Libraries\MH20xxLib\inc;...</IncludePath>
            </VariousControls>
          </Cads>
        </TargetArmAds>
      </TargetOption>
      <Groups>
        <Group>
          <GroupName>BSP</GroupName>
          <Files>
            <File>
              <FileName>mh2030a_spi1.c</FileName>
              <FileType>1</FileType>
              <FilePath>..\bsp\mh2030a_spi1.c</FilePath>
            </File>
          </Files>
        </Group>
      </Groups>
    </Target>
  </Targets>
</Project>
```

#### 群組與檔案選擇機制 (IncludeInBuild)

Target 間的功能切換透過三層分流：

1. **C 編譯器巨集 (Define)**：`MH2030A_UIP_PORT` 切換 `main.c` vs `main_uip_mh2030a.c`；`MH2030A_DM9051_SPI_DMA` 啟用 DMA 路徑；`DMPLUG_INT` 啟用中斷路徑。

2. **Group 層級 IncludeInBuild**：uIP 相關群組 (`MH2030A_PORT`、`DM9051_CORE`、`uIP_CORE`、`uIP_APP`、`WEB_APP`) 在基礎 Target 中設為 `0` 排除。

3. **File 層級 IncludeInBuild**：特定檔案在各 Target 中獨立設定是否參與編譯。

| 群組 | 檔案 | DM9051A (基礎) | uIP | uIP DMA | uIP INT |
|------|------|:--------------:|:---:|:-------:|:-------:|
| `USER` | `main.c` | 1 | 0 | 0 | 0 |
| `USER` | `main_uip_mh2030a.c` | 0 | 1 | 1 | 1 |
| `MH2030A_PORT` | `mh2030a_dm9051_spi.c` | 群組排除 | 1 | 0 | 1 |
| `MH2030A_PORT` | `mh2030a_dm9051_spi_dma.c` | 群組排除 | 0 | 1 | 0 |
| `MH2030A_PORT` | `mh2030a_dm9051_int.c` | 群組排除 | - | - | 啟用 |

> **注意**：所有 Target 共用同一份 `<Groups>` 結構，檔案排除是透過各 Target 各自維護的 `<FileOption><IncludeInBuild>0</IncludeInBuild></FileOption>` 達成。

## 4. 驅動初始化與使用說明 (Getting Started)

### 4.1 HAL 介面重點

`hal/inc/dm9051_hal.h` 是 DM9051A driver 可移植性的核心。它不直接依賴 MH2030A，也不包含 SPI/GPIO 的具體實作，而是定義一組平台必須提供的操作函式。

| HAL 成員 | 功能 | 由誰實作 | 說明 |
|---|---|---|---|
| `read_reg` | 讀取 DM9051 暫存器 | `ports/mh2030a/*` | 透過 SPI 送出 register read command，讀回單一暫存器值。 |
| `write_reg` | 寫入 DM9051 暫存器 | `ports/mh2030a/*` | 透過 SPI 寫入單一 DM9051 暫存器。 |
| `read_mem` | 從 RX FIFO 讀取資料 | `ports/mh2030a/*` | 通常對應 DM9051 `MRCMD` 流程，用於接收 Ethernet frame。 |
| `write_mem` | 寫入 TX FIFO | `ports/mh2030a/*` | 通常對應 DM9051 `MWCMD` 流程，用於送出 Ethernet frame。 |
| `reset` | 控制硬體 reset | `ports/mh2030a/*` | 可透過 GPIO 控制 DM9051 reset pin。 |
| `delay_ms` | 毫秒延遲 | `ports/mh2030a/delay.*` | 用於 reset、PHY 操作或初始化等待。 |
| `delay_us` | 微秒延遲 | `ports/mh2030a/delay.*` | 用於 SPI/PHY busy wait 等短延遲。 |
| `irq_enable` | 啟用 MCU 端 DM9051 中斷 | `ports/mh2030a/dm9051_hal_mh2030a_int.*` | 控制 EXTI/NVIC，而不是直接修改 DM9051 IMR。 |
| `irq_disable` | 停用 MCU 端 DM9051 中斷 | `ports/mh2030a/dm9051_hal_mh2030a_int.*` | 避免中斷重入或進入臨界區時使用。 |
| `enter_critical` | 進入臨界區 | 平台層，可用 CMSIS PRIMASK | 保護 shared state，例如 interrupt flag。 |
| `exit_critical` | 離開臨界區 | 平台層，可用 CMSIS PRIMASK | 使用 `enter_critical` 回傳的 state 還原中斷狀態。 |

### 4.2 初始化流程

```text
Application
  -> dm9051_uip_init() / dm9051_if_init()
  -> dm9051_core_open()
  -> HAL reset/read_reg/write_reg
  -> DM9051 chip ID probe
  -> MAC address 設定
  -> RX/IRQ 啟用
```

| 階段 | 主要函式 | 責任 |
|---|---|---|
| 應用層啟動 | `main_*_demo.c` | 建立網路設定與啟動 stack。 |
| Adapter 初始化 | `dm9051_uip_init()` / `dm9051_if_init()` | 將 uIP/lwIP 需求轉換成 DM9051 core 呼叫。 |
| Core 開啟裝置 | `dm9051_core_open()` | 驗證 config、綁定 HAL、探測 chip ID。 |
| 硬體存取 | `read_reg` / `write_reg` | 透過 MH2030A SPI 實際存取 DM9051。 |
| 啟動 RX | core 內部初始化流程 | 設定 MAC、multicast filter、IMR、RCR 等暫存器。 |

#### 最低驗證程式碼 (Smoke Test)

```c
#include "dm9051_core.h"
#include "dm9051_hal_mh2030a_spi1.h"

static dm9051_device_t dev;
static dm9051_hal_t hal;
static const uint8_t my_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

void main(void)
{
    dm9051_config_t core_config;
    dm9051_mh2030a_config_t port_config;

    mh2030a_uip_board_init(115200);               // 初始化時脈、UART、delay
    dm9051_core_default_config(&core_config);      // 填預設 core config
    core_config.mac_addr = my_mac;
    dm9051_mh2030a_default_config(&port_config);   // 填 port config
    dm9051_mh2030a_hal_bind(&hal, &port_config);   // 綁定 HAL vtable

    int status = dm9051_core_open(&dev, &core_config, &hal);

    if (status == DM9051_OK) {
        printf("VID=0x%04X PID=0x%04X CHIPR=0x%02X\n",
               dm9051_core_vendor_id(&dev),
               dm9051_core_product_id(&dev),
               dm9051_core_chip_revision(&dev));
    }
    while (1);
}
```

### 4.3 接收封包流程

```text
DM9051 RX FIFO
  -> HAL read_reg / read_mem
  -> dm9051_core_receive()
  -> adapter input
  -> uIP / lwIP
```

| 階段 | 主要函式 | 責任 |
|---|---|---|
| 檢查封包 | `dm9051_core_receive_ex()` | 讀取 RX ready byte，判斷是否有封包。 |
| 讀取 RX header | core RX helper | 解析封包狀態與長度。 |
| 讀取 payload | `read_mem` | 從 DM9051 RX FIFO 讀出 Ethernet frame。 |
| 交給協定棧 | `dm9051_uip_input()` / `ethernetif_input()` | 將 frame 傳入 uIP 或 lwIP。 |

### 4.4 傳送封包流程

```text
uIP / lwIP
  -> adapter output
  -> dm9051_core_send()
  -> HAL write_mem / write_reg
  -> DM9051 TX FIFO
```

| 階段 | 主要函式 | 責任 |
|---|---|---|
| 協定棧送包 | uIP/lwIP output callback | 產生 Ethernet frame。 |
| Adapter 轉接 | `dm9051_uip_output()` / lwIP `linkoutput` | 呼叫 DM9051 core 傳送 API。 |
| 寫入 TX FIFO | `dm9051_core_send()` | 設定 TX 長度、寫入 payload、觸發 TX request。 |
| 實際 SPI 傳輸 | `write_mem` / `write_reg` | 由 MH2030A port 完成 SPI 寫入。 |

## 5. 常見問題與排錯 (Troubleshooting)

| 問題現象 | 優先檢查檔案 | 檢查重點 |
|---|---|---|
| 無法讀到 DM9051 chip ID | `ports/mh2030a/dm9051_hal_mh2030a_spi1.c`、`core/src/dm9051_core.c` | SPI mode、CS 腳位、reset timing、`VIDL/VIDH/PIDL/PIDH` 讀值。 |
| 初始化成功但收不到封包 | `core/src/dm9051_core.c`、`adapters/uip/*` 或 `adapters/lwip/*` | `RCR`、`IMR`、RX ready byte、adapter poll 是否被呼叫。 |
| 可以收包但不能送包 | `core/src/dm9051_core.c` | TX length register、`MWCMD` FIFO write、`TCR_TXREQ` 是否正確觸發。 |
| 中斷模式無反應 | `ports/mh2030a/dm9051_hal_mh2030a_int.c` | EXTI line、NVIC enable、INT pin polarity、core interrupt flag。 |
| DMA 模式異常 | `ports/mh2030a/dm9051_hal_mh2030a_spi1_dma.c` | DMA channel、transfer length、FIFO command sequence、cache/資料對齊問題。 |
| lwIP 沒有收到封包 | `middlewares/3rd_party/lwip-2.1.2/port/ethernetif.c` | `netif->input`、`pbuf_alloc`、`linkoutput`、`NO_SYS` poll loop。 |
| SPI 讀取 VID 總是 0xFFFF | SPI 初始化與接線 | CS 極性 (DM9051 CS low active)、SPI 模式 (CPOL=0, CPHA=0)、時脈頻率 (勿超過 25MHz) |
| `core_open()` 回傳 ERR_PARAM | HAL vtable 與 config | 檢查 HAL 必選函式是否都已填入，`interrupt_mode` 是否合法 |

## 6. 維護與擴充建議

### 6.1 新增其他 MCU 平台

若未來要將 DM9051A driver 移植到其他 MCU (如 STM32、HC32 等)，建議不要修改 `core/` 目錄，而是新增新的 `ports/` 子目錄。詳細移植步驟請參閱 `docs/PORTING_GUIDE.md`。

```text
ports/
  mh2030a/          現有 MH2030A 平台
  stm32/            新增 STM32 平台
  hc32/             新增 HC32 平台
```

| 要新增的內容 | 說明 |
|---|---|
| 新平台 HAL binding | 實作 `dm9051_hal_ops_t` 中的 SPI、GPIO、delay、IRQ 操作 |
| 新平台 board init | 初始化 clock、GPIO mux、SPI peripheral、debug UART |
| 平台專用 header | 集中包含 MCU vendor library，避免污染 core layer |
| 平台範例程式 | 放在 `examples/` 下，避免 demo code 進入 driver core |

### 6.2 新增其他 SPI 傳輸模式

| 傳輸模式 | 建議位置 | 說明 |
|---|---|---|
| SPI polling | `ports/mh2030a/dm9051_hal_mh2030a_spi1.c` | 最基本、最容易除錯的傳輸方式。 |
| SPI DMA | `ports/mh2030a/dm9051_hal_mh2030a_spi1_dma.c` | 適合大量 RX/TX FIFO 傳輸。 |
| SPI interrupt | 可新增 `dm9051_hal_mh2030a_spi1_it.c` | 適合非阻塞式傳輸架構。 |
| RTOS SPI wrapper | 可新增 `dm9051_hal_mh2030a_spi1_rtos.c` | 適合 FreeRTOS task/queue/semaphore 架構。 |

### 6.3 新增其他 TCP/IP Stack

DM9051 core 不應直接包含 uIP 或 lwIP header。若要支援新的 stack，應新增 adapter。

```text
adapters/
  uip/               現有 uIP adapter
  lwip/              現有 lwIP adapter
  freertos_tcp/      未來可新增 FreeRTOS+TCP
  custom_stack/      未來可新增自訂 stack
```

| Stack | Adapter 責任 |
|---|---|
| uIP | 管理 `uip_buf`、poll、ARP timer、封包輸入輸出 |
| lwIP | 管理 `struct netif`、`pbuf`、`linkoutput`、input callback |
| FreeRTOS+TCP | 管理 network interface descriptor 與 buffer descriptor |
| 自訂 stack | 將 stack 的 frame I/O API 轉換成 `dm9051_core_receive()` / `dm9051_core_send()` |

### 6.4 建議的開發規範

| 規範 | 說明 |
|---|---|
| Core layer 不直接操作 SPI/GPIO | 所有硬體操作都應透過 `dm9051_hal_t`。 |
| Adapter layer 不直接操作暫存器 | uIP/lwIP adapter 只應呼叫 core API。 |
| Port layer 不包含協定棧邏輯 | MH2030A port 只負責 MCU peripheral 與 HAL binding。 |
| Register 定義集中於 `dm9051_regs.h` | 避免同一個 register magic number 分散在多個檔案。 |
| 共用型別集中於 `dm9051_types.h` | 避免 core、adapter、port 對 runtime state 有不同理解。 |
| 範例程式不可成為 driver 相依條件 | `examples/` 可使用 driver，但 driver 不應依賴 examples。 |
| 修改前執行 GitNexus impact analysis | 確認更動符號的 blast radius 後再編輯。 |

## 7. 總體評估

目前 `dm9051_driver` 的結構已經從傳統單一平台綁定式 driver，逐步整理成可移植、可測試、可擴充的分層架構。核心方向是正確的：DM9051A 晶片邏輯集中在 `core/`，硬體差異收斂到 `ports/`，協定棧差異收斂到 `adapters/`。

**專案規模** (GitNexus 索引)：24,768 個符號、39,272 個關聯、300 條執行流程。

**專案檔案**：`ModuleDemo/DM9051A/DM9051A.uvprojx` 包含 5 個 Build Target，涵蓋 SPI Probe、SPI DMA、uIP Polling、uIP DMA、uIP Interrupt 五種功能組合。

後續維護時，最重要的是守住 layer boundary。只要避免讓 core 直接依賴 MH2030A，也避免讓 adapter 直接碰 DM9051 register，這份 driver 之後要支援 DMA、中斷、不同 TCP/IP stack 或不同 MCU，都會比較穩定。
