#include QMK_KEYBOARD_H

enum my_keycodes { DBE = SAFE_RANGE };

typedef union {
  uint32_t raw;
} user_config_t;

user_config_t user_config;
uint32_t counter = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {LAYOUT_ortho_1x1(DBE)};

void keyboard_pre_init_kb(void) {
    debug_enable   = true;
    debug_matrix   = true;
    debug_keyboard = true;
    debug_mouse    = true;
    keyboard_pre_init_user();
}

void keyboard_post_init_user(void) {

  // Read the user config from EEPROM
  user_config.raw = eeconfig_read_user();

}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DBE:
            if (record->event.pressed) {
                // Do something when pressed
                dprintf("%02X\n", user_config.raw);
            } else {
                counter++;
                eeconfig_update_user(counter);
                // Do something else when release
            }
            return false; // Skip all further processing of this key
        default:
            return true; // Process all other keycodes normally
    }
}
