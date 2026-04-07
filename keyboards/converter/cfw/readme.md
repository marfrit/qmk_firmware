# CFW — Converter Framework for QMK

Multi-platform keyboard converter architecture that supports vintage
keyboards (AT/PS2, Terminal, ADB, IBM 5291) via a pluggable 3-layer design:

- **Converter layer** — protocol handling and scancode translation (UNIMAP)
- **Wire layer** — electrical interface abstraction (clock/data, muxstrobe)
- **Platform layer** — MCU-specific I/O (RP2040 PIO+DMA, STM32 PAL, AVR ISR)

## Supported converters

| Converter | Protocols | Keyboards |
|-----------|-----------|-----------|
| IBM PC    | AT, PS/2, Terminal | Model M, Model F, various AT/PS2 keyboards |
| ADB       | Apple Desktop Bus | Apple Extended Keyboard, AEKII, M0116 |
| IBM 5291  | Muxstrobe (capsense) | IBM 5291 Displaywriter "Bigfoot" |

## Supported platforms

| Platform | MCU | Features |
|----------|-----|----------|
| RP2040   | Pi Pico / KB2040 | PIO + DMA + dual-core + SIO FIFO |
| STM32    | Blue/Black Pill, Proton C | ChibiOS PAL callback, true open-drain |
| AVR      | Pro Micro / ATmega32U4 | ISR-driven, 32-byte ring buffer |

## Building

```
qmk compile -kb converter/cfw -km default
```

See subdirectories under `platforms/` for MCU-specific `keyboard.json` overrides.
