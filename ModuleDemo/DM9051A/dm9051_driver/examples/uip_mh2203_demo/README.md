# MH2203 DM9051 uIP Demo

Smoke test for the MH2203 DM9051 port using the staged uIP adapter.

## Expected output

On success, the demo prints:

```text
VID=0x0A46 PID=0x9051 CHIPR=0xXX
DM9051 found and opened successfully!
```

## Usage

Build the uIP demo target with `MH2203_UIP_PORT` defined and include path
pointing to `../../port/mh2203`.
