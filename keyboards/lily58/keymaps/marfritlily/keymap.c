#include QMK_KEYBOARD_H

#define MUTE_HOLD_DELAY 400

enum custom_keycodes {
    MUTE_TEAMS = SAFE_RANGE,
    MUTE_SKYPE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT(
/*
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  | BSPC |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   K  |   U  |   Q  |   .  |   J  |                    |   V  |   G  |   C  |   L  |   F  | DEL  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |LShift|   H  |   I  |   E  |   A  |   O  |-------.    ,-------|   D  |   T  |   R  |   N  |   S  |Enter |
 * |------+------+------+------+------+------|   [   |    |    ]  |------+------+------+------+------+------|
 * |LGUI  |   X  |   Y  |   ;  |   ,  |   /  |-------|    |-------|   B  |   P  |   W  |   M  |   Z  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | LAlt | LGUI |LOWER | /Space  /       \Enter \  |RAISE |BackSP| RGUI |
 *                   |      |      |      |/       /         \      \ |      |      |      |
 *                   `----------------------------'           '------''--------------------'
*/
        ALL_T(KC_ESC), KC_1, KC_2,         KC_3,         KC_4,          KC_5,                                  KC_6,       KC_7,         KC_8,         KC_9,         KC_0, KC_BSPC,
        KC_TAB,        KC_K, KC_U,         KC_Q,         LT(3,KC_DOT),  KC_J,                                  KC_V,       LT(3,KC_G),   KC_C,         KC_L,         KC_F, KC_DEL,
        KC_LSFT,       KC_H, LALT_T(KC_I), LCTL_T(KC_E), LSFT_T(KC_A),  KC_O,                                  KC_D,       RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), KC_S, KC_ENT,
        KC_LGUI,       KC_X, RALT_T(KC_Y), KC_SCLN,      LT(1,KC_COMM), LT(2,KC_SLSH), MUTE_TEAMS, MUTE_SKYPE, LT(2,KC_B), LT(1,KC_P),   KC_W,         RALT_T(KC_M), KC_Z, KC_RSFT,
                                           MO(3),        MO(2),         MO(1),         KC_SPC,     KC_BSPC,    MO(1),      MO(2),        MO(3)),
	[1] = LAYOUT(
        KC_F11,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                      KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F12,
        KC_TRNS, KC_AT,   KC_UNDS, KC_LBRC, KC_RBRC, KC_CIRC,                    KC_EXLM, KC_LT,   KC_GT,   KC_EQL,  KC_AMPR, KC_TRNS,
        KC_TRNS, KC_BSLS, KC_SLSH, KC_LCBR, KC_RCBR, KC_ASTR,                    KC_QUES, KC_LPRN, KC_RPRN, KC_MINS, KC_COLN, KC_TRNS,
        KC_TRNS, KC_HASH, KC_DLR,  KC_PIPE, KC_TILD, KC_GRV,  KC_TRNS,  KC_TRNS, KC_PLUS, KC_PERC, KC_DQUO, KC_QUOT, KC_SCLN, KC_TRNS,
                                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, MO(3), KC_TRNS, KC_TRNS),
	[2] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_ESC,  KC_7,    KC_8,    KC_9,    KC_TRNS,                    KC_HOME, KC_PGUP, KC_UP,   KC_PGDN, KC_BSPC, KC_TRNS,
        KC_TRNS, KC_TAB,  KC_4,    KC_5,    KC_6,    KC_TRNS,                    KC_END,  KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT,  KC_TRNS,
        KC_TRNS, KC_CAPS, KC_1,    KC_2,    KC_3,    KC_0,    KC_TRNS,  KC_TRNS, KC_UNDO, KC_CUT,  KC_COPY, KC_PSTE, KC_DEL,  KC_TRNS,
                                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[3] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,                       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,    QK_BOOT,
        KC_TRNS, KC_TRNS, RALT(KC_Y), KC_TRNS,    KC_TRNS,    KC_TRNS,                       KC_TRNS, RGB_TOG, RGB_VAI, RGB_MOD,  KC_TRNS,    KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS,    RALT(KC_5), RALT(KC_Q), RALT(KC_P),                    KC_TRNS, BL_OFF,  RGB_VAD, RGB_RMOD, RALT(KC_S), KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,    KC_TRNS,
                                      KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS)
};

#ifdef OLED_ENABLE

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
