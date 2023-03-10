#include "marfrit.h"

#ifdef OLED_ENABLE

__attribute__ ((weak))
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  if (!is_keyboard_master())
    return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
  return rotation;
}

// When you add source files to SRC in rules.mk, you can use functions.
const char *read_layer_state(void);
const char *read_logo(void);
void set_keylog(uint16_t keycode, keyrecord_t *record);
const char *read_keylog(void);
const char *read_keylogs(void);

// const char *read_mode_icon(bool swap);
// const char *read_host_led_state(void);
// void set_timelog(void);
// const char *read_timelog(void);

__attribute__ ((weak))
bool oled_task_user(void) {
  if (is_keyboard_master()) {
    // If you want to change the display of OLED, you need to change here
    oled_write_ln(read_layer_state(), false);
    oled_write_ln(read_keylog(), false);
    oled_write_ln(read_keylogs(), false);
    //oled_write_ln(read_mode_icon(keymap_config.swap_lalt_lgui), false);
    //oled_write_ln(read_host_led_state(), false);
    //oled_write_ln(read_timelog(), false);
  } else {
    oled_write(read_logo(), false);
  }
    return false;
}
#endif // OLED_ENABLE

static uint16_t mute_hold_timert = 0;
static uint16_t mute_hold_timers = 0;

__attribute__ ((weak))
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
#ifdef OLED_ENABLE
    set_keylog(keycode & 255, record);  // Ignore LT et al
#endif
    // set_timelog();
  }
  switch (keycode)
  {
  case MUTE_TEAMS:
    if (record->event.pressed) {
        mute_hold_timert = timer_read();
        tap_code16(LCTL(LSFT(KC_M)));
    } else {
        if (timer_elapsed(mute_hold_timert) > MUTE_HOLD_DELAY)
            tap_code16(LCTL(LSFT(KC_M)));
    }
    break;
  case MUTE_SKYPE:
    if (record->event.pressed) {
        mute_hold_timers = timer_read();
        tap_code16(LGUI(KC_F4));
    } else {
        if (timer_elapsed(mute_hold_timers) > MUTE_HOLD_DELAY)
            tap_code16(LGUI(KC_F4));
    }
    break;

  default:
    break;
  }
  return true;
}
