/* Copyright 2017 Jeremy Cowgar
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

#ifdef AUTO_SHIFT_ENABLE

#    include <stdbool.h>
#    include <stdio.h>

#    include "process_auto_shift.h"

static uint16_t autoshift_time     = 0;
static uint16_t autoshift_timeout  = AUTO_SHIFT_TIMEOUT;
static uint16_t autoshift_lastkey  = KC_NO;
#    ifdef RETRO_SHIFT
static uint16_t retroshift_time    = 0;
static uint16_t retroshift_lastkey = KC_NO;
// Used to check if we should start keyrepeating as getting a keycode early
// enough is not possible (I think).
static keypos_t retro_repeat;
// Stored record of key down, so that we can trigger the hold action if needed.
static keyrecord_t retro_record;
#    endif
static struct {
    // Whether autoshift is enabled.
    bool enabled : 1;
    // Whether the last auto-shifted key was released after the timeout.  This
    // is used to replicate the last key for a tap-then-hold.
    bool lastshifted : 1;
    // Whether an auto-shiftable key has been pressed but not processed.
    bool in_progress : 1;
    // Whether the auto-shifted keypress has been registered.
    bool holding_shift : 1;
} autoshift_flags = {true, false, false, false};

/** \brief Record the press of an autoshiftable key
 *
 *  \return Whether the record should be further processed.
 */
static bool autoshift_press(uint16_t keycode, uint16_t now, keyrecord_t *record) {
    if (!autoshift_flags.enabled) {
        return true;
    }

#    ifndef AUTO_SHIFT_MODIFIERS
    if (get_mods() & (~MOD_BIT(KC_LSFT))) {
        return true;
    }
#    endif
#    ifdef AUTO_SHIFT_REPEAT
    const uint16_t elapsed = TIMER_DIFF_16(now, autoshift_time);
#        ifndef AUTO_SHIFT_NO_AUTO_REPEAT
    if (
#            ifdef RETRO_SHIFT
        retroshift_lastkey != KC_NO ||
#            endif
        !autoshift_flags.lastshifted
    ) {
#        endif
        if (elapsed < TAPPING_TERM && keycode == autoshift_lastkey) {
            // Allow a tap-then-hold for keyrepeat.
            if (!autoshift_flags.lastshifted) {
                register_code(autoshift_lastkey);
            } else {
                // Simulate pressing the shift key.
                add_weak_mods(MOD_BIT(KC_LSFT));
                register_code(autoshift_lastkey);
            }
            return false;
        }
#        ifndef AUTO_SHIFT_NO_AUTO_REPEAT
    }
#        endif
#    endif

#    ifdef RETRO_SHIFT
    if (retroshift_lastkey == KC_NO) {
#    endif
    // Record the keycode so we can simulate it later.
    autoshift_lastkey = keycode;
    autoshift_time    = now;
#        ifdef RETRO_SHIFT
    }
#        endif
    autoshift_flags.in_progress = true;

#    if !defined(NO_ACTION_ONESHOT) && !defined(NO_ACTION_TAPPING)
    clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
#    endif
    return false;
}

/** \brief Registers an autoshiftable key under the right conditions
 *
 * If the autoshift delay has elapsed, register a shift and the key.
 *
 * If the autoshift key is released before the delay has elapsed, register the
 * key without a shift.
 */
