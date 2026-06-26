# MH2030A DM9051 Port

This directory will contain the MH2030A implementation of the portable DM9051
HAL interface.

## Files

| File | Role |
| --- | --- |
| `dm9051_hal_mh2030a_spi1.c/.h` | DM9051 HAL binding for MH2030A SPI1 polling, GPIO, delay, and transport selection. |
| `dm9051_hal_mh2030a_spi1_priv.h` | Private shared SPI1/pin definitions used by optional MH2030A port files. |
| `dm9051_hal_mh2030a_spi1_dma.c/.h` | Optional SPI1 DMA FIFO transfer implementation. |
| `dm9051_hal_mh2030a_int.c/.h` | Optional PF6 / EXTI line 6 interrupt implementation. |
| `mh2030a_platform.h` | Local MH2030A platform include shim for `mh20xx.h` and `delay.h`. |
| `mh2030a_board.c/.h` | MH2030A board bring-up helpers for clock, debug UART, and printf retargeting. |

## Port File Naming Plan

Use names that expose both the platform and the bus. The current staging
implementation keeps the default SPI1 polling path in the base file and puts
optional DMA / interrupt behavior in independently named files:

| File class | Responsibility |
| --- | --- |
| `dm9051_hal_mh2030a_spi1.c/.h` | SPI1 polling transfer, CS control, reset GPIO, and delay binding. |
| `dm9051_hal_mh2030a_spi1_dma.c/.h` | SPI1 DMA transfer implementation selected by `DM9051_MH2030A_TRANSPORT_DMA`. |
| `dm9051_hal_mh2030a_int.c/.h` | DM9051 INT pin / EXTI setup, enable, disable, and ISR handoff helpers. |

This mirrors the current production split under `ModuleDemo/DM9051A/port/mh2030a`
while making the SPI instance explicit for future ports that may use another
SPI peripheral.

## Current Pin Mapping

| Signal | Pin |
| --- | --- |
| CS | PA15 |
| SCK | PB3 |
| MISO | PB4 |
| MOSI | PB5 |
| INT | PF6 / EXTI line 6 |
| RST | PF7 |

## Current Build Modes

| Mode | Define | Current source |
| --- | --- | --- |
| Polling SPI | none | `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_spi.c` |
| SPI DMA | `MH2030A_DM9051_SPI_DMA` | `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_spi_dma.c` |
| Interrupt | `DMPLUG_INT` | `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_int.c` |

The current Keil targets select files with per-target file options. The future
portable build should make the transport selection explicit to avoid duplicate
`hal_*` symbols.

See `../../docs/BUILD_SELECTION.md` for the current target matrix.

## Port API

The staging header `dm9051_hal_mh2030a_spi1.h` defines an explicit config model:

- `dm9051_mh2030a_transport_t`: polling or DMA SPI.
- `dm9051_mh2030a_irq_mode_t`: IRQ off or EXTI IRQ.
- `dm9051_mh2030a_pins_t`: pin assignment for CS/SCK/MISO/MOSI/RST/INT.
- `dm9051_mh2030a_config_t`: transport, IRQ, pins, and timeout.

The staged implementation binds this config into `dm9051_hal_t` with
`dm9051_mh2030a_hal_bind()`. The legacy production code still uses the current
flat `hal_*` implementation.

Current staging implementation status:

- `dm9051_mh2030a_default_config()` sets a transport/IRQ/timeout default without
  touching hardware and records the current MH2030A DM9051 pin mapping.
- `dm9051_mh2030a_config_is_valid()` validates transport, IRQ mode, and timeout
  without requiring platform headers.
- `dm9051_mh2030a_transport_name()` and `dm9051_mh2030a_irq_name()` are usable
  string helpers.
- `dm9051_mh2030a_hal_bind()` validates parameters and copies config into an
  internal HAL context.
- Polling transport binds real SPI register/FIFO operations based on the
  current `mh2030a_dm9051_spi.c` behavior.
- DMA transport lives in `dm9051_hal_mh2030a_spi1_dma.c`; it keeps register
  access on byte polling and uses DMA for FIFO `read_mem` / `write_mem`, based
  on `mh2030a_dm9051_spi_dma.c`.
- EXTI IRQ mode lives in `dm9051_hal_mh2030a_int.c`; it configures PF6 / EXTI
  line 6 and provides
  `dm9051_mh2030a_irq_attach_device()` plus `dm9051_mh2030a_irq_handler()` for
  the instance-based core interrupt flag.
- Define `DM9051_MH2030A_OWN_EXTI4_15_HANDLER=0` if the application already
  owns `EXTI4_15_IRQHandler()` and call `dm9051_mh2030a_irq_handler()` from the
  shared handler.

## Keil Options

The `MH2030A_DM9051_uIP` target includes the optional DMA and interrupt source
files. Feature availability and smoke-demo runtime selection are controlled by
preprocessor defines:

| Define | Meaning |
| --- | --- |
| `DM9051_MH2030A_ENABLE_DMA=1` | Compile and link the SPI1 DMA transport file. |
| `DM9051_MH2030A_ENABLE_IRQ=1` | Compile and link the PF6 / EXTI interrupt file. |
| `DM9051_MH2030A_USE_DMA=1` | Select DMA transport in the staged smoke glue. |
| `DM9051_MH2030A_USE_IRQ=1` | Select core interrupt mode and EXTI IRQ mode in the staged smoke glue. |

When `DM9051_MH2030A_ENABLE_DMA` or `DM9051_MH2030A_ENABLE_IRQ` is not defined,
it defaults to the matching `DM9051_MH2030A_USE_*` value. Define `ENABLE_*`
explicitly only when the feature should be compiled in but not selected by the
current smoke-demo runtime mode, for example `ENABLE_DMA=1, USE_DMA=0`.

Default staged target values keep runtime behavior conservative:
`DM9051_MH2030A_USE_DMA=0` and `DM9051_MH2030A_USE_IRQ=0`.
