LTO_ENABLE       = yes
BOOTMAGIC_ENABLE = no        # Virtual DIP switch configuration(+1000)
EXTRAKEY_ENABLE  = yes       # Audio control and System control(+450)
BLUETOOTH_ENABLE = no        # Enable Bluetooth with the Adafruit EZ-Key HID

#----------------------------------------------------------------------------
# make gergo:marfrit:dfu
# Make sure you have dfu-programmer installed!
#----------------------------------------------------------------------------
# Firmware options
BALLER = no 			# Enable to ball out
BALLSTEP = 20  			# Multiple in px to move, multiplied by layer number
SCROLLSTEP = 1 			# Lines to scroll with ball
MOUSEKEY_ENABLE = yes  		# Mouse keys, needed for baller

#Debug options
VERBOSE 		       = yes
DEBUG_MATRIX_SCAN_RATE = no
DEBUG_BALLER 		   = no
DEBUG_MATRIX		   = yes

# A bunch of stuff that you shouldn't touch unless you
# know what you're doing.
#
# No touchy, capiche?
SRC += matrix.c i2c_master.c
ifneq ($(strip $(BALLSTEP)),)
    OPT_DEFS += -DTRKSTEP=$(strip $(BALLSTEP))
endif
ifneq ($(strip $(SCROLLSTEP)),)
    OPT_DEFS += -DSCROLLSTEP=$(strip $(SCROLLSTEP))
endif
ifeq ($(strip $(BALLER)), yes)
	POINTING_DEVICE_ENABLE	= yes
    POINTING_DEVICE_DRIVER = custom
    OPT_DEFS += -DBALLER
endif
ifeq ($(strip $(DEBUG_BALLER)), yes)
    OPT_DEFS += -DDEBUG_BALLER
endif
ifeq ($(strip $(DEBUG_MATRIX)), yes)
    OPT_DEFS += -DDEBUG_MATRIX
endif

SRC += marfrit.c
