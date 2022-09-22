AUTO_SHIFT_ENABLE = no
LTO_ENABLE = yes
MOUSEKEY_ENABLE = no         # Mouse keys(+4700)
EXTRAKEY_ENABLE = yes        # Audio control and System control(+450)
NKRO_ENABLE = no
MIDI_ENABLE = no             # MIDI controls
AUDIO_ENABLE = no            # Audio output on port C6
BLUETOOTH_ENABLE = no        # Enable Bluetooth with the Adafruit EZ-Key HID
RGBLIGHT_ENABLE = yes       # Enable WS2812 RGB underlight.
SWAP_HANDS_ENABLE = no      # Enable one-hand typing
OLED_DRIVER_ENABLE= yes     # OLED display
OLED_ENABLE = yes

# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = yes    # Breathing sleep LED during USB suspend

# If you want to change the display of OLED, you need to change here
SRC +=  ./lib/rgb_state_reader.c \
        ./lib/layer_state_reader.c \
        ./lib/logo_reader.c \
        ./lib/keylogger.c \
        # ./lib/mode_icon_reader.c \
        # ./lib/host_led_state_reader.c \
        # ./lib/timelogger.c \
