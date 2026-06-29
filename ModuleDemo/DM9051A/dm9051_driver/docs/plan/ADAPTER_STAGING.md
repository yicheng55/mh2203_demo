# Adapter Staging Notes

This document records the current staging behavior for network-stack adapters.
The production uIP adapter remains unchanged.

## Current Staging Behavior

| Adapter | Function | Behavior |
| --- | --- | --- |
| uIP | `dm9051_uip_init(dev)` | Uses `dm9051_netif_device_is_valid(dev)` and returns `DM9051_OK` only after a core device has been attached. |
| uIP | `dm9051_uip_attach(dev)` | Attaches an already opened staged core device. Does not include MH2030A headers or open hardware directly. |
| uIP | `dm9051_uip_input(buf, len)` | Calls `dm9051_core_receive()` on the attached device. No uIP packet dispatch yet. |
| uIP | `dm9051_uip_output(buf, len)` | Calls `dm9051_core_send()` on the attached device. No uIP packet queue integration yet. |
| uIP | `dm9051_uip_poll()` | No-op. |
| uIP | `dm9051_uip_target_mode()` | Returns the active input mode string from `dm9051_uip_interrupt_mode()`: `"polling"` (interrupt off / no device), `"interrupt"`, or `"interrupt+clkout"`. |
| lwIP | (removed — `dm9051_lwip.c/h` deleted in favor of direct `ethernetif.c/h` API) | — |

`dm9051_netif_device_is_valid()` is intentionally permissive at this stage. It
checks only that the device pointer exists; MAC and IP policy should be copied
from the production adapter only after the core/HAL init path is ready.

## Why Not Call uIP/lwIP Yet?

The staged core can now perform probe, init, RX smoke, and TX smoke through the
MH2030A polling HAL. The uIP adapter still does not call uIP process functions
or include uIP headers. It only exposes a stack-neutral bridge to the opened core
device so the next migration step can connect the production uIP packet loop
deliberately.

## Future Migration

1. Bind `dm9051_device_t` and `dm9051_hal_t` from the example or board layer.
2. Decide whether adapter init should own `dm9051_core_open()` or keep hardware
   opening in the board/example layer.
3. Move RX/TX stack handling from the production adapter into the staged adapter.
4. Keep MH2030A headers out of adapter source files.
