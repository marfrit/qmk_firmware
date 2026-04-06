MCU = windows

CUSTOM_MATRIX = yes
SRC += matrix.c

EEPROM_DRIVER = custom
SRC += platforms/windows/eeprom.c
SRC += platforms/windows/weak_stubs.c

MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes
NKRO_ENABLE = no
