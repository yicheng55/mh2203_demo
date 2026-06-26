# Core API Plan

This document defines the staged core API direction. It is documentation only;
the existing production driver remains unchanged.

## Goals

- Keep current legacy entry points available while the Keil/uIP targets are
  migrated.
- Add a context-based API that can eventually support explicit HAL binding and
  more than one device instance.
- Avoid exposing uIP, lwIP, or MH2030A details from `core/inc`.

## Context-Based API

Declared in `core/inc/dm9051_core.h`:

```c
struct dm9051_hal;

void dm9051_core_default_config(dm9051_config_t *config);
int dm9051_core_config_is_valid(const dm9051_config_t *config);
int dm9051_netif_device_is_valid(const dm9051_netif_device_t *dev);

int dm9051_core_open(dm9051_device_t *dev,
                     const dm9051_config_t *config,
                     struct dm9051_hal *hal);
int dm9051_core_close(dm9051_device_t *dev);

uint16_t dm9051_core_receive(dm9051_device_t *dev,
                             uint8_t *buf,
                             uint16_t buf_len);
int dm9051_core_send(dm9051_device_t *dev,
                     const uint8_t *buf,
                     uint16_t len);

uint16_t dm9051_core_phy_read(dm9051_device_t *dev, uint16_t reg);
int dm9051_core_phy_write(dm9051_device_t *dev, uint16_t reg, uint16_t value);

void dm9051_core_interrupt_set(dm9051_device_t *dev, uint32_t irq_line);
int dm9051_core_interrupt_take(dm9051_device_t *dev);
void dm9051_core_interrupt_reset(dm9051_device_t *dev);

const uint8_t *dm9051_core_mac(const dm9051_device_t *dev);
int dm9051_core_device_found(const dm9051_device_t *dev);
uint16_t dm9051_core_vendor_id(const dm9051_device_t *dev);
uint16_t dm9051_core_product_id(const dm9051_device_t *dev);
uint8_t dm9051_core_chip_revision(const dm9051_device_t *dev);
```

`struct dm9051_hal` is forward-declared to keep `dm9051_core.h` independent
from `hal/inc/dm9051_hal.h`.

## Legacy Compatibility Mapping

| Current API | Future wrapper behavior |
| --- | --- |
| `dm9051_conf()` | Configure/bind the default static `dm9051_device_t`. |
| `dm9051_init(adr)` | Call `dm9051_core_open(&default_dev, config, hal)` and return active MAC. |
| `dm9051_rx(buf, len)` | Call `dm9051_core_receive(&default_dev, buf, len)`. |
| `dm9051_tx(buf, len)` | Call `dm9051_core_send(&default_dev, buf, len)`. |
| `cspi_phy_read(reg)` | Call `dm9051_core_phy_read(&default_dev, reg)`. |
| `dm9051_interrupt_set(line)` | Call `dm9051_core_interrupt_set(&default_dev, line)`. |
| `dm9051_interrupt_get()` | Call `dm9051_core_interrupt_take(&default_dev)`. |
| `dm9051_interrupt_reset()` | Call `dm9051_core_interrupt_reset(&default_dev)`. |

## Migration Rule

Start with one default static device in `dm9051_core.c` to preserve current
single-instance behavior. After existing uIP targets pass, adapters can be
moved to the context API directly.

## Current Staging Implementation

The context API currently implements only safe state handling:

- `dm9051_core_default_config()` clears a config object and selects polling
  mode as the default input mode.
- `dm9051_core_config_is_valid()` accepts only known input modes and leaves MAC
  content policy to the later production-compatible init path.
- `dm9051_netif_device_is_valid()` currently checks only pointer validity so
  adapters do not prematurely reject DHCP or legacy zero-address staging cases.
- `dm9051_core_open()` validates config and the minimum HAL binding set, copies
  config/HAL/MAC into `dm9051_device_t`, resets the device if the HAL provides
  a reset hook, probes `VIDL/VIDH/PIDL/PIDH/CHIPR`, and returns `DM9051_OK`
  only when the staged ID probe succeeds.
- `dm9051_core_close()` clears the device context.
- `dm9051_core_interrupt_set/take/reset()` operate on the staged interrupt event
  flag only.
- `dm9051_core_mac()` returns the staged MAC buffer.
- `dm9051_core_device_found()`, `dm9051_core_vendor_id()`,
  `dm9051_core_product_id()`, and `dm9051_core_chip_revision()` expose the
  staged probe result.
- RX/TX/PHY functions return neutral not-ready values until the production core
  logic is copied.

## Impact Requirements

Before changing any production implementation for the legacy APIs, run GitNexus
impact analysis for that symbol. Treat `dm9051_rx`, `dm9051_tx`, and
`dm9051_interrupt_*` as HIGH practical risk even if graph impact appears low,
because they are Ethernet hot-path and IRQ-path behavior.
