# Core Split Map

This document maps the current monolithic DM9051 core implementation to the
future reusable-driver layout. It is documentation only; no production source is
changed by this staging step.

Current source of truth:

- `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_beta.c`
- `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_internal.h`
- `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051.h`

## Target Core Files

Keep the core source layout intentionally small. RX, TX, PHY, and IRQ behavior
should first be organized as internal sections inside `dm9051_core.c`, not split
into separate compilation units.

```text
core/inc/
  dm9051_core.h
  dm9051_regs.h
  dm9051_types.h

core/src/
  dm9051_core.c       public API, init orchestration, shared state
  dm9051_debug.c      diagnostics, logging, optional debug helpers
```

Do not add more core source files until the two-file layout becomes a real
maintenance bottleneck.

## Public API Surface

These functions are currently declared in `dm9051.h` and should remain
compatibility-preserving until callers are migrated:

| Function | Current role | Future owner |
| --- | --- | --- |
| `dm9051_conf` | Configure SPI and optional IRQ through flat HAL | `dm9051_core.c` |
| `dm9051_init` | Initialize chip, validate MAC, start RX | `dm9051_core.c` |
| `dm9051_rx` | Receive one frame from RX FIFO | `dm9051_core.c` |
| `dm9051_tx` | Transmit one Ethernet frame | `dm9051_core.c` |
| `cspi_phy_read` | Public PHY register read helper | `dm9051_core.c` |
| `dm9051_interrupt_set` | ISR-facing interrupt event set | `dm9051_core.c` |
| `dm9051_interrupt_get` | Poll interrupt event state | `dm9051_core.c` |
| `dm9051_interrupt_reset` | Clear interrupt event state | `dm9051_core.c` |

## Initialization And Configuration

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `dm9051_conf` | Board/HAL configure entry point | `dm9051_core.c` compatibility wrapper |
| `cspi_core_reset` | Reset DM9051 core and soft defaults | `dm9051_core.c` |
| `cspi_soft_default` | Program default chip registers | `dm9051_core.c` |
| `cspi_core_start1` | Enable RX path and IRQ | `dm9051_core.c` |
| `env_chip_id_and_ticks` | Probe chip ID with heartbeat/timing output | `dm9051_core.c` or `dm9051_debug.c` |
| `check_force_stop` | Optional stop-on-not-found policy | `dm9051_core.c` |
| `cspi_get_chipid` | Read product/chip ID | `dm9051_core.c` |
| `cspi_get_control_status` | Read NCR/NSR status | `dm9051_core.c` |

## MAC Address Handling

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `is_zero_ether_addr` | Check zero MAC | `dm9051_core.c` |
| `validate_macaddr` | Validate caller-provided MAC | `dm9051_core.c` |
| `cspi_macaddr_process` | Choose/program MAC address | `dm9051_core.c` |
| `cspi_get_par` | Read programmed MAC | `dm9051_core.c` |
| `cspi_set_par` | Program MAC | `dm9051_core.c` |

## Register And PHY Helpers

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `cspi_write_regs` | Sequential register write | `dm9051_core.c` or private reg helper |
| `cspi_read_regs` | Sequential register read | `dm9051_core.c` or private reg helper |
| `cspi_phy_read` | PHY read sequence | `dm9051_core.c` |
| `cspi_phy_write` | PHY write sequence | `dm9051_core.c` |
| `cspi_phycore_on` | Power on PHY | `dm9051_core.c` |

## Interrupt And Receive Enable

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `conf_ext_line` | Store EXTI line | `dm9051_core.c` |
| `dm9051_isr_enab` | Acknowledge ISR | `dm9051_core.c` |
| `dm9051_imr_disab` | Disable chip interrupt mask | `dm9051_core.c` |
| `dm9051_imr_enab` | Enable chip interrupt mask by mode | `dm9051_core.c` |
| `cspi_set_imr` | Program IMR | `dm9051_core.c` |
| `cspi_disble_irq` | Disable chip IRQ | `dm9051_core.c` |
| `cspi_enable_irq` | Enable chip IRQ | `dm9051_core.c` |
| `cspi_set_recv` | Enable receive path | `dm9051_core.c` |
| `cspi_set_mar` | Program multicast filter | `dm9051_core.c` |
| `cspi_set_fcr` | Program flow control | `dm9051_core.c` |
| `cspi_set_rcr` | Program receive control | `dm9051_core.c` |

