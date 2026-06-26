# API Boundary Notes

This document defines the intended dependency direction for the staged DM9051
driver. It is documentation only; the existing Keil targets still use the
current production files.

## Allowed Dependencies

```text
application
  -> adapters/uip or adapters/lwip
  -> core/inc
  -> hal/inc
  -> ports/mh2030a
  -> MH20xx peripheral library
```

The dependency arrow points downward only. Lower layers must not include upper
layers.

## Layer Rules

| Layer | May include | Must not include |
| --- | --- | --- |
| `core/inc` | `stdint.h`, `dm9051_regs.h`, `dm9051_types.h` | uIP, lwIP, MH2030A, Keil target headers |
| `core/src` | `core/inc`, `hal/inc` | uIP, lwIP, MH2030A app config |
| `hal/inc` | `stdint.h` | DM9051 app code, uIP, lwIP, MH2030A concrete headers |
| `adapters/uip` | `core/inc`, uIP headers in implementation only | MH2030A SPI/GPIO/IRQ headers |
| `adapters/lwip` | `core/inc`, lwIP headers in implementation only | MH2030A SPI/GPIO/IRQ headers |
| `ports/mh2030a` | `hal/inc`, MH20xx peripheral headers, delay | uIP/lwIP application logic |
| `examples/*` | adapter headers, board config | core internals, register internals |

## Compatibility Names

Current production names:

- `dm9051_uip_adapter_init`
- `dm9051_uip_adapter_poll`
- `dm9051_netif_target_mode`

Staging names:

- `dm9051_uip_init`
- `dm9051_uip_poll`
- `dm9051_uip_target_mode`

Do not rename production symbols until callers are migrated behind wrappers and
GitNexus impact analysis has been run for each changed symbol.

## Adapter Ownership

The adapter owns stack-specific behavior:

- uIP: `uip_init`, `uip_input`, `uip_periodic`, ARP timer, `uip_buf`.
- lwIP: `struct netif`, `pbuf`, input/output callbacks.

The DM9051 core owns chip behavior:

- reset/init sequence
- register access policy
- PHY read/write
- RX FIFO framing
- TX FIFO framing
- interrupt event state

The port owns MCU behavior:

- SPI transfer
- GPIO chip select/reset
- EXTI/NVIC interrupt wiring
- delay and critical section primitives

