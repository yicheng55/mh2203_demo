# C 語言命名規範（Clean Code 版）

> 適用於 C 語言程式碼的產生、重構、命名審查與 code review。  
> 本規範預設採用 `snake_case` 作為 C 專案主要命名風格，並以 Clean Code 的「清楚、可讀、可搜尋、可維護」為核心原則。

---

## 1. 總體原則

1. 命名必須一致、可讀、可搜尋、可維護。
2. 名稱應清楚表達用途，避免過度簡寫。
3. 除了迴圈索引 `i`, `j`, `k` 或極短作用域暫存變數外，不使用單字母命名。
4. 同一專案中只能使用一套主要風格，不得混用 `camelCase` 與 `snake_case`。
5. 本規範預設採用 `snake_case` 作為 C 專案主要風格。
6. 函數命名應以「動作」為核心，名稱需能清楚表達函數正在做什麼。
7. 對外 API、內部 helper、static 私有函數都必須維持一致的命名層次。

---

## 2. 命名風格總表

| 類型 | 規則 | 範例 | 說明 |
|---|---|---|---|
| 區域變數 | `lower_snake_case` | `total_count`, `retry_limit` | 描述用途，避免模糊名稱 |
| 函數參數 | `lower_snake_case` | `buffer_length`, `timeout_ms` | 與區域變數一致 |
| 全域變數 | `g_` + `lower_snake_case` | `g_system_state` | 明確表示全域範圍 |
| 檔案內靜態變數 | `s_` + `lower_snake_case` | `s_rx_buffer_size` | 表示 module-private |
| 指標變數 | `p_` + `lower_snake_case` | `p_buffer`, `p_device` | 明確指出為指標 |
| 雙重指標 | `pp_` + `lower_snake_case` | `pp_node`, `pp_out_list` | 僅在必要時使用 |
| 常數變數 | `UPPER_SNAKE_CASE` | `MAX_BUFFER_SIZE` | `const` 常數亦適用 |
| 巨集 | `UPPER_SNAKE_CASE` | `ARRAY_SIZE(x)` | 宏名稱全大寫 |
| 函數 | `module_action_target` | `uart_init`, `buffer_clear` | 模組 + 動作 + 目標，不加 `_fun` |
| 結構名稱 | `PascalCase` | `UartDevice`, `PacketHeader` | 結構型別名稱 |
| typedef 型別 | `snake_case_t` | `uart_device_t` | 預設使用 `_t` 風格 |
| 枚舉型別 | `PascalCase` | `UartStatus` | 型別名稱清楚 |
| 枚舉成員 | `ENUM_PREFIX_` + `UPPER_SNAKE_CASE` | `UART_STATUS_OK` | 必須帶型別前綴 |
| 錯誤碼 | `ERR_` + `UPPER_SNAKE_CASE` | `ERR_TIMEOUT`, `ERR_INVALID_PARAM` | 統一錯誤碼風格 |
| 布林變數 | `is_` / `has_` / `can_` + 描述 | `is_ready`, `has_error` | 便於閱讀條件判斷 |
| 模組私有函數 | `static` + `module_` 前綴 | `static uart_parse_frame(...)` | 避免命名衝突 |
| 檔名 | `lower_snake_case` | `uart_driver.c`, `net_utils.h` | 與函數/模組一致 |

---

## 3. 優先級規則

當多條規則可能同時適用時，依下列優先級決定。

### 3.1 作用域規則優先

全域、靜態、區域命名規則優先於一般命名風格。

```c
uint32_t g_system_tick;
static uint8_t s_rx_buffer[128];
```

### 3.2 型別語意優先

指標語意優先於一般變數命名。

```c
uint8_t *p_buffer;
uart_device_t *p_device;
```

若同時是全域指標：

```c
uart_device_t *g_p_device;
```

若同時是靜態指標：

```c
static uart_context_t *s_p_context;
```

### 3.3 模組前綴優先於通用動詞

函數應先表現模組歸屬，再表現行為。

```c
uart_init();
uart_read_byte();
net_packet_parse();
```