static void autoshift_end(uint16_t keycode, uint16_t now, bool matrix_trigger) {
    // Called on key down with KC_NO, auto-shifted key up, and timeout.
    if (autoshift_flags.in_progress && (keycode == autoshift_lastkey || keycode == KC_NO)) {
        // Process the auto-shiftable key.
        autoshift_flags.in_progress = false;

        // Time since the initial press was recorded.
        const uint16_t elapsed = TIMER_DIFF_16(now, autoshift_time);
        if (elapsed < autoshift_timeout) {
            register_code(autoshift_lastkey & 0xFF);
            autoshift_flags.lastshifted = false;
        } else {
            // Simulate pressing the shift key.
            add_weak_mods(MOD_BIT(KC_LSFT));
            register_code(autoshift_lastkey & 0xFF);
            autoshift_flags.lastshifted = true;
#    if defined(AUTO_SHIFT_REPEAT) && !defined(AUTO_SHIFT_NO_AUTO_REPEAT)
            if (matrix_trigger) {
                // Prevents release.
                return;
            }
#    endif
        }

#    if TAP_CODE_DELAY > 0
        wait_ms(TAP_CODE_DELAY);
#    endif
        unregister_code(autoshift_lastkey);
        del_weak_mods(MOD_BIT(KC_LSFT));
    } else {
        // Release after keyrepeat.
        unregister_code(keycode & 0xFF);
        if (keycode == autoshift_lastkey) {
            // This will only fire when the key was the last auto-shiftable
            // pressed. That prevents aaaaBBBB then releasing a from unshifting
            // later Bs (if B wasn't auto-shiftable).
            del_weak_mods(MOD_BIT(KC_LSFT));
        }
    }
    send_keyboard_report(); // del_weak_mods doesn't send one.
    // Roll the autoshift_time forward for detecting tap-and-hold.
    autoshift_time = now;
}

/** \brief Simulates auto-shifted key releases when timeout is hit
 *
 *  Can be called from \c matrix_scan_user so that auto-shifted keys are sent
 *  immediately after the timeout has expired, rather than waiting for the key
 *  to be released.
 */
void autoshift_matrix_scan(void) {
    if (autoshift_flags.in_progress) {
        const uint16_t now     = timer_read();
#    ifdef RETRO_SHIFT
        if (retroshift_lastkey == KC_NO) {
#    endif
            if (TIMER_DIFF_16(now, autoshift_time) >= autoshift_timeout) {
                autoshift_end(autoshift_lastkey, now, true);
            }
#    ifdef RETRO_SHIFT
        } else {
            if ((RETRO_SHIFT + 0) != 0 && TIMER_DIFF_16(now, retroshift_time) > (RETRO_SHIFT + 0)) {
                process_record_handler(&retro_record);
                autoshift_flags.in_progress = false;
                retroshift_lastkey = KC_NO;
            }
        }
#    endif
    }
}

void autoshift_toggle(void) {
    autoshift_flags.enabled = !autoshift_flags.enabled;
    del_weak_mods(MOD_BIT(KC_LSFT));
}

void autoshift_enable(void) { autoshift_flags.enabled = true; }

void autoshift_disable(void) {
    autoshift_flags.enabled = false;
    del_weak_mods(MOD_BIT(KC_LSFT));
}

#    ifndef AUTO_SHIFT_NO_SETUP
void autoshift_timer_report(void) {
    char display[8];

    snprintf(display, 8, "\n%d\n", autoshift_timeout);

    send_string((const char *)display);
}
#    endif

bool get_autoshift_state(void) { return autoshift_flags.enabled; }

uint16_t get_autoshift_timeout(void) { return autoshift_timeout; }

void set_autoshift_timeout(uint16_t timeout) { autoshift_timeout = timeout; }

bool process_auto_shift(uint16_t keycode, keyrecord_t *record) {
    // Note that record->event.time isn't reliable, see:
    // https://github.com/qmk/qmk_firmware/pull/9826#issuecomment-733559550
    const uint16_t now = timer_read();

    if (record->event.pressed) {
#    ifdef RETRO_SHIFT
        if (keycode == retroshift_lastkey) {
            // Keyrepeat, the down event already happened.
            return false;
        }
        // Retro Shifted key and pressing its usual hold action.
        if (retroshift_lastkey != KC_NO) {
            if ((RETRO_SHIFT + 0) == 0) {
                process_record_handler(&retro_record);
                autoshift_flags.in_progress = false;
            }
#        ifdef IGNORE_MOD_TAP_INTERRUPT
            else if (autoshift_flags.in_progress && TIMER_DIFF_16(now, retroshift_time) < (RETRO_SHIFT + 0)) {
                autoshift_time = retroshift_time;
                autoshift_lastkey = retroshift_lastkey;
                autoshift_end(autoshift_lastkey, now, false);
            }
#        endif
            retroshift_lastkey = KC_NO;
        }
        if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) || (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) || (keycode >= QK_MODS && keycode <= QK_MODS_MAX)) {
            if (autoshift_flags.in_progress) {
                autoshift_end(KC_NO, now, false);
            }
#        ifdef RETRO_TAPPING_PER_KEY
            if (!get_retro_tapping(get_event_keycode(record->event, false), record)) {
                return true;
            }
#        endif
            if (record->tap.interrupted) {
                return true;
            } else {
                retroshift_lastkey = keycode;
                retro_repeat = record->event.key;
                retro_record = *record;
            }
        }
