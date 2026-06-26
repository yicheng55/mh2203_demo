# State Model Notes

This document maps the current DM9051 global state to the staged runtime model.
It is documentation only; the production driver remains unchanged.

## Current Production State

| Current symbol | Current location | Purpose |
| --- | --- | --- |
| `dm9051_def_inf` | `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_internal.h` | Default static driver configuration. |
| `dm9051_inf` | `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_internal.h` | Pointer to active configuration. |
| `dm9051_interrupt_event` | `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_internal.h` | ISR-to-poll event flag. |
| `ext_int_pin` | `drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_internal.h` | Active EXTI line. |

## Staged Model

The staging type is defined in `core/inc/dm9051_types.h`:

```c
typedef struct dm9051_runtime {
    dm9051_config_t config;
    dm9051_mac_t current_mac;
    uint16_t vendor_id;
    uint16_t product_id;
    uint32_t irq_line;
    volatile uint8_t interrupt_event;
    uint8_t chip_revision;
    uint8_t device_found;
} dm9051_runtime_t;

typedef struct dm9051_device {
    dm9051_runtime_t runtime;
    void *hal;
} dm9051_device_t;
```

`void *hal` is intentionally opaque in `dm9051_types.h` so core types do not
depend on `hal/inc/dm9051_hal.h`. The implementation in `dm9051_core.c` can
cast it to `dm9051_hal_t *` after including the HAL header.

## Migration Rule

During compatibility migration, keep a single default static `dm9051_device_t`
inside `dm9051_core.c` to preserve current single-instance behavior. Do not
introduce multi-instance behavior until the existing uIP targets pass with the
default instance.

## State Ownership

| Staged field | Owns |
| --- | --- |
| `runtime.config` | checksum options, RX filter mode, flow control, force-stop policy, input mode |
| `runtime.current_mac` | active programmed MAC |
| `runtime.vendor_id` | staged probe result from `VIDL/VIDH` |
| `runtime.product_id` | staged probe result from `PIDL/PIDH` |
| `runtime.irq_line` | EXTI line currently associated with DM9051 |
| `runtime.interrupt_event` | ISR-to-main-loop event flag |
| `runtime.chip_revision` | staged probe result from `CHIPR` |
| `runtime.device_found` | probe result from chip ID / init |
| `hal` | platform bus/IRQ/delay operations |
