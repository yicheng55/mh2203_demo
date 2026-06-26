# DM9051 Driver Porting Guide

將 DM9051 SPI 乙太網路驅動移植到新的 MCU 平台的逐步指南。

> **檔案狀態**：`ModuleDemo/DM9051A/dm9051_driver/docs/PORTING_GUIDE.md`

---

## 目錄

- [1. 分層架構](#1-分層架構)
- [2. 移植步驟](#2-移植步驟)
  - [Step 1：建立 port 目錄](#step-1-建立-port-目錄)
  - [Step 2：實作 SPI 傳輸原語](#step-2-實作-spi-傳輸原語)
  - [Step 3：實作 GPIO](#step-3-實作-gpio)
  - [Step 4：實作延時函式](#step-4-實作延時函式)
  - [Step 5：(選用) 實作 IRQ 中斷](#step-5-選用-實作-irq-中斷)
  - [Step 6：(選用) 實作 Critical Section](#step-6-選用-實作-critical-section)
  - [Step 7：實作 HAL Bind 函式](#step-7-實作-hal-bind-函式)
  - [Step 8：撰寫 Smoke Test 驗證](#step-8-撰寫-smoke-test-驗證)
- [3. HAL vtable 操作合約摘要](#3-hal-vtable-操作合約摘要)
- [4. MH2030A 參考實作對映表](#4-mh2030a-參考實作對映表)
- [5. 建置整合](#5-建置整合)
  - [5.1 Keil MDK `.uvprojx` 設定](#51-keil-mdk-uvprojx-設定)
  - [5.2 巨集選擇組合](#52-巨集選擇組合)
  - [5.3 CMake / Make](#53-cmake--make)
- [6. 依賴方向規則](#6-依賴方向規則)
- [7. 常見問題](#7-常見問題)
- [8. 參考文件](#8-參考文件)

---

## 1. 分層架構

```
application / uIP / lwIP          ← 你的應用程式或協定棧
        ↓
  network stack adapter            ← adapters/uip/ 或 adapters/lwip/
        ↓
  DM9051 core driver               ← core/ 晶片初始化、RX、TX、PHY、IRQ
        ↓
  DM9051 HAL interface             ← hal/inc/dm9051_hal.h (vtable)
        ↓
  MCU port layer                   ← ports/<your_mcu>/ 你要實作的部分
```

**移植目標**：實作最底層的 MCU port layer，填入 `dm9051_hal_ops_t` vtable 的所有函式指標。

---

## 2. 移植步驟

### Step 1: 建立 port 目錄

在 `ports/` 下為你的 MCU 建立目錄，例如 `ports/my_mcu/`。參考 `ports/mh2030a/` 的結構。

最少需要建立 3 個檔案：

| 檔案 | 用途 |
|------|------|
| `dm9051_hal_<mcu>_spi.h` | 公開的 port API：config 結構、HAL bind 函式 |
| `dm9051_hal_<mcu>_spi.c` | vtable 實作：SPI 收發、GPIO CS/RST、delay |
| `platform.h` | 包含 MCU 暫存器標頭、基本型別 |

選用檔案：

| 檔案 | 用途 |
|------|------|
| `dm9051_hal_<mcu>_int.c` | EXTI/NVIC 中斷支援 |
| `dm9051_hal_<mcu>_spi_dma.c` | SPI DMA 傳輸支援 |
| `board.c` / `board.h` | 板級初始化 (時脈、UART、tick) |
| `delay.c` / `delay.h` | 延時函式 (如無現成可用) |

---

### Step 2: 實作 SPI 傳輸原語

Core driver 透過 4 個 HAL 操作與 DM9051 晶片通訊。這是移植的核心工作。

#### 2.1 暫存器讀寫 (`read_reg` / `write_reg`)

SPI 協定：

- 命令位元組 = 暫存器位址 `|` `DM9051_OPC_REG_R` (0x00) 或 `DM9051_OPC_REG_W` (0x80)
- CS 低電位有效，整個交易完成後釋放
- 讀取：發送命令 → 發送 dummy byte (0x00) 同時接收資料 → 結束交易
- 寫入：發送命令 → 發送資料位元組 → 結束交易

```c
// 讀取範例 (參考 dm9051_hal_mh2030a_spi1.c:230)
int my_read_reg(void *ctx, uint8_t reg, uint8_t *val)
{
    cs_select();                                    // CS low
    spi_transfer_byte(cmd = reg | DM9051_OPC_REG_R); // 送命令
    spi_transfer_byte(0x00);                        // dummy, 接收資料在 *val
    spi_wait_idle();
    cs_deselect();                                  // CS high
    return DM9051_HAL_OK;
}
```

**合約** (`HAL_CONTRACT.md` 參照)：

| 規則 | 說明 |
|------|------|
| `reg` 是 DM9051 暫存器位址 (不含 SPI opcode) | Port 層負責組合 opcode |
| `val == NULL` 時回傳 `DM9051_HAL_ERR_PARAM` | |
| 失敗時 `*val` 內容不應被使用 | |
| CS 由 port 層完全控制 | |

#### 2.2 FIFO 大量讀寫 (`read_mem` / `write_mem`)

- `read_mem`：使用 MRCMD (0x72) 自動遞增位址讀取 RX FIFO
- `write_mem`：使用 MWCMD (0x78) 自動遞增位址寫入 TX FIFO
- `len == 0` 為合法，應直接回傳 `DM9051_HAL_OK`
- `buf == NULL && len != 0` 需回傳 `DM9051_HAL_ERR_PARAM`

```c
// RX FIFO 讀取範例 (dm9051_hal_mh2030a_spi1.c:297)
int my_read_mem(void *ctx, uint8_t *buf, uint16_t len)
{
    if (len == 0) return DM9051_HAL_OK;

    cs_select();
    spi_transfer_byte(DM9051_MRCMD | DM9051_OPC_REG_R);
    for (i = 0; i < len; i++)
        spi_transfer_byte(0x00, &buf[i]);
    spi_wait_idle();
    cs_deselect();
    return DM9051_HAL_OK;
}
```

#### 2.3 SPI 初始化

需設定：

| 參數 | MH2030A 參考值 | 說明 |
|------|----------------|------|
| Mode | Master | SPI 主模式 |
| Data Size | 8-bit | |
| CPOL | Low (0) | 時脈極性 |
| CPHA | 1 Edge (0) | 時脈相位 |
| NSS | Software | 軟體 CS |
| Baud Rate | /4 (18MHz @ 72MHz) | 不高於 25MHz |
| MSB First | yes | |

---

### Step 3: 實作 GPIO (CS / RST)

| 訊號 | 說明 |
|------|------|
| CS (Chip Select) | CS low 選中 DM9051，transaction 結束後拉 high |
| RST (Reset) | 硬體重置：拉 low → 等待 2ms → 拉 high → 等待 10ms |

```c
// 重置序列 (dm9051_hal_mh2030a_spi1.c:182)
void my_reset(void *ctx)
{
    gpio_rst_low();
    delay_ms(2);
    gpio_rst_high();
    delay_ms(10);
}
```

CS 接腳參考 (MH2030A)：PA15。RST 接腳參考：PF7。

---

### Step 4: 實作延時函式 (`delay_ms` / `delay_us`)

Core 在 init 和 PHY 操作時需要精確延時。兩個函式必須在驅動初始化早期就可用。

| 函式 | 精度要求 | 用途 |
|------|----------|------|
| `delay_ms(ms)` | ±1ms | 晶片重置、PHY 操作等待 |
| `delay_us(us)` | ±1µs | SPI timeout polling、PHY busy 輪詢 |

實作方式建議：

- **delay_ms**：使用 SysTick 中斷計數器 (典型 1ms tick)
- **delay_us**：使用 CPU 迴圈計數或硬體 timer

---

### Step 5: (選用) 實作 IRQ 中斷

如果使用 polling 模式，此步驟可跳過 (vtable 設 NULL 即可)。

實作 EXTI/NVIC 中斷支援：

| HAL op | 行為 |
|--------|------|
| `irq_enable(ctx)` | 使能 NVIC 中斷通道 |
| `irq_disable(ctx)` | 禁能 NVIC 中斷通道 |

IRQ handler 流程 (參考 `dm9051_hal_mh2030a_int.c`)：

```c
void EXTI4_15_IRQHandler(void)
{
    if (EXTI_GetITStatus(DM9051_MH2030A_INT_LINE) != RESET) {
        dm9051_mh2030a_irq_handler();  // 呼叫 core 的 interrupt_set
        EXTI_ClearITPendingBit(DM9051_MH2030A_INT_LINE);
    }
}
```

中斷接腳參考 (MH2030A)：PF6 / EXTI6 / EXTI4_15_IRQn。

---

### Step 6: (選用) 實作 Critical Section

選擇性實作。Polling-only 單一實例可設為 NULL。

```c
uint32_t my_enter_critical(void *ctx)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

void my_exit_critical(void *ctx, uint32_t state)
{
    __set_PRIMASK(state);
}
```

---

### Step 7: 實作 HAL Bind 函式

建立 `dm9051_hal_mh2030a_spi1.h` 風格的公開 API，提供 `dm9051_mh2030a_hal_bind()` 將 vtable 綁定到 `dm9051_hal_t`：

```c
// 結構定義 (參考 dm9051_hal_mh2030a_spi1.h)
typedef struct my_mcu_config {
    uint8_t transport;       // 0=polling, 1=dma
    uint8_t irq_mode;        // 0=off, 1=exti
    uint32_t spi_timeout;    // SPI timeout 計數
    // pin 定義可用巨集或硬編碼
} my_mcu_config_t;

// HAL 綁定函式 (參考 dm9051_hal_mh2030a_spi1.c:440)
int my_mcu_hal_bind(dm9051_hal_t *hal, const my_mcu_config_t *config)
{
    hal->ctx = (void *)config;
    hal->ops = &my_polling_ops;  // 或 &my_dma_ops
    return DM9051_HAL_OK;
}
```

vtable 實例 (參考 `dm9051_hal_mh2030a_spi1.c:11`)：

```c
const dm9051_hal_ops_t my_polling_ops = {
    .read_reg        = my_read_reg,
    .write_reg       = my_write_reg,
    .read_mem        = my_read_mem,
    .write_mem       = my_write_mem,
    .reset           = my_reset,
    .delay_ms        = my_delay_ms,
    .delay_us        = my_delay_us,
    .irq_enable      = my_irq_enable,   // 可 NULL
    .irq_disable     = my_irq_disable,  // 可 NULL
    .enter_critical  = my_enter_critical, // 可 NULL
    .exit_critical   = my_exit_critical,  // 可 NULL
};
```

---

### Step 8: 撰寫 Smoke Test 驗證

參考 `examples/uip_mh2030a_demo/dm9051_uip_mh2030a_smoke.c` 的流程：

```c
int main(void)
{
    dm9051_device_t dev;
    dm9051_hal_t hal;
    dm9051_config_t config;
    my_mcu_config_t port_config;

    board_init();                              // 初始化時脈、UART、delay

    dm9051_core_default_config(&config);       // 填預設 core config
    config.mac_addr = my_mac;

    my_mcu_default_config(&port_config);        // 填 port config

    my_mcu_hal_bind(&hal, &port_config);        // 綁定 HAL vtable

    int status = dm9051_core_open(&dev, &config, &hal);

    if (status == DM9051_OK) {
        printf("VID=0x%04X PID=0x%04X CHIPR=0x%02X\n",
               dm9051_core_vendor_id(&dev),
               dm9051_core_product_id(&dev),
               dm9051_core_chip_revision(&dev));
        printf("DM9051 found and opened successfully!\n");
    } else {
        printf("DM9051 open failed: %d\n", status);
    }

    while (1);
}
```

**驗證標準**：`dm9051_core_open()` 成功時，`vendor_id` 應為 `0x0A46`，`product_id` 應為 `0x9051`。

---

## 3. HAL vtable 操作合約摘要

| HAL op | 原型 | 必選 | 關鍵規則 |
|--------|------|------|----------|
| `read_reg` | `int (*)(void *ctx, uint8_t reg, uint8_t *val)` | **是** | `reg` 不含 SPI opcode；`val==NULL` 回傳 `ERR_PARAM` |
| `write_reg` | `int (*)(void *ctx, uint8_t reg, uint8_t val)` | **是** | |
| `read_mem` | `int (*)(void *ctx, uint8_t *buf, uint16_t len)` | **是** | `len==0` 合法；`buf==NULL && len!=0` 回傳 `ERR_PARAM` |
| `write_mem` | `int (*)(void *ctx, const uint8_t *buf, uint16_t len)` | **是** | 同上 |
| `reset` | `void (*)(void *ctx)` | 否 | 可為 NULL，core 會用 soft reset |
| `delay_ms` | `void (*)(uint32_t ms)` | **是** | 必須在 early init 可用 |
| `delay_us` | `void (*)(uint32_t us)` | **是** | 同上 |
| `irq_enable` | `void (*)(void *ctx)` | 否 | 僅閘控 MCU 中斷，不操作 DM9051 IMR |
| `irq_disable` | `void (*)(void *ctx)` | 否 | 同上 |
| `enter_critical` | `uint32_t (*)(void *ctx)` | 否 | 回傳 platform state token |
| `exit_critical` | `void (*)(void *ctx, uint32_t state)` | 否 | 接收 enter_critical 回傳的 token |

回傳值代碼：

| 代碼 | 值 | 意義 |
|------|----|------|
| `DM9051_HAL_OK` | 0 | 成功 |
| `DM9051_HAL_ERR` | -1 | 一般錯誤 |
| `DM9051_HAL_ERR_TIMEOUT` | -2 | 逾時 |
| `DM9051_HAL_ERR_PARAM` | -3 | 參數錯誤 |
| `DM9051_HAL_ERR_NOT_READY` | -4 | 功能尚未就緒 |

---

## 4. MH2030A 參考實作對映表

| HAL op | MH2030A 實作位置 | 實作摘要 |
|--------|------------------|----------|
| `read_reg` / `write_reg` | `dm9051_hal_mh2030a_spi1.c:230` | SPI1 polling，CS PA15，cmd = reg `|` opcode |
| `read_mem` / `write_mem` | `dm9051_hal_mh2030a_spi1.c:297` | SPI1 MRCMD/MWCMD 序列 |
| `reset` | `dm9051_hal_mh2030a_spi1.c:190` | SPI init + GPIO PF7 重置序列 |
| `delay_ms` / `delay_us` | `dm9051_hal_mh2030a_spi1.c:199` | 包裝 `Delay_Ms()` / `Delay_Us()` (SysTick) |
| `irq_enable` / `irq_disable` | `dm9051_hal_mh2030a_int.c` | NVIC EXTI4_15_IRQn 閘控 |
| `enter_critical` / `exit_critical` | `dm9051_hal_mh2030a_spi1.c:214` | `__get_PRIMASK()` / `__disable_irq()` |

接腳配置 (MH2030A 參考)：

| 訊號 | 接腳 |
|------|------|
| SPI1 CS | PA15 |
| SPI1 SCK | PB3 |
| SPI1 MISO | PB4 |
| SPI1 MOSI | PB5 |
| DM9051 RST | PF7 |
| DM9051 INT | PF6 / EXTI6 |

---

## 5. 建置整合

### 5.1 Keil MDK `.uvprojx` 設定

本專案的 Keil 專案檔為 `DM9051A.uvprojx`，內含 5 個 Target。以下逐一說明 `.uvprojx` XML 中各區段的意義與設定方法。

#### `.uvprojx` XML 結構總覽

```xml
<Project>
  <Targets>
    <Target>                              ← 一個編譯目標
      <TargetName>DM9051A</TargetName>     ← 顯示在 Keil 下拉選單的名稱
      <TargetOption>
        <TargetCommonOption>              ← 輸出目錄、輸出檔名、Device、CPU、記憶體
          <OutputDirectory>..\OBJ\</OutputDirectory>
          <OutputName>DM9051A</OutputName>
          <Device>ARMCM0</Device>
          <Cpu>IROM(...,0x40000) IRAM(...,0x20000) CPUTYPE("Cortex-M0")</Cpu>
        </TargetCommonOption>
        <TargetArmAds>
          <Cads>                          ← C 編譯器設定 (Target 層級)
            <Optim>1</Optim>              ← 最佳化等級 (0-3)
            <wLevel>0</wLevel>            ← Warning 等級
            <uC99>0</uC99>                ← C99 模式
            <VariousControls>
              <Define>USE_STDPERIPH_DRIVER</Define>  ← 全域巨集定義
              <IncludePath>...;...;...</IncludePath>  ← 全域 include 路徑
            </VariousControls>
          </Cads>
        </TargetArmAds>
      </TargetOption>
      <Groups>                            ← 原始碼群組清單
        <Group>
          <GroupName>USER</GroupName>
          <Files>                         ← 群組內的檔案
            <File>
              <FileName>main.c</FileName>
              <FileType>1</FileType>       ← 1=C source, 2=Assembly
              <FilePath>.\main.c</FilePath>
            </File>
            <File>...</File>
          </Files>
        </Group>
      </Groups>
    </Target>
    <Target>...</Target>                  ← 其他 Target
  </Targets>
</Project>
```

---

#### Target 層級設定

每個 `<Target>` 區塊代表 Keil 下拉選單中的一個編譯目標。5 個 Target 的核心差異：

| XML 元素 | 用途 |
|----------|------|
| `TargetName` | 顯示名稱，例如 `DM9051A`、`MH2030A_DM9051_uIP` |
| `OutputDirectory` | 輸出目錄 (HEX/OBJ 存放處)，例如 `..\OBJ\`、`..\OBJ_UIP\` |
| `OutputName` | 輸出檔名，例如 `DM9051A`、`MH2030A_DM9051_uIP` |
| `Device` / `Cpu` | 目標晶片與核心，所有 Target 共用 `ARMCM0` / `Cortex-M0` |
| `Define` | C 編譯器巨集，各 Target 的主要分流機制 |
| `IncludePath` | include 搜尋路徑，各 Target 不同 |
| `Optim` | 最佳化等級 (全部為 1) |
| `wLevel` | Warning 等級 (`DM9051A_SPI_DMA`=2，其餘=0) |
| `uC99` | C99 啟用 (`MH2030A_DM9051_uIP_int`=1，其餘=0) |

**5 個 Target 的巨集與輸出設定：**

| TargetName | OutputDir | OutputName | Define |
|-----------|-----------|------------|--------|
| `DM9051A` | `..\OBJ\` | `DM9051A` | `USE_STDPERIPH_DRIVER` |
| `DM9051A_SPI_DMA` | `..\OBJ\` | `DM9051A_DMA` | `USE_STDPERIPH_DRIVER` |
| `MH2030A_DM9051_uIP` | `..\OBJ_UIP\` | `MH2030A_DM9051_uIP` | `USE_STDPERIPH_DRIVER,MH2030A_UIP_PORT` |
| `MH2030A_DM9051_uIP_dma` | `..\OBJ_UIP\` | `MH2030A_DM9051_uIP_dma` | `USE_STDPERIPH_DRIVER,MH2030A_UIP_PORT,MH2030A_DM9051_SPI_DMA` |
| `MH2030A_DM9051_uIP_int` | `..\OBJ_UIP_INT\` | `MH2030A_DM9051_uIP_int` | `USE_STDPERIPH_DRIVER,MH2030A_UIP_PORT,DMPLUG_INT` |

**包含路徑差異：**

- **基礎 Target** (`DM9051A`, `DM9051A_SPI_DMA`)：
  `..\SYSTEM\delay;..\USER;..\bsp;..\..\..\Libraries\MH20xxLib\inc;..\..\..\Libraries\CMSIS\Include`

- **uIP Target** (`MH2030A_DM9051_uIP`, `_dma`, `_int`)：
  以上路徑 **加上** `..\..\..\middlewares\3rd_party\uip\inc;..\..\..\middlewares\3rd_party\uip\port\at32f415_dm9051;..\..\..\apps\uip_dm9051_example_e1\...;..\..\..\drivers\dm9051_edriver_v1.6.1a_beta;..\port\mh2030a;..\port\uip`

---

#### Group 層級 IncludeInBuild

群組階層可用 `<GroupOption>` 設定 `IncludeInBuild`，控制整個群組是否參與編譯：

```xml
<Group>
  <GroupName>DM9051_CORE</GroupName>
  <GroupOption>
    <CommonProperty>
      <IncludeInBuild>0</IncludeInBuild>   ← 0=整個群組排除 (基礎 Target)
    </CommonProperty>
  </GroupOption>
  <Files>
    <File>...</File>
  </Files>
</Group>
```

| `IncludeInBuild` 值 | 意義 |
|---------------------|------|
| `1` | 參與編譯 (預設值，當無 GroupOption 時為 `1`) |
| `0` | **整個群組排除**，群組內所有檔案均不編譯 |
| `2` | 繼承上層設定 (即使用 Target 層級 `CommonProperty` 的 `IncludeInBuild`) |

**基礎 Target (`DM9051A`) 的群組狀態：**

| 群組名稱 | Group IncludeInBuild | 備註 |
|----------|---------------------|------|
| `USER` | (無) = 1 | 編譯 `main.c`，但 `mh20xx_it.c` 和 `main_uip_mh2030a.c` 有 `IncludeInBuild=0` 的檔案層級覆寫 |
| `BSP` | (無) = 1 | 編譯 `dm9051a.c`、`mh2030a_spi1.c`；排除 `mh2030a_spi1_dma.c` |
| `SYSTEM` | (無) = 1 | |
| `CORE` | (無) = 1 | `startup_mh20xx.s` |
| `FWLib` | (無) = 1 | MH20xx 標準外設庫 |
| `MH2030A_PORT` | **0** | 整個群組排除 (uIP 專用 port 檔案) |
| `DM9051_CORE` | **0** | 整個群組排除 (DM9051 beta 驅動) |
| `uIP_CORE` | **0** | 整個群組排除 (uIP 協定棧原始碼) |
| `uIP_APP` | **0** | 整個群組排除 (uIP 應用層) |
| `WEB_APP` | **0** | 整個群組排除 (Web Server) |

**uIP Target (`MH2030A_DM9051_uIP`) 的群組狀態：**

- 上列 `MH2030A_PORT`、`DM9051_CORE`、`uIP_CORE`、`uIP_APP`、`WEB_APP` 的 Group `IncludeInBuild` 改為 `1` (或省略 = `1`)
- 部分檔案有各自的 `IncludeInBuild=0` 檔案層級覆寫 (見下節)

---

#### File 層級 IncludeInBuild

當某個檔案需要跨 Target 排除時，可在 `<File>` 內加 `<FileOption>`：

```xml
<File>
  <FileName>mh2030a_spi1_dma.c</FileName>
  <FileType>1</FileType>
  <FilePath>..\bsp\mh2030a_spi1_dma.c</FilePath>
  <FileOption>
    <CommonProperty>
      <IncludeInBuild>0</IncludeInBuild>   ← 僅此檔案排除
    </CommonProperty>
  </FileOption>
</File>
```

**基礎 Target (`DM9051A`) 各檔案的 IncludeInBuild 狀態：**

| 群組 | 檔案 | IncludeInBuild | 原因 |
|------|------|:--------------:|------|
| `USER` | `main.c` | 1 (預設) | 主要 entry point |
| `USER` | `mh20xx_it.c` | **0** | 中斷向量，該 Target 未使用 |
| `USER` | `main_uip_mh2030a.c` | **0** | uIP entry point，基礎 Target 不使用 |
| `BSP` | `dm9051a.c` | 1 | SPI probe 邏輯 |
| `BSP` | `mh2030a_spi1.c` | 1 | Polling SPI 傳輸 |
| `BSP` | `mh2030a_spi1_dma.c` | **0** | DMA SPI，基礎 Target 用 Polling |

**uIP Target (`MH2030A_DM9051_uIP`) 的關鍵檔案切換：**

| 群組 | 檔案 | IncludeInBuild | 原因 |
|------|------|:--------------:|------|
| `USER` | `main.c` | **0** | 排除基礎 probe main |
| `USER` | `main_uip_mh2030a.c` | 1 | 改用 uIP entry point |
| `MH2030A_PORT` | `mh2030a_dm9051_spi.c` | 1 | uIP polling SPI |
| `MH2030A_PORT` | `mh2030a_dm9051_spi_dma.c` | **0** | 排除 DMA 版本 |

**uIP DMA Target (`MH2030A_DM9051_uIP_dma`)：**

| 群組 | 檔案 | IncludeInBuild | 原因 |
|------|------|:--------------:|------|
| `MH2030A_PORT` | `mh2030a_dm9051_spi.c` | **0** | 排除 Polling 版本 |
| `MH2030A_PORT` | `mh2030a_dm9051_spi_dma.c` | 1 | 改用 DMA 版本 |

**uIP Interrupt Target (`MH2030A_DM9051_uIP_int`)：**

| 群組 | 檔案 | IncludeInBuild | 原因 |
|------|------|:--------------:|------|
| `MH2030A_PORT` | `mh2030a_dm9051_spi.c` | 1 | 使用 Polling SPI + 中斷通知 |
| `MH2030A_PORT` | `mh2030a_dm9051_spi_dma.c` | **0** | 排除 DMA |
| `USER` | `mh2030a_dm9051_int.c` | 1 | 啟用 EXTI6 中斷 |

---

#### 移植時新增檔案的步驟

將新的 port 檔案加入現有 `.uvprojx`：

**Step A：新增群組 (選擇性)**

若需建立新的檔案群組，在 `<Groups>` 內加入：

```xml
<Group>
  <GroupName>MY_MCU_PORT</GroupName>
  <Files>
    <File>
      <FileName>dm9051_hal_my_mcu_spi.c</FileName>
      <FileType>1</FileType>
      <FilePath>..\dm9051_driver\ports\my_mcu\dm9051_hal_my_mcu_spi.c</FilePath>
    </File>
  </Files>
</Group>
```

**Step B：將檔案加入現有群組**

在對應群組的 `<Files>` 區塊內新增 `<File>` 項目，並指定 `FilePath` 為相對路徑 (相對於 `.uvprojx` 所在目錄)。

**Step C：設定檔案層級的 IncludeInBuild**

- 若檔案需要在**特定 Target 才編譯**，在該 Target 的群組內加入 `<FileOption>` 設定 `IncludeInBuild=0`。
- 注意：同一份 XML 內**所有 Target 共用同一份 `<Groups>` 結構**。檔案排除是透過**各 Target 各自維護**的 `<FileOption>` 來達成。

> **重要**：`uvprojx` 中每個 Target 雖然共用相同的群組/檔案結構，但 Keil IDE 會為每個 Target 分別儲存檔案的 `IncludeInBuild` 狀態。當你在 IDE 中右鍵點選檔案 → `Options for File` → 取消勾選 `Include in Target Build`，Keil 會為**當前選中的 Target** 寫入 `IncludeInBuild=0`。

**Step D：調整 Target 巨集與 IncludePath**

在該 Target 的 `<Cads><VariousControls>` 中：
- `<Define>` 加入 `USE_STDPERIPH_DRIVER` 以及所需的傳輸模式巨集
- `<IncludePath>` 加入 `..\dm9051_driver\core\inc;..\dm9051_driver\hal\inc;..\dm9051_driver\ports\my_mcu`

---

#### Target 切換機制總結

5 個 Target 的功能切換依賴三層分流：

1. **C 編譯器巨集**：`MH2030A_UIP_PORT` 切換 `main` vs `main_uip_mh2030a` 的 `#ifndef`/`#ifdef`；`MH2030A_DM9051_SPI_DMA` 啟用 DMA 路徑；`DMPLUG_INT` 啟用中斷路徑
2. **Group IncludeInBuild**：uIP 相關群組 (`MH2030A_PORT`、`DM9051_CORE`、`uIP_CORE`、`uIP_APP`、`WEB_APP`) 在基礎 Target 中設為 `0` 排除
3. **File IncludeInBuild**：特定檔案 (`main.c` vs `main_uip_mh2030a.c`、`mh2030a_dm9051_spi.c` vs `mh2030a_dm9051_spi_dma.c`) 在各 Target 中各自獨立設定

---

### 5.2 巨集選擇組合

| Transport | IRQ | 巨集設定 |
|-----------|-----|----------|
| polling | off | (無額外定義) |
| polling | on | `DM9051_MH2030A_USE_IRQ=1` |
| dma | off | `DM9051_MH2030A_USE_DMA=1` |
| dma | on | `DM9051_MH2030A_USE_DMA=1`, `DM9051_MH2030A_USE_IRQ=1` |

### 5.3 CMake / Make

參考專案根目錄 `Makefile` (目前僅為標示用，需自行擴充 include 與 source 列表)：

```makefile
INCLUDES += -Idm9051_driver/core/inc
INCLUDES += -Idm9051_driver/hal/inc
INCLUDES += -Idm9051_driver/ports/my_mcu

SRCS += dm9051_driver/core/src/dm9051_core.c
SRCS += dm9051_driver/ports/my_mcu/dm9051_hal_my_mcu_spi.c
```

---

## 6. 依賴方向規則

| 層級 | 可 include | 不可 include |
|------|------------|-------------|
| `core/` | `stdint.h`, `dm9051_regs.h`, `dm9051_types.h` | uIP, lwIP, MH2030A, Keil target headers |
| `hal/` | `stdint.h` | DM9051 app code, uIP, lwIP, MCU headers |
| `ports/<mcu>/` | `hal/inc/`, MCU peripheral library, delay | uIP/lwIP application logic |
| `adapters/` | `core/inc/` | MCU SPI/GPIO/IRQ headers |

---

## 7. 常見問題

1. **SPI 讀取 VID 總是 0xFFFF** → 檢查 CS 極性 (DM9051 CS low active)、SPI 模式 (CPOL=0, CPHA=0)、時脈頻率 (勿超過 25MHz)
2. **`dm9051_core_open()` 回傳 `ERR_PARAM`** → 檢查 HAL vtable 是否所有必選函式都已填入，`dm9051_config_t` 的 `interrupt_mode` 是否合法
3. **RX 收不到封包** → 確認 `RCR_RXEN` 已設定、檢查 NSR link status、確認 PHY 已連線
4. **TX 逾時** → 確認 TCR TXREQ bit 在傳送後是否確實清除、檢查 TX FIFO 寫入長度是否正確
5. **中斷沒觸發** → 確認 `DMPLUG_INT` 或對應巨集已定義、EXTI 觸發邊緣正確 (DM9051 INT active low)、NVIC 優先權設定

---

## 8. 參考文件

| 文件 | 內容 |
|------|------|
| `docs/plan/HAL_CONTRACT.md` | HAL 操作完整語意合約 |
| `docs/plan/API_BOUNDARY.md` | 分層依賴規則 |
| `docs/plan/STATE_MODEL.md` | 狀態模型 (`dm9051_device_t`) |
| `docs/plan/CORE_API_PLAN.md` | Core API 設計與 legacy 相容 |
| `docs/plan/BUILD_SELECTION.md` | Keil target 編譯選擇 |
| `ports/mh2030a/` | MH2030A 完整參考實作 |
| `examples/uip_mh2030a_demo/` | 驗證用 smoke test 範例 |