### 3.4 列舉成員必須帶型別前綴

```c
typedef enum
{
    UART_STATUS_OK = 0,
    UART_STATUS_BUSY,
    UART_STATUS_TIMEOUT,
    UART_STATUS_ERROR
} UartStatus;
```

不可只寫：

```c
OK
STATUS_OK
TIMEOUT
```

### 3.5 巨集與常數必須避免撞名

宏與常數若屬模組範圍，建議加模組前綴。

```c
#define UART_MAX_PORTS      4
#define UART_RX_BUFFER_SIZE 256
```

---

## 4. 衝突處理規則

### 4.1 可以自動裁決，不必詢問

以下情況請直接自動處理：

| 衝突情況 | 自動裁決 |
|---|---|
| `snake_case` 與 `camelCase` 衝突 | 一律採用 `snake_case` |
| 函數是否加 `_fun` 後綴 | 一律不加 `_fun`，除非使用者明確要求舊專案保留 |
| typedef 使用 `PascalCase` 或 `_t` 衝突 | 若既有專案多數為 `_t`，沿用 `_t`；未指定則用 `snake_case_t` |
| 宏與常數可能撞名 | 優先加入模組前綴 |
| 同名區域變數與成員名稱衝突 | 區域變數補語意，不使用 `temp1`, `temp2` |
| 指標命名過長 | 保留 `p_`，後段描述可合理縮短 |

範例：

```c
/* 不佳 */
uint8_t *p_received_packet_buffer;

/* 較佳 */
uint8_t *p_rx_buffer;
```

---

### 4.2 必須詢問使用者

只有以下情況才需要提出問題：

1. 使用者要求遵循既有專案風格，但專案內同時混用多套風格且無明顯主流。
2. 使用者要求與第三方 SDK / 晶片原廠 API 完全一致，而原廠風格與本規範衝突。
3. 需要決定 `typedef` 最終風格，但專案中 `PascalCase` 與 `_t` 使用比例接近，無法判定主流。
4. 使用者明確要求保留舊命名，即使該命名違反本規範。
5. 公開 API 命名已經成為外部相容介面，改名可能破壞相容性。

---

## 5. 前綴組合規則

當名稱同時具備多種屬性時，前綴順序固定如下：

```text
範圍前綴 + 指標前綴 + 主名稱
```

### 正確範例

```c
uart_device_t *g_p_device;
uint8_t *s_p_rx_buffer;
node_t **g_pp_node_list;
```

### 錯誤範例

```c
p_g_device;
device_g_p;
sp_buffer;
```

---

## 6. 函數命名規則（Clean Code 版）

函數名稱格式：

```text
module_action_target
```

### 6.1 命名核心

函數名稱應清楚表達：

```text
哪個模組 + 做什麼動作 + 作用在哪個目標
```

範例：

```c
uart_init();
uart_read_byte();
uart_write_buffer();
net_parse_packet();
device_reset();
buffer_clear();
```

---

### 6.2 函數行為命名以動詞為核心

C 專案通常需要模組前綴避免命名衝突，因此函數名稱不一定「整個名稱第一個字」就是動詞，而是應符合：

```text
module_action_target
```

其中 `action` 必須是明確動詞。

| 類型 | 範例 | 說明 |
|---|---|---|
| 初始化 | `uart_init` | 初始化 UART 模組 |
| 讀取 | `uart_read_byte` | 從 UART 讀取一個 byte |
| 寫入 | `uart_write_buffer` | 寫入 buffer |
| 清除 | `buffer_clear` | 清除 buffer |
| 解析 | `net_parse_packet` | 解析網路封包 |
| 重置 | `device_reset` | 重置裝置 |
| 啟用 | `timer_start` | 啟動 timer |
| 停止 | `timer_stop` | 停止 timer |
| 取得 | `sensor_get_value` | 取得 sensor 數值 |
| 設定 | `sensor_set_config` | 設定 sensor 參數 |

---

### 6.3 常用動詞建議表