## RX Path

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `dm9051_rx` | Public receive entry point | `dm9051_core.c` |
| `cspi_rx_ready` | Check packet-ready byte | `dm9051_core.c` |
| `cspi_rx_head` | Read/validate RX header | `dm9051_core.c` |
| `rx_head_takelen` | Extract status and length | `dm9051_core.c` |
| `cspi_rx_read` | Read RX payload | `dm9051_core.c` |
| `cspi_rx_discard` | Discard malformed frame | `dm9051_core.c` |
| `cspi_get_rwpa` | Read write pointer | `dm9051_core.c` |
| `cspi_get_mrrl` | Read memory read pointer | `dm9051_core.c` |
| `dm9051_read_rx_pointers` | Read RX diagnostic pointers | `dm9051_debug.c` |
| `rx_pointers_equ` | Compare RX pointers | `dm9051_core.c` |
| `env_evaluate_rxb` | RX ready-byte diagnostic | `dm9051_debug.c` |
| `env_evaluate_rxb_zero` | RX zero-byte diagnostic | `dm9051_debug.c` |
| `dm9051_show_rxbstatistic` | RX diagnostic histogram | `dm9051_debug.c` |
| `ret_fire_time` | RX diagnostic helper | `dm9051_debug.c` |

## TX Path

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `dm9051_tx` | Public transmit entry point | `dm9051_core.c` |
| `cspi_tx_len` | Program TX length registers | `dm9051_core.c` |
| `cspi_tx_packet_len` | Validate/program TX frame length | `dm9051_core.c` |
| `cspi_tx_write` | Write TX FIFO payload | `dm9051_core.c` |
| `cspi_tx_req` | Trigger TX request | `dm9051_core.c` |

## Status, Timing, Debug

| Function | Responsibility | Suggested future owner |
| --- | --- | --- |
| `dm9051_status` | Read status registers | `dm9051_debug.c` or public diagnostics |
| `dm9051_read_regs_info` | Read register snapshot | `dm9051_debug.c` |
| `hal_active_interrupt_mode` | Query selected input mode | `dm9051_core.c`, eventually config-backed |
| `davicom_haltickcount` | Driver heartbeat/tick shim | replace with HAL time policy |
| `ctick_delay_us` | Delay shim | replace with HAL delay op |
| `ctick_delay_ms` | Delay shim | replace with HAL delay op |
| `env_err_rsthdlr3` | Error print/reset handler | `dm9051_debug.c` |

## Extraction Order

1. Copy register constants into `dm9051_regs.h` and keep production constants
   unchanged.
2. Move pure types/config into `dm9051_types.h` as staging-only definitions.
3. Introduce `dm9051_hal_t` use behind compatibility wrappers.
4. Copy the production core into `dm9051_core.c` and organize it into internal
   sections: public API, init/config, MAC, reg/PHY, IRQ, RX, TX.
5. Move diagnostics-only helpers into `dm9051_debug.c` after confirming they are
   not required by the hot RX/TX path.
6. Only then consider switching a Keil target to staged source files.

## HIGH Risk Areas

- `dm9051_rx` and its helper chain: packet framing and FIFO pointer handling.
- `dm9051_tx` and TX request ordering.
- `hal_active_interrupt_mode` / IMR selection: polling, DMA, and interrupt mode
  behavior are coupled through compile-time selection today.
- `dm9051_internal.h` global state: splitting into more than two source files
  before state ownership is clarified can create duplicate definitions or hidden
  shared state.
