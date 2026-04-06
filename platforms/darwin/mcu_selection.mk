ifeq ($(MCU),darwin)
    PLATFORM_KEY = darwin
    BOOTLOADER_TYPE = none
    FIRMWARE_FORMAT = macho
endif
