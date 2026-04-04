# PIO PS/2 Converter

An RP2040-based PS/2 keyboard and mouse converter using the Programmable I/O (PIO) hardware for bit-level protocol handling. Supports combined keyboard/mouse devices like the IBM Model M4-1 and IBM Model M13.

## Key Feature: PIO-Based Reply Tagging

Unlike software-based PS/2 implementations that either block during command exchanges or use heuristic state tracking, this converter uses the RP2040's PIO state machine to **tag device replies at the hardware level**.

When the host sends a command to the keyboard, the PIO pre-loads a flag bit into the ISR before receiving the device's response. Spontaneous scan codes (key presses) don't carry this flag. The ISR handler routes tagged frames to a reply buffer and untagged frames to a message buffer — two separate ring buffers that fill concurrently.

**Why this matters:**
- No keystrokes are lost during command exchanges (unlike blocking approaches)
- No race conditions between scan codes and command ACKs (unlike software state machines)
- The distinction happens in PIO hardware, not in a timing-sensitive software heuristic
- Frame validation ensures the flag bit is only trusted when start/stop/parity are correct

This is functionally equivalent to what the original Intel 8042 PS/2 controller did in dedicated silicon — but implemented in the RP2040's programmable I/O.

## Hardware

### Pin Assignment

| Pin | Function |
|-----|----------|
| GP20 | PS/2 Data |
| GP21 | PS/2 Clock |
| GP16 | Keyboard power control |

**Note:** Clock pin must be Data pin + 1 (PIO requirement for paired pin access).

### Building a Converter

You need:
- RP2040 board (Raspberry Pi Pico, Waveshare RP2040-Zero, or similar)
- Bidirectional level shifter (e.g., BSS138-based) — RP2040 GPIOs are 3.3V, PS/2 is 5V
- PS/2 connector (Mini-DIN 6 or SDL for IBM keyboards)

There is also a dedicated PCB designed as a USB-stick-sized converter with SDL cable — see the `rp2040_converter_cable` KiCad project.

```
PS/2 Keyboard (5V) <---> Level Shifter <---> RP2040 (3.3V)
    CLK (pin 5)              HV/LV            GP21
    DATA (pin 1)             HV/LV            GP20
    VCC (pin 4)                               GP16 (switched) / 5V
    GND (pin 3)                               GND
```

## Supported Devices

- IBM Model M (AT/PS2 protocol)
- IBM Model M4-1 (combined keyboard + TrackPoint mouse)
- IBM Model M13 (combined keyboard + TrackPoint mouse)
- Any PS/2 standard-compliant keyboard
- PS/2 mice (remote mode)

## Building

```bash
qmk compile -kb converter/pio_ps2_converter -km default    # default keymap
qmk compile -kb converter/pio_ps2_converter -km marfrit    # marfrit keymap
qmk compile -kb converter/pio_ps2_converter -km debug      # debug output via HID console
```

Flash by holding BOOTSEL on the RP2040, connecting USB, and copying the `.uf2` file to the mass storage device.

## Keymaps

- `default` — standard layout
- `marfrit` — custom layout with debug output enabled
- `debug` — verbose debug logging via QMK console

## Technical Details

### PIO State Machine

The PS/2 protocol is handled entirely in PIO assembly (25 instructions). The state machine handles both device-to-host (scan codes) and host-to-device (commands) communication, with the reply tagging mechanism described above.

### Frame Format

```
PIO RX FIFO word (32 bits):
  [31]    stop bit
  [30]    parity bit
  [29:22] data byte (8 bits, LSB first)
  [21]    start bit (should be 0)
  [20]    repl_bit (1 = reply to host command, 0 = spontaneous data)
  [19:0]  unused
```

### Error Handling

Frames are validated before the reply flag is trusted:
- Start bit must be 0
- Stop bit must be 1
- Parity must match data

If any check fails, the frame is discarded entirely — preventing misrouted data from corrupted frames (noise, hot-plug, timing violations).

## Debug Resources

The `rp2040_error.sr` and `rp2040_error2.sr` files are sigrok/PulseView logic analyzer captures of PS/2 communication, useful for debugging protocol issues.

## Mouse Mode Toggle

The converter supports runtime switching between PS/2 Remote Mode and Stream Mode.

### Default Mode

Remote Mode is the default, configured in `config.h`:

```c
#define PS2_MOUSE_USE_REMOTE_MODE
```

To default to Stream Mode instead, remove this define (or comment it out). The runtime toggle works regardless of the default.

### Runtime Toggle

Assign `QK_USER_0` to a key in your keymap to toggle between modes:

```c
[_FN] = LAYOUT(
    ..., QK_USER_0, ...
)
```

Pressing the key switches the mouse between Remote and Stream mode immediately. Debug output (if enabled) will print the current mode.

### When to Use Each Mode

| Mode | Best For | Caveat |
|------|----------|--------|
| **Remote** (default) | IBM TrackPoint (M4, M13), combined keyboard/mouse | Slightly higher latency (~10ms polling) |
| **Stream** | Standard PS/2 mice, gaming mice | May cause click glitches on TrackPoint devices |
