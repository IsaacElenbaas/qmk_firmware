#include QMK_KEYBOARD_H
#include "process_auto_shift_isaacelenbaas.c"
#include "passwords.c" //Instead of extern just to cut down on compile time. Holds a single array.
#define MOUSEL KC_BTN1
#define MOUSER KC_BTN2
#define CTRLL LCTL(KC_LEFT)
#define CTRLR LCTL(KC_RGHT)
#define CAD LCTL(LALT(KC_DEL))

#define BASE_L  0
#define MOD_L   1
#define NAV_L   2
#define MACRO_L 3
#define PASS_L  4

#define CTRLL LCTL(KC_LEFT)
#define CTRLR LCTL(KC_RGHT)

bool hitPlay = true;
uint16_t play_time = 0;
bool hitColon = true;
uint16_t colon_time = 0;
bool hitEscape = true;

enum {
  HK_MOD = SAFE_RANGE,
  HK_NAV,
  HK_MACR,
  HK_PASS,
  HK_IF,
  HK_ELSE,
  HK_COSL,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [BASE_L] = LAYOUT_planck_2x2u(
    KC_VOLD, KC_VOLU, KC_COMM, KC_DOT,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_Z,    HK_MACR,
    KC_TAB,  KC_QUOT, KC_O,    KC_E,    KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_L,    KC_BSPC,
    HK_NAV,  KC_A,    KC_Q,    KC_J,    KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_S,    KC_ENT,
    MOUSER,  MOUSEL,  KC_MPLY, KC_SCLN,          KC_SPC,  HK_MOD,           KC_ESC,  KC_MNXT, KC_NO,   KC_NO
  ),
  [MOD_L] = LAYOUT_planck_2x2u(
    KC_NO,   KC_NO,   KC_MINUS,KC_ASTR, KC_SLSH, HK_IF,   HK_ELSE, KC_LPRN, KC_RPRN, KC_LCBR, KC_NO,   KC_NO,
    KC_UNDS, KC_PLUS, KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_RCBR, KC_CIRC,
    KC_GRAVE,KC_1,    KC_LABK, KC_RABK, KC_AMPR, KC_EQL,  KC_DLR,  KC_AT,   KC_HASH, KC_PERC, KC_0,    KC_TILD,
    KC_NO,   KC_NO,   KC_NO,   KC_DOT,           KC_SPC,  HK_MOD,           KC_NO,   KC_NO,   KC_NO,   KC_NO
  ),
  [NAV_L] = LAYOUT_planck_2x2u(
    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_HOME, KC_UP,   KC_END,  KC_NO,   KC_NO,
    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   CTRLL,   KC_LEFT, KC_DOWN, KC_RGHT, KC_NO,   KC_BSPC,
    HK_NAV,  KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   CTRLR,   KC_ENT,
    KC_NO,   KC_NO,   KC_NO,   KC_SCLN,          KC_SPC,  HK_MOD,           KC_NO,   KC_NO,   KC_NO,   KC_NO
  ),
  [MACRO_L] = LAYOUT_planck_2x2u(
    RESET,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,   KC_NO,   KC_ASUP, KC_ASDN, KC_ASRP, HK_COSL,
    KC_NO,   KC_NO,   KC_F3,   KC_F4,   KC_F5,   KC_F6,  KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,
    KC_F1,   KC_F2,   KC_NO,   KC_NO,   HK_PASS, KC_NO,  KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
    KC_NO,   KC_NO,   KC_MPRV, KC_MNXT,          KC_MPLY,KC_F24,           KC_F23,  KC_F22,  KC_NO,   KC_NO
  ),
  [PASS_L] = LAYOUT_planck_2x2u(
    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_P,    KC_Y,   KC_F,    KC_G,    KC_C,    KC_R,    KC_Z,    HK_COSL,
    KC_NO,   KC_NO,   KC_O,    KC_E,    KC_U,    KC_I,   KC_D,    KC_H,    KC_T,    KC_N,    KC_L,    KC_NO,
    KC_NO,   KC_A,    KC_Q,    KC_J,    KC_K,    KC_X,   KC_B,    KC_M,    KC_W,    KC_V,    KC_S,    KC_NO,
    KC_NO,   KC_NO,   KC_NO,   KC_NO,            KC_NO,  KC_NO,   HK_COSL,          KC_NO,   KC_NO,   KC_NO
  )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  //This section makes a few keys act as normal (aside from being sent on
  //release, fitting with autoshift) but instead act as a modifier if any key
  //is pressed while it was held (and then not sending the normal key).
  if(keycode != KC_MPLY && record->event.pressed) { hitPlay = false;  }
  if(keycode != KC_SCLN && record->event.pressed) { hitColon = false;  }
  if(keycode != KC_ESC && record->event.pressed)  { hitEscape = false; }
  if(process_auto_shift(keycode, record)) {
    if(keycode == KC_MPLY && IS_LAYER_OFF(MACRO_L)) {
      if(record->event.pressed) {
        play_time = timer_read();
        register_code(KC_LCTRL);
        hitPlay = true;
      }
      else {
        unregister_code(KC_LCTRL);
        if(hitPlay && timer_elapsed(play_time) < autoshift_timeout) {
          tap_code(KC_MPLY);
        }
      }
      return false;
    }

    if(keycode == KC_SCLN) {
      if(record->event.pressed) {
        colon_time = timer_read();
        register_code(KC_LSFT);
        hitColon = true;
      }
      else {
        if(!hitColon || timer_elapsed(colon_time) > 3 * autoshift_timeout || IS_LAYER_ON(NAV_L)) {
          unregister_code(KC_LSFT);
        }
        else {
          if(timer_elapsed(colon_time) > autoshift_timeout) {
            tap_code(KC_SCLN);
            unregister_code(KC_LSFT);
          }
          else {
            unregister_code(KC_LSFT);
            tap_code(KC_SCLN);
          }
        }
      }
      return false;
    }

    if(keycode == KC_ESC) {
      if(record->event.pressed) {
        register_code(KC_LGUI);
        hitEscape = true;
      }
      else {
        unregister_code(KC_LGUI);
        if(hitEscape) { tap_code(KC_ESC); }
      }
      return false;
    }
    //end wacky modifier stuff
    if(!record->event.pressed) {
      switch(keycode) {
        case HK_MACR:
          layer_invert(MACRO_L);
          break;
        case HK_PASS:
          autoshift_disable();
          layer_invert(PASS_L);
          layer_invert(MACRO_L);
          break;
        case HK_IF:
          SEND_STRING("if");
          break;
        case HK_ELSE:
          SEND_STRING("else");
          break;
        case HK_COSL:
          clear_keyboard();
        default:
          if(IS_LAYER_ON(MACRO_L) && keycode != KC_NO) { layer_invert(MACRO_L); }
          if(IS_LAYER_ON(PASS_L) && keycode != KC_NO) {
            autoshift_enable();
            layer_invert(PASS_L);
            if(keycode <= KC_Z) {
              send_string(passwords[keycode - KC_A]);
            }
            return false;
          }
      }
    }
    switch(keycode) {
      case HK_MOD:
        layer_invert(MOD_L);
        break;
      case HK_NAV:
        layer_invert(NAV_L);
        break;
      default:
        if(IS_LAYER_ON(PASS_L)) { return false; }
    }
    return true;
  }
  else { return false; }
};
