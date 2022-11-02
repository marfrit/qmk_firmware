# MCU name
MCU = atmega32u4

# Bootloader selection
BOOTLOADER = caterina

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
MOUSEKEY_ENABLE = no        # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = no         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = no            # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = no        # Enable keyboard RGB underglow
AUDIO_ENABLE = no           # Audio output
USB_HID_ENABLE = yes
CUSTOM_MATRIX = yes

POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom

SRC += custom_matrix.cpp hidboot.cpp

DEFAULT_FOLDER = converter/usb_usb/hasu

LAYOUTS = fullsize_ansi fullsize_iso
