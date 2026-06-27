# MH2203 DM9051 uIP Demo

MH2203 DM9051 的 uIP 整合 demo，使用分層驅動 (core → HAL → port mh2203) 與
staged uIP adapter，跑完整 app（web + DHCP + DNS + TCP/UDP appcall）。

## 組成

| 檔案 | 角色 |
|---|---|
| `main_uip_mh2203_demo.c` | 整合入口：platform init → open → uIP stack init → httpd_init → 主迴圈 |
| `dm9051_uip_mh2203_smoke.c/.h` | port 開啟膠合：`_open(mac)` (HAL bind + core_open) / `_eth()` / `_init` 純探測 |

主迴圈每圈呼叫 `dm9051_uip_link_poll()` 與 `dm9051_uip_stack_poll()`。
uIP 計時基準由 port 的 `mh2203_uip_tick_init()` + SysTick (`mh2203_uip_clock.c`) 驅動。

## 建置

以 Keil 專案 `ModuleDemo/DM9051A/USER/DM9051A_mh2203_uip.uvprojx`
（target `MH2203_DM9051_uIP`）建置，前置定義：

```
USE_STDPERIPH_DRIVER, MH2203_UIP_PORT,
DM9051_MH2203_DIAG=1, DM9051_TX_WAIT_DONE=1,
DM9051_MH2203_USE_DMA=0, DM9051_MH2203_USE_IRQ=0
```

（目前 polling、無 IRQ/DMA；DMA/IRQ 保留未來。）

## 預期輸出 (序列埠 @115200)

```text
[MH2203 uIP] board init
[DM9051 uIP] MH2203 staged uIP demo start
[DM9051 uIP] open status=0 found=1 VID=0x0A46 PID=0x9051 CHIPR=0xXX
[DM9051 uIP] stack init mode=staging
[DM9051 uIP] MAC 00:60:6E:90:51:01
[DM9051 uIP] IP  192.168.249.37
[DM9051 uIP] HTTP server listening on port 80
[DM9051 uIP] Link UP
```

之後可 `ping` 板子 IP，或瀏覽器開 `http://<ip>/`。

## 待辦

- 靜態 IP / MAC 目前為佔位值 (`main_uip_mh2203_demo.c` 內標 `TODO`)，待正式值替換。
- DHCP 程式碼已編入，預設不作用 (`app_call.h` `DHCPC_EN=0`)；要改用 DHCP 時設為 1
  並於 `network_init()` 呼叫 `dhcpc_init`。
