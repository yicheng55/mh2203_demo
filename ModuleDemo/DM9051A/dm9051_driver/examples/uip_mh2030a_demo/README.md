# uIP MH2030A Demo

Future location for a reusable DM9051 + uIP MH2030A example.

Current sources:

- `ModuleDemo/DM9051A/USER/main_uip_mh2030a.c`
- `ModuleDemo/DM9051A/port/uip/netconf_mh2030a.c`
- `ModuleDemo/DM9051A/port/uip/clock-arch.c`
- `apps/uip_dm9051_example_e1/uip_conf_inc/uip-conf.h`

This example should remain an application layer user of `adapters/uip`, not a
direct user of MH2030A SPI/GPIO functions.

## Current Staging Smoke Glue

`dm9051_uip_mh2030a_smoke.c` is an incremental validation helper for the new
`ModuleDemo/DM9051A/USER/DM9051A_uip.uvprojx` project.

It currently:

- Creates one static `dm9051_hal_t`.
- Creates one static `dm9051_device_t`.
- Binds the MH2030A polling HAL with `dm9051_mh2030a_hal_bind()`.
- Calls `dm9051_core_open()` to run the staged chip-ID probe.

`main_uip_mh2030a_smoke.c` is the current staging entry point selected by
`DM9051A_uip.uvprojx`. It initializes the board, opens the staged DM9051 core,
attaches the staged uIP adapter bridge, then performs hardware RX/TX smoke
using `dm9051_uip_input()` and `dm9051_uip_output()`.

It does not call uIP core functions. Keep it as the hardware regression test.

## Optional Staged uIP Loop

`main_uip_mh2030a_demo.c` is the next staged entry point. It keeps the same
DM9051 core/HAL open path, attaches the staged uIP adapter, initializes uIP and
ARP through `dm9051_uip_stack_init()`, then runs `dm9051_uip_stack_poll()`.

To build this optional demo, create a separate Keil target or replace the smoke
main with:

- `dm9051_driver/examples/uip_mh2030a_demo/main_uip_mh2030a_demo.c`
- `dm9051_driver/adapters/uip/dm9051_uip_stack.c`
- `dm9051_driver/ports/mh2030a/mh2030a_uip_clock.c`
- `middlewares/3rd_party/uip/src/uip.c`
- `middlewares/3rd_party/uip/src/uip_arp.c`
- `middlewares/3rd_party/uip/src/timer.c`

The current project-specific `uip-conf.h` includes `app_call.h`, so the uIP
application callback sources may also be required depending on enabled macros:

- `apps/uip_dm9051_example_e1/uip_app_src/app_call.c`
- selected TCP/UDP/webserver application sources referenced by `app_call.h`

Do not add the legacy `ModuleDemo/DM9051A/port/uip/clock-arch.c` to this staged
demo; use `dm9051_driver/ports/mh2030a/mh2030a_uip_clock.c` instead.
