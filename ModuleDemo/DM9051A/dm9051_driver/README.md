# DM9051 Portable Driver

This directory contains the staged portable DM9051 SPI Ethernet driver for the
`ModuleDemo/DM9051A` project. It separates the chip driver, network-stack
adapters, and MH2030A board support so the same DM9051 core can be reused by
uIP, lwIP, or another MCU port.

The original `ModuleDemo/DM9051A` production targets remain the compatibility
baseline. This directory is used for incremental validation and refactoring; do
not remove, rename, or redirect the legacy driver files unless a later migration
phase explicitly switches a target.

## Layering

```text
application / uIP / lwIP
        |
network stack adapter
        |
DM9051 core driver
        |
DM9051 HAL vtable
        |
MCU port layer
        |
SPI / GPIO / IRQ / delay
```

Dependency direction is one-way:

- `core/` is stack-neutral and platform-neutral.
- `hal/inc/` defines the vtable contract used by the core.
- `ports/<mcu>/` owns SPI, GPIO, IRQ, delay, board bring-up, and MCU headers.
- `adapters/` owns uIP/lwIP integration and must not include MCU SPI/GPIO
  headers directly.

See `docs/plan/API_BOUNDARY.md` for the detailed ownership rules.

## Directory Map

| Path | Purpose |
| --- | --- |
| `core/inc/` | Public core API, register definitions, portable runtime/config types. |
| `core/src/` | Context-based DM9051 open/close, RX, TX, PHY, interrupt state, and debug helpers. |
| `hal/inc/dm9051_hal.h` | Portable HAL vtable used by the core. |
| `ports/mh2030a/` | MH2030A SPI1 polling transport, optional DMA FIFO transport, optional PF6/EXTI IRQ, board/clock/delay helpers. |
| `adapters/uip/` | uIP-facing adapter and optional staged uIP polling loop helper. |
| `adapters/lwip/` | lwIP `struct netif` adapter, `NO_SYS=1` input polling helper, and local `lwipopts.h`. |
| `examples/uip_mh2030a_demo/` | Hardware smoke test and optional staged uIP loop entry points. |
| `examples/lwip_mh2030a_demo/` | lwIP MH2030A example placeholder and integration notes. |
| `docs/PORTING_GUIDE.md` | Step-by-step guide for porting the driver to another MCU. |
| `docs/plan/` | Refactor planning notes for API boundary, HAL contract, build selection, state model, and adapter staging. |
| `Makefile` | Staging-only helper target; current Keil projects do not build through this file. |

## Current Status

- `core/src/dm9051_core.c` implements the staged context-based core API:
  configuration validation, HAL binding validation, chip-ID probe, RX, TX, PHY
  access, link status, MAC accessors, and interrupt event state.
- Legacy compatibility entry points such as `dm9051_conf()`, `dm9051_init()`,
  `dm9051_rx()`, and `dm9051_tx()` are still declared for staged migration
  planning.
- `ports/mh2030a/dm9051_hal_mh2030a_spi1.c` provides the MH2030A SPI1 polling
  HAL implementation.
- `ports/mh2030a/dm9051_hal_mh2030a_spi1_dma.c` provides optional DMA-backed
  FIFO transfers while keeping register access on byte polling.
- `ports/mh2030a/dm9051_hal_mh2030a_int.c` provides optional PF6 / EXTI line 6
  IRQ support and hands events back to the instance-based core.
- `adapters/uip/dm9051_uip.c` bridges staged core RX/TX/interrupt state to the
  uIP-facing adapter API.
- `adapters/uip/dm9051_uip_stack.c` contains the optional staged uIP poll loop
  using uIP, ARP, and periodic timers.
- `adapters/lwip/dm9051_lwip.c` provides `dm9051_if_init()`,
  `dm9051_lwip_input()`, link status, and compatibility wrappers for staged
  lwIP integration.

## Build And Validation

The original Keil `DM9051A.uvprojx` production project remains unchanged. The
separate staged project `ModuleDemo/DM9051A/USER/DM9051A_uip.uvprojx` includes
the staged uIP/polling-driver files for incremental validation.

Current staged validation entry points:

| Entry point | Role |
| --- | --- |
| `examples/uip_mh2030a_demo/main_uip_mh2030a_smoke.c` | Board bring-up, staged core open, uIP adapter attach, and hardware RX/TX smoke validation. |
| `examples/uip_mh2030a_demo/main_uip_mh2030a_demo.c` | Optional staged uIP loop using `dm9051_uip_stack_init()` and `dm9051_uip_stack_poll()`. |
| `examples/lwip_mh2030a_demo/main_dm9051_lwip_example.c` | lwIP `netif_add()` style example for future MH2030A lwIP target wiring. |

Feature selection for the MH2030A staged port is controlled by preprocessor
defines in `ports/mh2030a/dm9051_hal_mh2030a_spi1.h`:

| Define | Meaning |
| --- | --- |
| `DM9051_MH2030A_USE_DMA=1` | Select DMA FIFO transport at runtime glue level. |
| `DM9051_MH2030A_ENABLE_DMA=1` | Compile/link optional DMA transport support. |
| `DM9051_MH2030A_USE_IRQ=1` | Select EXTI IRQ mode at runtime glue level. |
| `DM9051_MH2030A_ENABLE_IRQ=1` | Compile/link optional IRQ support. |
| `DM9051_MH2030A_OWN_EXTI4_15_HANDLER=0` | Let an application-owned `EXTI4_15_IRQHandler()` call `dm9051_mh2030a_irq_handler()`. |

Default staged behavior is conservative: polling SPI and no IRQ.

## Porting

To port the driver to another MCU, implement a new `ports/<mcu>/` directory that
binds `dm9051_hal_t` to the platform's SPI, GPIO, reset, delay, optional IRQ,
and optional critical-section primitives.

The minimum required HAL operations are:

- `read_reg` / `write_reg`
- `read_mem` / `write_mem`
- `delay_ms` / `delay_us`

Recommended porting sequence:

1. Copy the shape of `ports/mh2030a/dm9051_hal_mh2030a_spi1.h`.
2. Implement polling SPI register and FIFO transactions first.
3. Add reset GPIO and delay hooks.
4. Bind the vtable with a platform-specific `*_hal_bind()` function.
5. Run a chip-ID smoke test through `dm9051_core_open()`.
6. Add IRQ or DMA only after polling mode is stable.

See `docs/PORTING_GUIDE.md` and `docs/plan/HAL_CONTRACT.md` for the complete
porting contract.

## Reference Documents

| Document | Content |
| --- | --- |
| `docs/PORTING_GUIDE.md` | MCU porting steps and validation checklist. |
| `docs/plan/API_BOUNDARY.md` | Dependency direction and layer ownership. |
| `docs/plan/BUILD_SELECTION.md` | Keil target/file selection model. |
| `docs/plan/CORE_API_PLAN.md` | Context-based core API and legacy wrapper plan. |
| `docs/plan/CORE_SPLIT_MAP.md` | `dm9051_beta.c` responsibility split map. |
| `docs/plan/HAL_CONTRACT.md` | HAL operation semantics and return codes. |
| `docs/plan/STATE_MODEL.md` | `dm9051_device_t` runtime state model. |
| `docs/plan/ADAPTER_STAGING.md` | uIP/lwIP adapter staging behavior. |
| `ports/mh2030a/README.md` | MH2030A pin mapping, build defines, and port status. |
| `examples/uip_mh2030a_demo/README.md` | uIP smoke test and optional uIP loop notes. |
| `examples/lwip_mh2030a_demo/README.md` | lwIP example status. |
