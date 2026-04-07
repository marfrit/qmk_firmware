# QAP'LA — QMK As Process, Linux Adaptation

QAP'LA runs QMK firmware as a userspace process. It grabs your physical
keyboard via evdev, processes keystrokes through QMK (layers, macros,
tap-dance, combos — the full feature set), and outputs via a virtual
keyboard. Your keyboard becomes fully programmable without flashing
any firmware.

Vial GUI support is included: connect Vial to the virtual UHID device
and remap your keyboard live.

```
┌──────────────┐     ┌─────────┐     ┌──────────────┐
│ Physical KB   │────▶│  QAP'LA │────▶│ Virtual KB    │
│ (evdev grab)  │     │  (QMK)  │     │ (uinput)      │
└──────────────┘     └────┬────┘     └──────────────┘
                          │
                     ┌────▼────┐
                     │  Vial   │
                     │ (UHID)  │
                     └─────────┘
```

## Quick Start

```sh
# Build
qmk compile -kb qapla -km default

# Run (auto-detects keyboards)
sudo ./qapla_default.elf

# Run with a specific device
sudo QAPLA_EVDEV=/dev/input/event3 ./qapla_default.elf

# Stop: Ctrl+C or kill — keyboard is automatically ungrabbed
```

## Building

QAP'LA uses the standard QMK build system with `MCU = linux`:

```sh
# Set up QMK (if not already)
qmk setup

# Compile
qmk compile -kb qapla -km default

# The output is an ELF binary: qapla_default.elf
```

### Cross-compilation

QAP'LA targets aarch64 and x86_64 natively. For cross-compilation,
set `CC` in `platforms/linux/platform.mk` or override on the command line:

```sh
make qapla:default CC=aarch64-linux-gnu-gcc
```

## Running

### Permissions

QAP'LA needs access to:
- `/dev/input/event*` (read) — to grab the physical keyboard
- `/dev/uinput` (write) — to create the virtual keyboard
- `/dev/uhid` (read/write) — for Vial GUI support

Run as root, or install the udev rules (see below).

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `QAPLA_EVDEV` | *(auto-detect)* | Comma-separated evdev device paths. Example: `/dev/input/event3,/dev/input/event4`. When unset, QAP'LA scans all `/dev/input/event*` devices and grabs those with `KEY_A` capability (i.e., keyboards). Supports multi-interface keyboards (up to 8 devices). |

### EEPROM (Persistent Storage)

QMK settings (keymaps, Vial config, lighting) persist in a file:

| Platform | Path |
|----------|------|
| Linux | `~/.config/qapla/eeprom.bin` |
| macOS | `~/.config/qapla/eeprom.bin` |
| Windows | `%APPDATA%\qapla\eeprom.bin` |

The directory is created automatically. Delete the file to reset to defaults.
The file is 16 KB (matching `EEPROM_SIZE` in `config.h`).

### Vial GUI

QAP'LA creates a UHID device that Vial recognizes automatically.
The device appears as "QAP'LA mughwI'" with serial `vial:f64c2b3c`.

1. Start QAP'LA
2. Open [Vial](https://get.vial.today/) (v0.7+)
3. Your keyboard appears — remap keys, configure layers, set macros

Changes made in Vial are saved to the EEPROM file and persist across restarts.

## Installation

### udev Rules (recommended)

Install the udev rules to run QAP'LA without root:

```sh
sudo cp extra/99-qapla.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Then add your user to the `input` group:

```sh
sudo usermod -aG input $USER
# Log out and back in for the group change to take effect
```

### systemd Service

Install as a user service that starts on login:

```sh
# Copy the binary somewhere permanent
sudo install -m 755 qapla_default.elf /usr/local/bin/qapla

# Install the service file
mkdir -p ~/.config/systemd/user
cp extra/qapla.service ~/.config/systemd/user/

# Enable and start
systemctl --user daemon-reload
systemctl --user enable --now qapla
```

Check status:

```sh
systemctl --user status qapla
journalctl --user -u qapla -f
```

To specify a device, edit the service and add to the `Environment` line:

```sh
systemctl --user edit qapla
# Add: Environment=QAPLA_EVDEV=/dev/input/event3
```

## Features

| Feature | Linux | Windows | macOS |
|---------|:-----:|:-------:|:-----:|
| Key remapping (layers, macros, etc.) | ✓ | ✓ | ✓ |
| Mouse keys | ✓ | ✓ | ✗ |
| Vial GUI | ✓ | ✗ | ✗ |
| Multi-device grab | ✓ (up to 8) | N/A | ✓ |
| Consumer/system keys | ✗ | ✗ | ✗ |
| NKRO | ✗ | ✗ | ✗ |

## Other Platforms

### Windows

```sh
# Cross-compile from Linux
qmk compile -kb qapla_win -km default
# Produces: qapla_win_default.exe
```

Uses low-level keyboard hooks (`WH_KEYBOARD_LL`) for input and `SendInput`
for output. No driver installation needed. May need to run as Administrator
to intercept elevated windows. Anti-cheat software may block keyboard hooks.

EEPROM: `%APPDATA%\qapla\eeprom.bin`

### macOS

Uses either the Karabiner DriverKit backend (true device seize, recommended)
or CGEventTap (no driver needed, requires Input Monitoring permission).
See `tmk_core/protocol/darwin/darwin.c` for Karabiner installation instructions.

EEPROM: `~/.config/qapla/eeprom.bin`

## Known Limitations

- **Consumer/system keys** (volume, brightness, media) are not forwarded yet
- **NKRO** sends are stubbed — 6KRO works fine for normal use
- **LED feedback** (Caps/Num/Scroll Lock) is not forwarded to the physical keyboard
- The Vial serial `vial:f64c2b3c` is hardcoded — if you run multiple instances,
  Vial will see them as the same device

## License

GPL-2.0-or-later (same as QMK)
