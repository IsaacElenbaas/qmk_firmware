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

//Note the actual autoshift define is not being used (as this replaces it). This
//means the autoshift keys (for adjusting time) could break if they're ever put
//in a #ifdef in quantum_keycodes.h, so if they suddenly break check it.

#include "timer.h"

uint16_t autoshift_time = 0;
uint16_t autoshift_timeout = 150;
uint16_t autoshift_lastkey = KC_NO;
uint16_t autoshift_lastkeyShift = KC_NO;

void autoshift_timer_report(void) {
  //kills itself on planck without autoshift actually enabled - can't include stdio.h
  //char display[8];
  //snprintf(display, 8, "\n%d\n", autoshift_timeout);
  //send_string((const char *)display);
}

void autoshift_on(uint16_t keycode) {
  autoshift_time = timer_read();
  autoshift_lastkey = keycode;
}

void autoshift_flush(void) {
  if(autoshift_lastkey != KC_NO) {
    uint16_t elapsed = timer_elapsed(autoshift_time);
    if(elapsed > autoshift_timeout && !(get_mods() & MOD_BIT(KC_LSFT))) { //LSFT bit is for space holding force shift thing (see keymap)
      if(autoshift_lastkeyShift == KC_NO) {
        register_code(KC_LSFT);
        tap_code16(autoshift_lastkey);
        unregister_code(KC_LSFT);
      }
      else {
        tap_code16(autoshift_lastkeyShift);
      }
    }
    else {
      tap_code16(autoshift_lastkey);
    }
    autoshift_lastkey = KC_NO;
    autoshift_lastkeyShift = KC_NO;
  }
  clear_weak_mods(); //fixes https://github.com/qmk/qmk_firmware/issues/6214 which seems to occur with AS as well
}

bool autoshift_enabled = true;
void autoshift_enable(void) { autoshift_enabled = true; }
void autoshift_disable(void) {
  autoshift_enabled = false;
  autoshift_flush();
}
void autoshift_toggle(void) {
  (autoshift_enabled)
    ? autoshift_disable()
    : autoshift_enable();
}
bool autoshift_state(void) { return autoshift_enabled; }

bool process_auto_shift(uint16_t keycode, keyrecord_t *record) {
  autoshift_flush();
  if(record->event.pressed) {
    switch(keycode) {
      case KC_COMM:
        autoshift_lastkeyShift = KC_QUES;
        break;
      case KC_DOT:
        autoshift_lastkeyShift = KC_EXLM;
        break;
      //Symbol layer
      case KC_MINUS:
        autoshift_lastkeyShift = KC_F13;
        break;
      case KC_SLSH:
        autoshift_lastkeyShift = KC_BSLASH;
        break;
      case KC_LPRN:
        autoshift_lastkeyShift = KC_LBRC;
        break;
      case KC_RPRN:
        autoshift_lastkeyShift = KC_RBRC;
        break;
      case KC_AMPR:
        autoshift_lastkeyShift = KC_PIPE;
        break;
      case KC_EQL:
        autoshift_lastkeyShift = KC_EXLM;
        break;
    }
    switch(keycode) {
      case KC_ASUP:
        autoshift_timeout += 5;
        return false;
      case KC_ASDN:
        autoshift_timeout -= 5;
        return false;
      case KC_ASRP:
        autoshift_timer_report();
        return false;
      case KC_ASTG:
        autoshift_toggle();
        return false;
      case KC_ASON:
        autoshift_enable();
        return false;
      case KC_ASOFF:
        autoshift_disable();
        return false;

      case KC_QUOT:
      case KC_COMM:
      case KC_DOT:
      case KC_TAB:
      case KC_ENT:
      case KC_MINUS:
      case KC_SLSH:
      case KC_LPRN:
      case KC_RPRN:
      case KC_AMPR:
      case KC_EQL:
      case KC_A ... KC_Z:
        if(!autoshift_enabled) { return true; }
        autoshift_on(keycode);
        return false;
    }
  }
  return true;
}
