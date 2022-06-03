#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x6465
#define DEVICE_VER      0x0001
#define MANUFACTURER    none
#define PRODUCT         Acrylic65
#define DESCRIPTION     test board for qmk

/* Key matrix size */
#define MATRIX_ROWS 5
#define MATRIX_COLS 17

/* Key matrix pins */
#define MATRIX_ROW_PINS { B3, B4, B5, A10, A9}

#define MATRIX_COL_PINS { A2, A1, B2, B1, A8, A15, A0, B8, B13, B14, B15, B9, B10, B11, B12, A14, A13 }
#define UNUSED_PINS

/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION COL2ROW

/* Set 0 if debouncing isn't needed */
// #define DEBOUNCE 5

/* RGB Underglow */
#ifdef RGBLIGHT_ENABLE
#define RGB_DI_PIN B0
#define RGBLIGHT_LIMIT_VAL 140
#define RGBLED_NUM 30
#define RGBLIGHT_HUE_STEP 8
#define RGBLIGHT_SAT_STEP 8
#define RGBLIGHT_VAL_STEP 8
#define RGBLIGHT_ANIMATIONS
#endif

/* Audio */
#ifdef AUDIO_ENABLE
#define STARTUP_SONG SONG(ZELDA_TREASURE)
#endif
 
/* Encoders */
#define ENCODERS_PAD_A { A6 }
#define ENCODERS_PAD_B { A7 }
#define ENCODER_RESOLUTION 5
#define ENCODER_DIRECTION_FLIP

#define TAP_CODE_DELAY 10
#define MEDIA_KEY_DELAY 10