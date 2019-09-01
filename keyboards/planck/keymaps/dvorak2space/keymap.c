#include QMK_KEYBOARD_H
#include "passwords.c" //Instead of extern just to cut down on compile time. Holds a single array.
#define MOUSEL KC_BTN1
#define MOUSER KC_BTN2
#define CTRLL LCTL(KC_LEFT)
#define CTRLR LCTL(KC_RGHT)
#define CAD LCTL(LALT(KC_DEL))

#define BASE_L  0
#define SHFT_R  1
#define SHFT_L  2
#define MOD_L   3
#define NAV_L   4
#define AHK_L   5
#define LOCK_L  6
#define PASS_L  7

static host_driver_t *host_driver = 0;

int holdShift = 0;
bool macMode = false;

enum {
  HK_SLP = SAFE_RANGE,
  HK_FSLP,
  HK_IF,
  HK_ELSE,
  HK_COSL,
  LT_OFF,
  LT_ON,
  MD_PREV,
  MD_PAUSE,
  MD_NEXT,
  MM_TOGGLE
};

enum {
  FB = 0,
  LPN,
  RPN,
  BCK,
  DSH
};

enum {
  SINGLE_TAP = 1,
  SINGLE_HOLD = 2,
  DOUBLE_TAP = 3,
  DOUBLE_HOLD = 4,
  DOUBLE_SINGLE_TAP = 5, //Distinguishes between double tapping and typing, "tapping", for example. Not sure how accurate it is, and I have no need, so avoiding it at the moment.
  TRIPLE_TAP = 6,
  TRIPLE_HOLD = 7
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/* Base
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   '  |   ,  |   .  |   p  |   y  |   f  |   g  |   c  |   r  |   l  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * | Nav  |   a  |   o  |   e  |   u  |   i  |   d  |   h  |   t  |   n  |   s  | Enter|
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * | Shft |   ;  |   q  |   j  |   k  |   x  |   b  |   m  |   w  |   v  |   z  | Shft |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Esc  | RClk | LClk | Ctrl |    Space    |   Modifier  |  GUI | VolD | VolU |Macros|
 * `-----------------------------------------------------------------------------------'
 */
  [0] = LAYOUT_planck_2x2u(
    KC_TAB,  KC_QUOT,KC_COMM, KC_DOT,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_L,    TD(BCK),
    MO(NAV_L),KC_A,  KC_O,    KC_E,    KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_S,    KC_ENT,
    KC_LSFT, KC_SCLN,KC_Q,    KC_J,    KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_Z,    KC_RSFT,
    KC_ESC,  MOUSER, MOUSEL,  KC_LCTL,          KC_SPC,  MO(MOD_L),        KC_LGUI, KC_VOLD, KC_VOLU, OSL(AHK_L)
  ),
/* Custom Right Shift
 * ,-----------------------------------------------------------------------------------.
 * |      |      |   ?  |   !  |      |      |      |      |      |      |      |  DEL |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |             |             |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */
  [1] = LAYOUT_planck_2x2u(
    KC_TRNS,KC_TRNS,KC_SLSH,KC_1,   KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_DEL,
    KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,        KC_TRNS,KC_NO,          KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS
  ),
/* Custom Left Shift
 * ,-----------------------------------------------------------------------------------.
 * |  INS |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |  ALT |             |             |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */
  [2] = LAYOUT_planck_2x2u(
    KC_INS, KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    KC_TRNS,KC_TRNS,KC_TRNS,KC_LALT,        KC_TRNS,KC_NO,          KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS
  ),
/* Modifier
 * ,-----------------------------------------------------------------------------------.
 * |  Tab |   +  |   -  |   *  |  / \ |  if  | else |  ( [ |  ) ] |   {  |   }  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * |   =  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |   0  | Enter|
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |   `  |   <  |   >  |   &  |   |  |   _  |   $  |   @  |   #  |   %  |   ^  |   ~  |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |   .  |    Space    |             |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */
  [3] = LAYOUT_planck_2x2u(
    KC_TRNS,KC_PLUS,TD(DSH),KC_ASTR,TD(FB), HK_IF,  HK_ELSE,TD(LPN),TD(RPN),KC_LCBR,KC_RCBR,KC_TRNS,
    KC_EQL, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_TRNS,
    KC_GRV, KC_LT,  KC_GT,  KC_AMPR,KC_PIPE,KC_UNDS,KC_DLR, KC_AT,  KC_HASH,KC_PERC,KC_CIRC,LSFT(KC_GRV),
    KC_NO,  KC_NO,  KC_NO,  KC_DOT,         KC_SPC, KC_TRNS,        KC_NO,  KC_NO,  KC_NO,  KC_NO
  ),
//Nav
  [4] = LAYOUT_planck_2x2u(
    KC_TRNS,KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_HOME,KC_UP,  KC_END,  KC_NO,  KC_TRNS,
    KC_TRNS,KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  CTRLL,  KC_LEFT,KC_DOWN,KC_RGHT, CTRLR,  KC_TRNS,
    KC_LSFT,KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,   KC_NO,  KC_NO,
    KC_NO,  KC_NO,  KC_NO,  KC_NO,          KC_SPC, KC_NO,          KC_NO,  KC_NO,   KC_NO,  KC_NO
  ),
//Macros
  [5] = LAYOUT_planck_2x2u(
    LT_OFF,      KC_F14,      KC_F15,      KC_F16,      KC_F17,      KC_F18,      KC_F19,      KC_F20,      KC_F21,      KC_F22,      KC_F23,      LT_ON,
    LCTL(KC_F13),MD_PREV,     MD_PAUSE,    MD_NEXT,     LCTL(KC_F17),LCTL(KC_F18),LCTL(KC_F19),LCTL(KC_F20),LCTL(KC_F21),LCTL(KC_F22),LCTL(KC_F23),LCTL(KC_F24),
    LSFT(KC_F13),LSFT(KC_F14),LSFT(KC_F15),LSFT(KC_F16),LSFT(KC_F17),LSFT(KC_F18),MM_TOGGLE,   HK_FSLP,     LSFT(KC_F21),LSFT(KC_F22),LSFT(KC_F23),LSFT(KC_F24),
    RESET,       LALT(KC_F14),LALT(KC_F15),OSL(PASS_L),              CAD,         LALT(KC_F19),             LALT(KC_F21),LALT(KC_F22),HK_SLP,      HK_COSL
  ),
//Locked Screen
  [6] = LAYOUT_planck_2x2u(
    KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,
    KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,
    KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,
    KC_NO,  KC_NO,  KC_NO,  KC_NO,          KC_NO,  KC_NO,          KC_NO,  KC_NO,  HK_SLP, KC_NO
  ),
//Passwords (by first letter of service name, at least better than just one)
  [7] = LAYOUT_planck_2x2u(
    KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_L,    KC_NO,
    KC_NO,  KC_A,   KC_O,   KC_E,   KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_S,    KC_NO,
    KC_NO,  KC_NO,  KC_Q,   KC_J,   KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_Z,    KC_NO,
    KC_NO,  KC_NO,  KC_NO,  HK_COSL,         KC_NO,   KC_NO,            KC_NO,   KC_NO,   KC_NO,   KC_NO
  )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) { //X_KEY doesn't support aliases
  switch(keycode) {
    //pressing both shifts at the same time will hold down last pressed shift until another is pressed
    case KC_LSFT:
      if(record->event.pressed) {
        if(IS_LAYER_OFF(SHFT_L)) {
          layer_invert(SHFT_L);
        }
        if(holdShift) { //would be impossible on release without recording physical state of key
          if(IS_LAYER_ON(SHFT_R)) {
            layer_invert(SHFT_R);
          }
          unregister_mods(MOD_BIT(KC_RSFT));
        }
        holdShift = 0;
        if(get_mods() & MOD_BIT(KC_RSFT)) {
          holdShift = 2;
        }
      }
      else {
        if(holdShift == 2) {
          return false;
        }
        else {
          layer_invert(SHFT_L);
        }
      }
      break;
    case KC_RSFT:
      if(record->event.pressed) {
        if(IS_LAYER_OFF(SHFT_R)) {
          layer_invert(SHFT_R);
        }
        if(holdShift) {
          if(IS_LAYER_ON(SHFT_L)) {
            layer_invert(SHFT_L);
          }
          unregister_mods(MOD_BIT(KC_LSFT));
        }
        holdShift = 0;
        if(get_mods() & MOD_BIT(KC_LSFT)) {
          holdShift = 1;
        }
      }
      else {
        if(holdShift == 1) {
          return false;
        }
        else {
          layer_invert(SHFT_R);
        }
      }
      break;
    case KC_ENT: //won't repeat on hold and I can't find a solution other than hardcoding timers but I kinda prefer it anyway. Swaps enter and shift enter
      if(record->event.pressed) {
        (get_mods() & (MOD_BIT(KC_LSFT) | MOD_BIT(KC_LSFT))) //if shifted release correct shift, send, and press same shift, else send shift enter
          ? (IS_LAYER_ON(SHFT_L))
            ? SEND_STRING(SS_UP(X_LSHIFT) SS_TAP(X_ENTER) SS_DOWN(X_LSHIFT))
            : SEND_STRING(SS_UP(X_RSHIFT) SS_TAP(X_ENTER) SS_DOWN(X_RSHIFT))
          : SEND_STRING(SS_LSFT(SS_TAP(X_ENTER)));
      }
      return false;
    case HK_IF:
      if(record->event.pressed) { SEND_STRING("if"); }
      break;
    case HK_ELSE:
      if(record->event.pressed) { SEND_STRING("else"); }
      break;
    case HK_COSL:
      clear_keyboard();
      break;
    case KC_VOLU:
      if(!macMode) {
        break;
      }
      else {
        tap_code(KC__VOLUP);
        return false;
      }
    case KC_VOLD:
      if(!macMode) {
        break;
      }
      else {
        tap_code(KC__VOLDOWN);
        return false;
      }
    case LT_OFF:
      if(!macMode) {
        tap_code(KC_F13);
        break;
      }
      else {
        tap_code(KC_BRMD);
        return false;
      }
    case LT_ON:
      if(!macMode) {
        tap_code(KC_F24);
        break;
      }
      else {
        tap_code(KC_BRMU);
        return false;
      }
    case MD_PREV:
      if(!macMode) {
        SEND_STRING(SS_LCTRL(SS_TAP(X_F14)));
      }
      else {
        tap_code(KC_MEDIA_REWIND);
      }
      break;
    case MD_PAUSE:
      if(!macMode) {
        SEND_STRING(SS_LCTRL(SS_TAP(X_F15)));
      }
      else {
        tap_code(KC_MEDIA_PLAY_PAUSE);
      }
      break;
    case MD_NEXT:
      if(!macMode) {
        SEND_STRING(SS_LCTRL(SS_TAP(X_F16)));
      }
      else {
        tap_code(KC_MEDIA_FAST_FORWARD);
      }
      break;
    case MM_TOGGLE:
      macMode = !macMode;
      break;
    case HK_FSLP:
      layer_invert(LOCK_L);
      break;
    case HK_SLP:
      if(record->event.pressed) {
        if(IS_LAYER_OFF(LOCK_L)) {
          host_driver = host_get_driver();
          if(!macMode) {
            SEND_STRING(SS_LALT(SS_TAP(X_F23)));
          }
          else {
            SEND_STRING(SS_LSFT(SS_LCTRL(SS_TAP(X_POWER))));
          }
          host_set_driver(0);
        }
        else {
          host_set_driver(host_driver);
          if(!macMode) {
            SEND_STRING(SS_LALT(SS_TAP(X_F24)));
          }
          else {
            tap_code(KC_POWER);
          }
        }
        return false;
      }
      layer_invert(LOCK_L);
      if(IS_LAYER_ON(AHK_L))
        layer_invert(AHK_L);
      break;
    default:
      if(IS_LAYER_ON(PASS_L) && keycode <= KC_Z) {
        SEND_STRING(passwords[keycode - KC_A]);
        layer_invert(PASS_L);
        return false;
      }
  }
  return true;
};

