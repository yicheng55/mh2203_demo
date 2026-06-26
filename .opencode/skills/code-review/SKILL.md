---
name: code-review
description: Perform thorough code review for embedded C/C++ firmware projects, covering correctness, safety, performance, and coding standards
---

## What I do

Review code changes for the **DM9051 Portable Driver** (dm9051_driver/) — a staged-refactored 4-layer SPI Ethernet driver for **MH2030A** (Cortex-M0) + **AT32F403A/AT32F407** MCUs. Supports uIP and lwIP stacks. See `docs/plan/` for design docs: `HAL_CONTRACT.md`, `API_BOUNDARY.md`, `STATE_MODEL.md`, `CORE_API_PLAN.md`, `CORE_SPLIT_MAP.md`, `BUILD_SELECTION.md`, `ADAPTER_STAGING.md`.

### Architecture layers (strict one-way dependency)

```
Application / uIP / lwIP
        |
Network Stack Adapter  (adapters/uip/, adapters/lwip/)
        |
DM9051 Core Driver    (core/src/, core/inc/)
        |
DM9051 HAL vtable     (hal/inc/dm9051_hal.h)
        |
MH2030A Platform Port (ports/mh2030a/ — SPI, GPIO, IRQ, delay, board)
```

### Review checklist by layer

**Core layer** (`core/`):
- Context-based API violations: `dm9051_core_*` functions using global state instead of `dm9051_device_t *`
- Missing `dm9051_device_t` instance checks (`dev == 0`) before dereference
- Legacy API wrappers (`dm9051_conf`, `dm9051_rx`, `dm9051_tx`) calling through static internal state without HAL guard
- Debug module (`core/src/dm9051_debug.c`): currently a placeholder with no exported symbols; any future register-dump or diagnostic helpers added here must not depend on uIP/lwIP/MH2030A headers
- HAL vtable null-check bypass: calling `hal->ops->xxx` without verifying `hal`, `hal->ops`, or the specific op is non-NULL
- TX wait timeout: `DM9051_TX_WAIT_DONE` / `DM9051_TX_WAIT_TIMEOUT_US` — timeout loop must guarantee forward progress
- RX error histogram (`rxb_error_hist[]`) overflow: `DM9051_RXB_RESET_THRESHOLD` logic must not infinite-loop
- Bus acquire/release missing paired call on every early-return path
- TX wait timeout poll delay: `DM9051_TX_WAIT_POLL_DELAY_US=0` means tight spin — acceptable? watchdog implications?

**HAL vtable** (`hal/inc/dm9051_hal.h`):
- New HAL ops added without updating docs/plan/HAL_CONTRACT.md
- Error code mapping: `dm9051_core_hal_status()` maps `HAL_ERR -> DM9051_ERR_NOT_READY` — verify intent
- Mandatory ops (`read_reg`, `write_reg`, `read_mem`, `write_mem`, `delay_ms`, `delay_us`) must never be NULL in a production port
- `enter_critical`/`exit_critical` must always be paired; return token must be passed verbatim to `exit`; never do nested critical sections without recursion count

**Port layer** (`ports/mh2030a/`):
- SPI byte-transfer timeout: `dm9051_mh2030a_transfer_byte()` — TXE and RXNE polling loops each have `spi_timeout` but must handle partial transfers
- `dm9051_mh2030a_finish_transfer()` has a NULL deref bug: `config` checked *after* calling `wait_spi_idle(config)`
- DMA transport (`dm9051_hal_mh2030a_spi1_dma.c`): ops table fully populated with `dm9051_mh2030a_dma_read_mem`/`dma_write_mem`; `ERR_NOT_READY` only when `DM9051_MH2030A_ENABLE_DMA=0` — verify FIFO coherency when enabled
- `deselect()` / GPIO release must happen in all exit paths of `read_reg`, `write_reg`, `read_mem`, `write_mem`
- IRQ handler (`dm9051_hal_mh2030a_int.c`): EXTI line 6 PF6, `OWN_EXTI4_15_HANDLER` — must guard shared `dm9051_device_t` with critical section
- GPIO AF config for SPI1 SCK/PB3, MISO/PB4, MOSI/PB5 must match `dm9051_hal_mh2030a_spi1_priv.h`
- `dm9051_mh2030a_hal_bind()` stores config in a static `dm9051_mh2030a_bound_config` — single-instance limitation; multi-instance will break
- Board support (`mh2030a_board.c/h`): `mh2030a_uip_board_init()` sets SysTick + printf — must not be called before clock init; `mh2030a_uip_clock_init()` and `mh2030a_uip_tick_isr()` drive uIP `clock_time()` — tick rate mismatch breaks uIP timers
- Platform header (`mh2030a_platform.h`): aggregates `mh20xx.h` + `delay.h`; `USE_STDPERIPH_DRIVER` defined here may conflict with target-level defines
- uIP clock (`mh2030a_uip_clock.c/h`): provides `clock_time()` for uIP timers; must use the same tick source as board init
- Interrupt stubs (`mh20xx_it.c`): contains `SysTick_Handler`, `HardFault_Handler`, `EXTI4_15_IRQHandler` (if `OWN_EXTI4_15_HANDLER=1`) — shared IRQn name collision with user handlers