| 動詞 | 用途 | 範例 |
|---|---|---|
| `init` | 初始化 | `uart_init` |
| `deinit` | 反初始化 / 釋放 | `uart_deinit` |
| `open` | 開啟資源 | `file_open` |
| `close` | 關閉資源 | `file_close` |
| `read` | 讀取資料 | `uart_read_byte` |
| `write` | 寫入資料 | `uart_write_buffer` |
| `send` | 發送資料 | `net_send_packet` |
| `receive` / `recv` | 接收資料 | `net_recv_packet` |
| `parse` | 解析資料 | `packet_parse_header` |
| `build` | 建立資料內容 | `packet_build_header` |
| `clear` | 清除內容 | `buffer_clear` |
| `reset` | 重置狀態 | `device_reset` |
| `start` | 啟動 | `timer_start` |
| `stop` | 停止 | `timer_stop` |
| `enable` | 啟用功能 | `irq_enable` |
| `disable` | 停用功能 | `irq_disable` |
| `get` | 取得值 | `gpio_get_level` |
| `set` | 設定值 | `gpio_set_level` |
| `check` | 檢查條件 | `link_check_status` |
| `update` | 更新狀態 | `system_update_tick` |
| `handle` | 處理事件 | `uart_handle_irq` |
| `process` | 處理流程 | `net_process_packet` |
| `convert` | 轉換格式 | `time_convert_ms_to_tick` |
| `copy` | 複製資料 | `buffer_copy` |
| `find` | 查找資料 | `list_find_node` |
| `remove` | 移除資料 | `list_remove_node` |

---

### 6.4 不建議的函數命名

| 不建議名稱 | 問題 | 建議名稱 |
|---|---|---|
| `uart_fun` | 無法表達用途，且 `_fun` 無實質意義 | `uart_init` / `uart_read_byte` |
| `do_uart` | 動作模糊 | `uart_handle_irq` |
| `process` | 缺少模組與目標 | `net_process_packet` |
| `handle_data` | 缺少模組與資料語意 | `uart_handle_rx_data` |
| `test1` | 無法維護與搜尋 | `uart_test_loopback` |
| `temp_proc` | 語意不明 | `packet_parse_header` |

---

## 7. 結構、列舉與型別規則

### 7.1 結構

預設 typedef 型別使用 `snake_case_t`。

```c
typedef struct
{
    uint8_t *p_buffer;
    uint16_t length;
    uint16_t capacity;
} uart_packet_t;
```

規則：

1. 結構欄位名稱使用 `lower_snake_case`。
2. 欄位若為指標，也使用 `p_`。
3. 欄位名稱應表達用途，不使用 `data1`, `value2`, `tmp`。

---

### 7.2 枚舉

枚舉型別使用 `PascalCase`，枚舉成員使用 `TYPEPREFIX_` + `UPPER_SNAKE_CASE`。

```c
typedef enum
{
    UART_STATUS_OK = 0,
    UART_STATUS_BUSY,
    UART_STATUS_TIMEOUT,
    UART_STATUS_ERROR
} UartStatus;
```

規則：

1. 枚舉型別用 `PascalCase`。
2. 枚舉成員必須帶型別前綴。
3. 枚舉成員不可只寫 `OK`, `ERROR`, `TIMEOUT`。

---

## 8. 布林、狀態與錯誤碼命名

### 8.1 布林值

布林變數應使用可以直接閱讀成條件句的名稱。

```c
bool is_ready;
bool has_data;
bool can_retry;
bool is_link_up;
```

不建議：

```c
bool flag1;
bool check;
bool status_ok;
```

---

### 8.2 狀態值

狀態變數應明確描述狀態來源與用途。

```c
system_state_t system_state;
link_status_t link_status;
uart_status_t uart_status;
```

不建議：

```c
uint8_t status1;
uint8_t state_tmp;
```

---

### 8.3 錯誤碼

錯誤碼使用 `ERR_` + `UPPER_SNAKE_CASE`。

