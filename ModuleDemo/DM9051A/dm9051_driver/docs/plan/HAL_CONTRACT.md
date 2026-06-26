# HAL Contract

This document defines the staged DM9051 HAL operation semantics. It is
documentation only; the current production driver still uses the flat `hal_*`
API in `drivers/dm9051_edriver_v1.6.1a_beta/include/dm9051_hal_api.h`.

## Status Codes

| Code | Meaning |
| --- | --- |
| `DM9051_HAL_OK` | Operation completed successfully. |
| `DM9051_HAL_ERR` | General platform or bus failure. |
| `DM9051_HAL_ERR_TIMEOUT` | SPI/DMA/IRQ wait timed out. |
| `DM9051_HAL_ERR_PARAM` | Invalid argument, such as NULL buffer with nonzero length. |
| `DM9051_HAL_ERR_NOT_READY` | HAL shape is present, but the transport is not wired to hardware yet. |

## Register Access

```c
int (*read_reg)(void *ctx, uint8_t reg, uint8_t *val);
int (*write_reg)(void *ctx, uint8_t reg, uint8_t val);
```

Rules:

- `reg` is a DM9051 register address without SPI read/write opcode bits.
- The port implementation owns chip-select assertion, SPI opcode composition,
  transfer completion, and chip-select release.
- `read_reg` must return `DM9051_HAL_ERR_PARAM` when `val == NULL`.
- On read failure, `*val` should not be treated as valid by the caller.

## Minimum Core Binding Set

`dm9051_core_open()` treats these HAL operations as mandatory before accepting
a staged device binding:

- `read_reg`
- `write_reg`
- `read_mem`
- `write_mem`
- `delay_ms`
- `delay_us`

`reset`, IRQ hooks, and critical-section hooks remain optional at this stage.
This lets polling-only targets bind without an interrupt implementation while
still preventing an empty HAL vtable from reaching future RX/TX code.

Staging ports may provide a complete vtable whose bus operations return
`DM9051_HAL_ERR_NOT_READY`. This is useful for adapter/core build checks, but
must not be treated as a functional hardware transport.

## FIFO Access

```c
int (*read_mem)(void *ctx, uint8_t *buf, uint16_t len);
int (*write_mem)(void *ctx, const uint8_t *buf, uint16_t len);
```

Rules:

- `read_mem` performs a DM9051 RX FIFO read using the MRCMD sequence.
- `write_mem` performs a DM9051 TX FIFO write using the MWCMD sequence.
- `len == 0` is valid and should succeed without touching `buf`.
- `buf == NULL && len != 0` must return `DM9051_HAL_ERR_PARAM`.
- The port implementation may use polling or DMA internally, but the core must
  see identical behavior.

## Reset And Timing

```c
void (*reset)(void *ctx);
void (*delay_ms)(uint32_t ms);
void (*delay_us)(uint32_t us);
```

Rules:

- `reset` is optional. If NULL, core compatibility code may rely on the legacy
  behavior where reset happens during SPI initialization.
- `delay_ms` and `delay_us` should be non-NULL for production ports.
- Delay functions must be safe during early driver initialization.

## IRQ Control

```c
void (*irq_enable)(void *ctx);
void (*irq_disable)(void *ctx);
```

Rules:

- These functions gate the MCU interrupt channel connected to the DM9051 INT
  pin.
- They do not directly modify DM9051 IMR/ISR registers; chip interrupt mask
  policy stays in the core.
- In polling-only builds these hooks may be NULL or no-op.

## Critical Sections

```c
uint32_t (*enter_critical)(void *ctx);
void (*exit_critical)(void *ctx, uint32_t state);
```

Rules:

- These hooks are optional until shared state is moved behind `dm9051_device_t`.
- `enter_critical` returns a platform state token.
- `exit_critical` receives exactly the token returned by `enter_critical`.
- On Cortex-M, the state token can be PRIMASK.

## MH2030A Mapping

The staged MH2030A port should map:

| HAL op | Current production source |
| --- | --- |
| `read_reg`, `write_reg`, `read_mem`, `write_mem` | `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_spi.c` or `mh2030a_dm9051_spi_dma.c` |
| `reset`, `delay_ms`, `delay_us` | `hal_mh2030a.h` / `delay.h` and current SPI init reset sequence |
| `irq_enable`, `irq_disable` | `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_int.c` |
| `enter_critical`, `exit_critical` | CMSIS PRIMASK wrappers |

Current staging status:

- MH2030A polling transport is wired to the vtable.
- MH2030A DMA transport still returns `DM9051_HAL_ERR_NOT_READY`.
- IRQ and critical-section hooks are still no-op/NULL in the staged polling
  binding.