**Adapter layer**:
- uIP (`adapters/uip/`): single-instance static `dm9051_uip_attached_dev`; `dm9051_uip_attach()` before `dm9051_uip_init()` ordering; `dm9051_uip_interrupt_take()` in poll mode returns 1 (always ready) vs real IRQ mode check
- uIP ethernetif (`struct uip_ethernetif` in `dm9051_uip.h`): embeds `dm9051_device_t` + `dm9051_hal_t` + RX/TX buffers — `dm9051_uip_link_poll()` accesses `eth->dev` directly, must verify `eth != 0`
- uIP stack loop (`dm9051_uip_stack.c`): `dm9051_uip_stack_init()` calls `uip_init()` + `uip_arp_init()` + attaches device/ethernetif; `dm9051_uip_stack_poll()` drains RX with burst limit (`DM9051_UIP_RX_BURST_MAX=8`) — periodic timers (`CLOCK_SECOND/2` for TCP, `CLOCK_SECOND*10` for ARP) must not overflow on Cortex-M0 tick; `dm9051_uip_interrupt_take()` called inside poll path — lock ordering vs bus_acquire?
- lwIP (`adapters/lwip/`): **not yet staged** — directory contains only `lwipopts.h`; `dm9051_lwip.c` (planned) will provide `struct netif` adapter, pbuf RX/TX, `ethernetif_input()` — must not be called from ISR unless NO_SYS=1 is confirmed

**Examples layer** (`examples/`):
- uIP smoke test (`examples/uip_mh2030a_demo/main_uip_mh2030a_smoke.c`): board bring-up, `dm9051_core_open`, uIP attach, and RX/TX smoke — must not silently pass if chip-ID probe fails
- uIP demo (`examples/uip_mh2030a_demo/main_uip_mh2030a_demo.c`): optional `dm9051_uip_stack_init()`/`dm9051_uip_stack_poll()` entry — `dm9051_uip_stack_poll()` blocks inside periodic timer loops; watchdog implications?
- lwIP example (`examples/lwip_mh2030a_demo/main_dm9051_lwip_example.c`): `netif_add()` style wiring — currently staging placeholder; must verify `netif->state` lifetime

**Staged migration concerns**:
- Legacy symbols (`dm9051_conf`, `dm9051_init`, `dm9051_rx`, `dm9051_tx`) must NOT be renamed or removed until production targets are migrated
- No new includes of uIP/lwIP/MH2030A headers in `core/` or `hal/`
- No new includes of MH2030A SPI/GPIO/IRQ headers in `adapters/`
- `DM9051_USE_UIP` and `DM9051_USE_LWIP` must not both be defined
- Feature flags (`DM9051_MH2030A_USE_DMA`, `DM9051_MH2030A_USE_IRQ`, `DM9051_MH2030A_ENABLE_DMA`, `DM9051_MH2030A_ENABLE_IRQ`, `DM9051_MH2030A_OWN_EXTI4_15_HANDLER`) — compile vs runtime selection consistency; `ENABLE_*` defaults to `USE_*`, single point of definition in `dm9051_hal_mh2030a_spi1.h`

**General embedded C concerns**:
- Buffer overflow: RX `header[4]` but `read_mem` writes `DM9051_RX_HEAD_SIZE` (4) bytes; `rx_len` computed from header must not exceed `DM9051_ETH_FRAME_MAX` (1514)
- `dm9051_core_close()` zeroes entire `dm9051_device_t` including hal pointer — any subsequent access is UAF
- Accessing `dev->runtime` fields without confirming `dev` is not NULL
- `volatile` on `interrupt_event` and `bus_busy` — is the compiler barrier sufficient for the MPU-less Cortex-M0?
- `dm9051_core_link_is_up()` returns 0 on any error — indistinguishable from link-down; caller must distinguish
- `dm9051_core_phy_read()` returns `0xFFFF` on error — not distinguishable from a legitimate PHY register value of 0xFFFF
- `dm9051_core_receive_ex()` error path: `rx_len` read in `dm9051_core_rx_header()` is only valid on success; line 1168 checks it after failure — potential use of uninitialized variable if `read_mem` inside `rx_header` fails
- `dm9051_mh2030a_irq_handler()` stores `dm9051_device_t *` in a static volatile pointer (`dm9051_mh2030a_irq_device`) without a critical section for the pointer assignment itself — concurrent attach/detach vs ISR race

## When to use me

- Before committing or merging changes to any file under `dm9051_driver/`
- When reviewing PRs or patches touching the DM9051 core, HAL binding, or port layer
- When investigating a bug in SPI communication, RX corruption, TX timeout, or link detection
- When onboarding new team members to review their first port/adapter contribution

## How I review

1. **Layer check first**: identify which layer(s) the diff touches. Verify no include/API dependency direction violation.
2. **Error path audit**: for every `return DM9051_ERR_*` or `return DM9051_HAL_ERR_*`, confirm `bus_release`, CS deselect, and critical section exit happen before the return.
3. **Null pointer scan**: every function taking `dm9051_device_t *`, `dm9051_hal_t *`, `dm9051_config_t *`, or buffer pointer must guard with `== 0` check.
4. **Timeout safety**: SPI/DMA/PHY write timeout loops must have a finite bound and not hang the MCU.
5. **Interrupt safety**: shared state (`interrupt_event`, `bus_busy`) accessed from both ISR and main loop — verify `volatile` + critical section pairing.
6. **Staged compatibility**: check that new code does not break existing legacy API users. No symbol removal without GitNexus impact analysis.
7. **Plan docs sync**: every API change must update the relevant `docs/plan/` doc (`HAL_CONTRACT.md` for HAL ops, `API_BOUNDARY.md` for layer dependencies, `STATE_MODEL.md` for runtime state, `CORE_API_PLAN.md` for core API changes).
8. **Suggest improvements with specific code examples, prioritize by severity**.
