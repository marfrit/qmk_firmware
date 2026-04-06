MCU = linux

CUSTOM_MATRIX = yes
SRC += matrix.c

EEPROM_DRIVER = custom
SRC += platforms/linux/eeprom.c

# Features
MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes
NKRO_ENABLE = no
RAW_ENABLE = yes
VIA_ENABLE = yes