```c
#define ERR_INVALID_PARAM   (-1)
#define ERR_TIMEOUT         (-2)
#define ERR_NOT_FOUND       (-3)
```

不建議：

```c
#define ERROR1              (-1)
#define FAIL                (-2)
#define TIMEOUT_ERROR       (-3)
```

---

## 9. AI 生成程式碼時的執行要求

AI 在產生、重構、審查 C 程式碼時，請依序執行：

1. 先判斷此專案是否已有既有命名主風格。
2. 若既有風格明確，優先與既有風格一致。
3. 若既有風格不明確，使用本規範預設風格。
4. 若遇到小衝突，先自動裁決，不要頻繁詢問。
5. 只有在涉及以下情況時才詢問使用者：
   - 公開 API 相容性
   - 第三方 SDK 命名一致性
   - 多套風格難以判定
   - 使用者明確要求保留舊命名
6. 在 code review 時，除了指出命名不一致，也必須提出「建議改名版本」。
7. 若是重構程式碼，應優先保留函數行為語意，再調整命名格式。
8. 若改名會影響公開 API，需標註相容性風險。

---

## 10. 命名審查輸出格式

當 AI 審查命名時，請使用以下格式：

```text
* 原名稱：
* 問題：
* 建議名稱：
* 理由：
```

### 範例

```text
* 原名稱：temp
* 問題：語意不明，無法表達實際用途
* 建議名稱：rx_timeout_ms
* 理由：名稱可直接反映其用途與單位
```

---

## 11. 預設裁決結論

若未另行指定，請固定使用以下預設：

| 項目 | 預設規則 |
|---|---|
| 主要風格 | `lower_snake_case` |
| 函數命名 | `module_action_target` |
| 函數動作語意 | `action` 使用明確動詞 |
| 函數後綴 | 不加 `_fun` |
| 全域變數 | `g_` |
| 靜態變數 | `s_` |
| 指標 | `p_` |
| 雙重指標 | `pp_` |
| 常數 | `UPPER_SNAKE_CASE` |
| 巨集 | `UPPER_SNAKE_CASE` |
| 枚舉成員 | `UPPER_SNAKE_CASE` 並帶型別前綴 |
| 枚舉型別 | `PascalCase` |
| typedef 型別 | `snake_case_t` |
| 檔名 | `lower_snake_case` |

---

## 12. 實務範例

### 12.1 變數命名

```c
uint32_t retry_count;
uint32_t timeout_ms;
uint8_t *p_rx_buffer;
bool is_ready;
bool has_error;
```

### 12.2 全域與靜態變數

```c
uint32_t g_system_tick;
uart_device_t *g_p_uart_device;

static uint8_t s_rx_buffer[256];
static uart_context_t *s_p_uart_context;
```

### 12.3 函數命名

```c
void uart_init(void);
int uart_read_byte(uint8_t *p_byte);
int uart_write_buffer(const uint8_t *p_buffer, uint16_t length);
void uart_handle_irq(void);

void net_parse_packet(const uint8_t *p_packet, uint16_t packet_length);
void net_process_packet(void);
```

### 12.4 結構與枚舉

```c
typedef struct
{
    uint8_t *p_buffer;
    uint16_t length;
    uint16_t capacity;
} uart_packet_t;

typedef enum
{
    UART_STATUS_OK = 0,
    UART_STATUS_BUSY,
    UART_STATUS_TIMEOUT,
    UART_STATUS_ERROR
} UartStatus;
```

---

## 13. 給 AI 的使用指令

請在任何新產生的 C 程式碼、重構結果、命名建議、code review 建議中，優先遵守本規範。

若遇到命名衝突，先依照本文件的「優先級規則」與「衝突處理規則」自動裁決。  
只有在公開 API 相容性、第三方 SDK 命名一致性、多套風格難以判定、或使用者明確要求保留舊命名時，才需要提出詢問。

審查命名時，請務必輸出：

```text
* 原名稱：
* 問題：
* 建議名稱：
* 理由：
```

---

## 14. 建議檔案名稱

```text
c_naming_convention_clean_code.md
```