//tapdance state evaluation
int cur_dance(qk_tap_dance_state_t *state) {
  int press = 0;
  switch(state->count) {
    case 1:
      press = (state->interrupted || !state->pressed)
        ? SINGLE_TAP
        : SINGLE_HOLD;
      break;
     case 2:
      press = DOUBLE_TAP;
      break;
    case 3:
      press = TRIPLE_TAP;
  }
  return press;
}

void back_tap(qk_tap_dance_state_t *state, void *user_data) { tap_code(KC_BSPACE); }

void back_finished(qk_tap_dance_state_t *state, void *user_data) { if(!(state->interrupted || !state->pressed)) tap_code16(LCTL(KC_BSPACE)); }

void slash_finished(qk_tap_dance_state_t *state, void *user_data) {
  int td_state = cur_dance(state);
  switch(td_state) {
    case SINGLE_TAP:
      clear_mods();
      clear_weak_mods();
      tap_code(KC_SLSH);
      break;
    case DOUBLE_TAP:
      tap_code(KC_NUBS);
  }
}

void dash_finished(qk_tap_dance_state_t *state, void *user_data) {
  int td_state = cur_dance(state);
  switch(td_state) {
    case SINGLE_TAP:
      tap_code(KC_PMNS);
      break;
    case SINGLE_HOLD:
      if(!macMode) {
        register_mods(MOD_BIT(KC_LALT));
        tap_code(KC_KP_0);
        tap_code(KC_KP_1);
        tap_code(KC_KP_5);
        tap_code(KC_KP_1);
        unregister_mods(MOD_BIT(KC_LALT));
      }
      else {
        register_mods(MOD_BIT(KC_LSFT));
        register_mods(MOD_BIT(KC_LALT));
        tap_code(KC_MINUS);
        unregister_mods(MOD_BIT(KC_LALT));
        unregister_mods(MOD_BIT(KC_LSFT));
      }
      break;
    case DOUBLE_TAP:
      tap_code(KC_PMNS);
      tap_code(KC_PMNS);
  }
}

qk_tap_dance_action_t tap_dance_actions[] = {
  [LPN] = ACTION_TAP_DANCE_DOUBLE(KC_LPRN, KC_LBRC),
  [RPN] = ACTION_TAP_DANCE_DOUBLE(KC_RPRN, KC_RBRC),
  [FB]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, slash_finished, NULL),
  [BCK] = ACTION_TAP_DANCE_FN_ADVANCED(back_tap, back_finished, NULL), //each tap, on finished, and reset. Normally register_code on press unregister on reset so keys can be held down.
  [DSH] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dash_finished, NULL)
};
