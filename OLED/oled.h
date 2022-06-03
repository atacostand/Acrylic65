#pragma once

enum crkbd_layers {
  _MAIN,
  _FN
};


bool process_record_user_oled(uint16_t keycode, keyrecord_t *record);
