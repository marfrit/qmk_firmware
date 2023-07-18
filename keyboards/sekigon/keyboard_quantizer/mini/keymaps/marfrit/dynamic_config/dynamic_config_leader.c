// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include "dynamic_config.h"
#include "dynamic_config_def.h"

#include "process_leader.h"

extern bool     leading;
extern uint16_t leader_sequence[];

static int check_leader_sequence(dleader_t const *p_leader) {
    int key_len = p_leader->key_len;
    for (int key = 0; key < key_len; key++) {
        if (leader_sequence[key] == KC_NO) {
            // in progress
            return 0;
        } else if (leader_sequence[key] != p_leader->keys[key]) {
            // failed
            return -1;
        }
    }

    // match all sequence
    return 1;
}

void process_dynamic_config_leader_task(void) {
    if (leading) {
        int16_t app_cnt           = *p_active_app_cnt;
        bool    any_active_leader = false;
        for (int app = app_cnt - 1; app >= 0; app--) {
            if (p_active_apps[app] >= p_config->app_len) break;

            const application_t *p_app = &p_config->p_app[p_active_apps[app]];
            int                  leader_len = p_app->leader_len;
            for (int leader = 0; leader < leader_len; leader++) {
                const dleader_t *p_leader = &p_app->p_leader[leader];
                int              match    = check_leader_sequence(p_leader);
                if (match == 1) {
                    // do action
                    leading = false;
                    dynamic_config_tap_code(p_leader->keycode);
                    return;
                } else if (match == 0) {
                    any_active_leader |= true;
                }
            }
        }

        if (!any_active_leader) {
            leading = false;
        }
    }
}

bool process_record_dynamic_leader(uint16_t keycode, keyrecord_t *record) {
    if (leading && keycode == KC_ESC && record->event.pressed) {
        leading = false;
        return false;
    }

    return true;
}

#if !defined(LEADER_NO_TIMEOUT)
#    error "Option not satisfied"
#endif