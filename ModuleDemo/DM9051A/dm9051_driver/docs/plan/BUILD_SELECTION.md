# Build Selection Notes

This file records the current Keil target file-selection behavior before any
DM9051 driver refactor is wired into the build.

No existing target is changed by this staging document.

## Current Keil Targets

Source inspected:

- `ModuleDemo/DM9051A/USER/DM9051A.uvprojx`

Transport source files:

- `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_spi.c`
- `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_spi_dma.c`
- `ModuleDemo/DM9051A/port/mh2030a/mh2030a_dm9051_int.c`

`IncludeInBuild=default(1)` means the file has no per-file override in the
target, so Keil treats it as included.

| Target | Defines | Polling SPI | SPI DMA | INT file | Intended mode |
| --- | --- | --- | --- | --- | --- |
| `DM9051A` | `USE_STDPERIPH_DRIVER` | default(1) | default(1) | default(1) | Legacy probe; needs verification. |
| `DM9051A_SPI_DMA` | `USE_STDPERIPH_DRIVER` | 0 | default(1) | default(1) | Legacy DMA probe. |
| `MH2030A_DM9051_uIP` | `USE_STDPERIPH_DRIVER,MH2030A_UIP_PORT` | default(1) | 0 | default(1) | uIP polling. |
| `MH2030A_DM9051_uIP_dma` | `USE_STDPERIPH_DRIVER,MH2030A_UIP_PORT,MH2030A_DM9051_SPI_DMA` | 0 | default(1) | 1 | uIP SPI DMA. |
| `MH2030A_DM9051_uIP_int` | `USE_STDPERIPH_DRIVER,MH2030A_UIP_PORT,DMPLUG_INT` | default(1) | 0 | default(1) | uIP interrupt + polling SPI. |
| `MH2203_DM9051_uIP` | `USE_STDPERIPH_DRIVER,MH2203_UIP_PORT` | port/mh2203 polling SPI | 0 | port/mh2203 IRQ(off) | MH2203 uIP polling. |
| `MH2203_DM9051_uIP_dma` | `USE_STDPERIPH_DRIVER,MH2203_UIP_PORT,MH2203_DM9051_SPI_DMA` | 0 | port/mh2203 DMA SPI | port/mh2203 IRQ(off) | MH2203 uIP DMA. |
| `MH2203_DM9051_uIP_int` | `USE_STDPERIPH_DRIVER,MH2203_UIP_PORT,MH2203_DM9051_IRQ_EXTI` | default(1) | 0 | default(1) | MH2203 uIP interrupt. |

## Observations

- `mh2030a_dm9051_spi.c` and `mh2030a_dm9051_spi_dma.c` both define the same
  flat HAL symbols such as `hal_spi_initialize`, `hal_read_reg`,
  `hal_write_reg`, `hal_read_mem`, and `hal_write_mem`.
- Those two files must remain mutually exclusive in any target that links the
  current flat HAL ABI.
- `mh2030a_dm9051_int.c` is safe to include in polling/DMA targets because it
  provides no-op interrupt functions when `DMPLUG_INT` is not defined.
- The legacy `DM9051A` target currently appears to include both polling and DMA
  SPI transport files by default. Do not change this during staging; verify the
  actual Keil build behavior before using it as a refactor baseline.

## Future Portable Build Rule

The staged driver should replace per-file Keil toggles with explicit transport
selection:

```text
DM9051_MH2030A_TRANSPORT=polling
DM9051_MH2030A_TRANSPORT=dma
```

Interrupt support should be orthogonal:

```text
DM9051_MH2030A_IRQ=off
DM9051_MH2030A_IRQ=on
```

That gives these valid combinations:

| Transport | IRQ | Expected sources |
| --- | --- | --- |
| polling | off | polling SPI + no-op IRQ |
| polling | on | polling SPI + EXTI IRQ |
| dma | off | DMA SPI + no-op IRQ |
| dma | on | DMA SPI + EXTI IRQ |

The current uIP interrupt target uses polling SPI plus EXTI IRQ.

