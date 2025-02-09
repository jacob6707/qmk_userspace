/* Copyright 2022 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "color.h"
#include "matrix.h"
#include "os_detection.h"
#include "suspend.h"
#include "timer.h"
#include "action.h"
#include "keycodes.h"
#include "quantum.h"
#include "quantum_keycodes.h"
#include "rgb_matrix.h"

#include "km_util.h"
#include "custom_config.h"

#include QMK_KEYBOARD_H
// clang-format off

enum layers{
  MAC_BASE,
  MAC_FN,
  WIN_BASE,
  WIN_FN
};

enum custom_keys {
    KC_WINDOWS_SCREENSHOT = QK_KB,
};

enum command_ids {
    VOLUME_CHANGED = 0x01
};

bool isIdle = true;
static bool is_volume_display_active = false;
static uint32_t last_volume_change_time = 0;    // Time of the last volume change
static uint8_t volume_percentage = 0;           // Last received volume percentage
static uint8_t max_x_coord = 0;                 // To store the rightmost x-coordinate

#define KC_WINSS KC_WINDOWS_SCREENSHOT

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_ansi_109(
        KC_ESC,   KC_BRID,  KC_BRIU,  KC_NO,    KC_NO,    RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,    KC_MUTE,  KC_NO,    KC_NO,    RGB_MOD,  KC_F13,   KC_F14,   KC_F15,   KC_F16,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,  KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,  KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                 KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,            KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,  KC_LOPT,  KC_LCMD,                                KC_SPC,                                 KC_RCMD,  KC_ROPT,  MO(MAC_FN), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),
    [MAC_FN] = LAYOUT_ansi_109(
        _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,     RGB_TOG,  _______,  _______,  RGB_TOG,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,            _______,            _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,    _______,  _______,  _______,  _______,  _______,            _______,  _______),
    [WIN_BASE] = LAYOUT_ansi_109(
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,     KC_MUTE,  KC_WINSS, KC_F17,   RGB_MOD,  KC_F13,   KC_F14,   KC_F15,   KC_F16,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,  KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,  KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                 KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,            KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN,  MO(WIN_FN), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),
    [WIN_FN] = LAYOUT_ansi_109(
        _______,  KC_BRID,  KC_BRIU,  _______,  _______,  RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,    RGB_TOG,  KC_PSCR,  KC_F18,   RGB_TOG,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,            _______,            _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,    _______,  _______,  _______,  _______,  _______,            _______,  _______),
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [MAC_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [MAC_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI) },
    [WIN_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [WIN_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI) }
};
#endif

#if !defined(RGB_MATRIX_ENABLE) || (!defined(CAPS_LOCK_LED_INDEX) && !defined(NUM_LOCK_LED_INDEX))
    bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
        if (!process_record_user(keycode,record)) {
            return false;
        }
        return true;
    }
    bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
        if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
            return false;
        }
        return true;
    }
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (isIdle && record->event.pressed) {
        isIdle = false;
        rgb_matrix_reload_from_eeprom();
    }
    switch(keycode) {
        case KC_WINSS:
            if (record->event.pressed) {
                register_code(KC_LWIN);
                register_code(KC_LSFT);
                register_code(KC_S);
            } else {
                unregister_code(KC_S);
                unregister_code(KC_LSFT);
                unregister_code(KC_LWIN);
            }
            return false;
        case KC_CAPS_LOCK:
            if (record->event.pressed) {
                if (!host_keyboard_led_state().caps_lock) {
                    if (rgb_matrix_get_flags() == LED_FLAG_ALL) {
                        rgb_matrix_sethsv_noeeprom(HSV_RED);
                    }
                } else {
                    rgb_matrix_reload_from_eeprom();
                }
            }
            return true;
        case QK_RGB_MATRIX_TOGGLE:
        case RGB_TOG:
            if (record->event.pressed) {
                switch (rgb_matrix_get_flags()) {
                    case LED_FLAG_ALL: {
                        if (host_keyboard_led_state().caps_lock) {
                            rgb_matrix_reload_from_eeprom();
                        }
                        rgb_matrix_set_flags(LED_FLAG_NONE);
                        rgb_matrix_set_color_all(0, 0, 0);
                    } break;
                    default: {
                        rgb_matrix_set_flags(LED_FLAG_ALL);
                    } break;
                }
            }
            if (!rgb_matrix_is_enabled()) {
                rgb_matrix_set_flags(LED_FLAG_ALL);
                rgb_matrix_enable();
            }
            return false;
    }

    return true;
}

void keyboard_post_init_user(void) {
    reset_rgb_matrix_noeeprom();
    custom_config_init();
    find_max_x_coord(&max_x_coord);
}

bool process_detected_host_os_user(os_variant_t detected_os) {
    if (!isIdle) return false;
    switch (detected_os) {
        case OS_MACOS:
        case OS_IOS:
            rgb_matrix_sethsv_noeeprom(HSV_WHITE);
            break;
        case OS_WINDOWS:
            rgb_matrix_sethsv_noeeprom(HSV_TEAL);
            break;
        case OS_LINUX:
            rgb_matrix_sethsv_noeeprom(HSV_GREEN);
            break;
        case OS_UNSURE:
            rgb_matrix_sethsv_noeeprom(HSV_RED);
            break;
    }

    return true;
}

void matrix_scan_user(void) {
    if (is_volume_display_active && timer_elapsed(last_volume_change_time) > map_to_time_range(g_custom_config.vol_conf.fade_speed)) {  // 1000 ms = 1 second
        is_volume_display_active = false;
    }
}

bool via_command_kb(uint8_t *data, uint8_t length) {
    if (data[0] != 0xFF) return false;
    uint8_t command_id = data[1];
    uint8_t *command_data = &(data[2]);

    switch(command_id) {
        case VOLUME_CHANGED:
            volume_percentage = command_data[0];
            if (!is_volume_display_active) {
                is_volume_display_active = true;
            }
            last_volume_change_time = timer_read32();  // Capture current time in milliseconds
            return true;
    }

    return true;
}

// Custom RGB Matrix effect
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (is_volume_display_active) {
        uint8_t threshold_x = (max_x_coord * volume_percentage) / 100;
        uint16_t fade_time = map_to_time_range(g_custom_config.vol_conf.fade_speed);

        uint8_t brightness = get_fadeout_value((1.0f - timer_elapsed(last_volume_change_time) / (float)fade_time) * 255);
        RGB rgb = hsv_to_rgb(g_custom_config.vol_conf.hsv);

        for (uint8_t i = led_min; i < led_max; i++) {
            if (g_led_config.point[i].x < threshold_x) {
                rgb_matrix_set_color(i, rgb.r * brightness / 255, rgb.g * brightness / 255, rgb.b * brightness / 255);  // Light up LEDs within the threshold
            }
            else if (g_led_config.point[i].x <= threshold_x + 16) {
                uint8_t ratio = (threshold_x + 16 - g_led_config.point[i].x) * 255 / 16;
                uint8_t fade_value = (brightness * ratio) / 255;

                rgb_matrix_set_color(i, rgb.r * fade_value / 255, rgb.g * fade_value / 255, rgb.b * fade_value / 255);  // Light up the LED at the threshold
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
    }

    // RGB_MATRIX_INDICATOR_SET_COLOR(index, red, green, blue);
#   if defined(CAPS_LOCK_LED)
    if (host_keyboard_led_state().caps_lock) {
        RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED, 255, 0, 0);
    } else {
        if (!rgb_matrix_get_flags()) {
            RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED, 0, 0, 0);
        }
    }
#   endif // CAPS_LOCK_LED
#   if defined(NUM_LOCK_LED)
    if (host_keyboard_led_state().num_lock) {
        RGB_MATRIX_INDICATOR_SET_COLOR(NUM_LOCK_LED, 0, 255, 0);
    } else {
        if (!rgb_matrix_get_flags()) {
            RGB_MATRIX_INDICATOR_SET_COLOR(NUM_LOCK_LED, 0, 0, 0);
        }
    }
#   endif // NUM_LOCK_LED

    return true; // Return true to allow other effects to run
}

void suspend_power_down_user() {
    erase_wlength_data();
    isIdle = true;
}

void suspend_wakeup_init_user() {
    erase_wlength_data();
    reset_rgb_matrix_noeeprom();
    os_detection_task();
}