#    endif
        if (autoshift_flags.in_progress) {
            // Evaluate previous key if there is one. Doing this elsewhere is
            // more complicated and easier to break.
            autoshift_end(KC_NO, now, false);
        }
        // For pressing another key while keyrepeating shifted autoshift.
        del_weak_mods(MOD_BIT(KC_LSFT));

        switch (keycode & 0xFF) {
            case KC_ASTG:
                autoshift_toggle();
                return true;
            case KC_ASON:
                autoshift_enable();
                return true;
            case KC_ASOFF:
                autoshift_disable();
                return true;

#    ifndef AUTO_SHIFT_NO_SETUP
            case KC_ASUP:
                autoshift_timeout += 5;
                return true;
            case KC_ASDN:
                autoshift_timeout -= 5;
                return true;

            case KC_ASRP:
                autoshift_timer_report();
                return true;
#    endif
        }
    }
#    ifdef RETRO_SHIFT
    else if (keycode == retroshift_lastkey) {
        // Retro Shift key was tapped/held or keyrepeated (handled by us).
        if (!((RETRO_SHIFT + 0) != 0 && TIMER_DIFF_16(now, retroshift_time) > (RETRO_SHIFT + 0))) {
            autoshift_time = retroshift_time;
            autoshift_lastkey = retroshift_lastkey;
            autoshift_end(keycode, now, false);
            retroshift_lastkey = KC_NO;
            return false;
        } else {
            // Hold release action needs to happen.
            return true;
        }
    } else if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) || (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) || (keycode >= QK_MODS && keycode <= QK_MODS_MAX)) {
        // The Tap Hold key was interrupted, is not an Auto Shift key, was
        // cancelled by RETRO_TAPPING_PER_KEY, or RETRO_SHIFT was exceeded.
        if (record->tap.count == 1 && !((RETRO_SHIFT + 0) != 0 && TIMER_DIFF_16(now, retroshift_time) > (RETRO_SHIFT + 0))) {
#        ifdef RETRO_TAPPING_PER_KEY
            if (!get_retro_tapping(get_event_keycode(record->event, false), record)) {
                return true;
            }
#        endif
        } else {
            // Hold release action needs to happen.
            return true;
        }
    }
#    endif

    switch (keycode & 0xFF) {
#    ifndef NO_AUTO_SHIFT_ALPHA
        case KC_A ... KC_Z:
#    endif
#    ifndef NO_AUTO_SHIFT_NUMERIC
        case KC_1 ... KC_0:
#    endif
#    ifndef NO_AUTO_SHIFT_SPECIAL
        case KC_TAB:
        case KC_MINUS ... KC_SLASH:
        case KC_NONUS_BSLASH:
#    endif
            if (record->event.pressed) {
                return autoshift_press(keycode, now, record);
            } else {
                autoshift_end(keycode, now, false);
                return false;
            }
    }
    autoshift_lastkey = KC_NO;
#    ifdef RETRO_SHIFT
    retroshift_lastkey = KC_NO;
#    endif
    return true;
}

#    ifdef RETRO_SHIFT
// event still passed as all places that call this have it; if event->time
// can be fixed a timer read can be avoided.
void retro_shift_set_time(keyevent_t *event) {
    retroshift_time = timer_read();
#        ifdef AUTO_SHIFT_REPEAT
    if (event->key.col == retro_repeat.col && event->key.row == retro_repeat.row && ((autoshift_lastkey >= QK_MOD_TAP && autoshift_lastkey <= QK_MOD_TAP_MAX) || (autoshift_lastkey >= QK_LAYER_TAP && autoshift_lastkey <= QK_LAYER_TAP_MAX) || (autoshift_lastkey >= QK_MODS && autoshift_lastkey <= QK_MODS_MAX))) {
        retroshift_lastkey = autoshift_lastkey;
        autoshift_press(autoshift_lastkey, retroshift_time, false);
    }
#        endif
}
#    endif
#endif
