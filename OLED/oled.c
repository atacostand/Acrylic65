#include "acrylic65.h"
#include "oled.h"

#define ROW_1 OLED_DISPLAY_WIDTH
#define ROW_2 (OLED_DISPLAY_WIDTH * 2)

static uint32_t oled_sleep_timer = 0;

// 40 fps
#define FRAME_TIMEOUT (1000/40)
// 30 sec
#define SLEEP_TIMEOUT 30000

static uint16_t anim_timer = 0;

static uint8_t next_bar_val = 0;
static uint8_t next_log_byte[OLED_FONT_WIDTH] = {0};
static uint8_t line1[OLED_DISPLAY_WIDTH] = {0};
static uint8_t line2[OLED_DISPLAY_WIDTH] = {0};

static const uint8_t PROGMEM bar_lut[8] = {0, 16, 24, 56, 60, 124, 126, 255};

#define BAR_KEY_WEIGHT 128
#define BAR_KEY_DECAY_MAX 18
static uint8_t bar_key_decay = BAR_KEY_DECAY_MAX;

// clang-format off
static const char PROGMEM code_to_name[0xFF] = {
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    ' ', ' ', ' ', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',  // 0x
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2',  // 1x
    '3', '4', '5', '6', '7', '8', '9', '0',  20,  19,  27,  26,  22, '-', '=', '[',  // 2x
    ']','\\', '#', ';','\'', '`', ',', '.', '/', 128,   7,   7,   7,   7,   7,   7,  // 3x
      7,   7,   7,   7,   7,   7, 'P', 'S', 'P', 'I', 'H',  30,  16,  31,  17,  27,  // 4x
     26,  25,  24, ' ', ' ', '/', '*', '-', '+',  20, '1', '+', '3', '4', '5', '6',  // 5x
    '7', '8', '9', '0', '.','\\', 'A', 'P', '=',   7,   7,   7,   7,   7,   7,   7,  // 6x
      7,   7,   7,   7, 'X', 'H', 'M', 'S', 'S', 'A', 'U', 'C', 'C', 'P', 'F', 'M',  // 7x
    ' ', ' ', ' ', ' ', ' ', ',', '=', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // 8x
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // 9x
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Ax
    ' ', ' ', ' ', ' ', ' ', ' ', '(', ')', '{', '}', ' ', ' ', 'A', 'B', 'C', 'D',  // Bx
    'E', 'F', 'X', '^', '%', '<', '>', '&', '&', '|', '|', ':', '#', ' ', '@', '!',  // Cx
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Dx
    'C', 'S', 'A', 'C', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Ex
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '        // Fx
};
// clang-format on

void add_keylog(uint16_t keycode) {
    if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) ||
            (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) ||
            (keycode >= QK_MODS && keycode <= QK_MODS_MAX)) {
        keycode = keycode & 0xFF;
    } else if (keycode > 0xFF) {
        keycode = 0;
    }

    if (keycode < (sizeof(code_to_name) / sizeof(char))) {
        char log_char = pgm_read_byte(&code_to_name[keycode]);
        for (uint8_t j = 0; j < OLED_FONT_WIDTH; j++) {
            const uint8_t glyph_line = pgm_read_byte(&font[log_char * OLED_FONT_WIDTH + j]);
            next_log_byte[j] = glyph_line;
        }
    }
}

bool process_record_user_oled(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        oled_sleep_timer = timer_read32();
        add_keylog(keycode);

        uint8_t t = next_bar_val + BAR_KEY_WEIGHT;
        if (t < next_bar_val) {
            next_bar_val = 255;
        } else {
            next_bar_val = t;
        }

        bar_key_decay = BAR_KEY_DECAY_MAX;
    }
    return true;
}
uint8_t render_layer_state(void) {
     uint8_t len = 0;
    oled_write_P(PSTR("LAYER: "), false);
    switch (get_highest_layer(layer_state)) {
        case _MAIN:
            oled_write_P(PSTR("MAIN"), false);
				len=6;
            break;
        case _FN:
            oled_write_P(PSTR("FN"), false);
			len=5;
            break;
    }

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
            break;
 
    return len;
}
void render_mod_status(uint8_t modifiers) {
    //oled_write_P(PSTR("Mods: "), false);
    oled_write_char('S', (modifiers & MOD_MASK_SHIFT));
    oled_write_char('C', (modifiers & MOD_MASK_CTRL));
    oled_write_char('A', (modifiers & MOD_MASK_ALT));
    oled_write_char('G', (modifiers & MOD_MASK_GUI));
    //oled_write_char('\n', false);
}

void render_frame(void) {
    // rotate line 1, and stick in the next byte of the next char,
    // then rotate the next char buffer
    memmove(line1+1, line1, OLED_DISPLAY_WIDTH - 1);
    line1[0] = next_log_byte[OLED_FONT_WIDTH - 1];
    memmove(next_log_byte+1, next_log_byte, OLED_FONT_WIDTH - 1);
    next_log_byte[0] = 0;

    // rotate line 2, sticking in the next display value
    uint8_t disp_val = pgm_read_byte(&bar_lut[next_bar_val / 32]);
    memmove(line2+1, line2, OLED_DISPLAY_WIDTH - 1);
    line2[0] = disp_val;

    // draw both bars
    for (uint8_t i = 0; i < OLED_DISPLAY_WIDTH; i++) {
        oled_write_raw_byte(line1[i], ROW_1 + i);
        oled_write_raw_byte(line2[i], ROW_2 + i);
    }

    // decay the next bar value
    if (next_bar_val > bar_key_decay) {
        next_bar_val -= bar_key_decay;
    } else {
        next_bar_val = 0;
    }

    if (bar_key_decay > 1) {
        bar_key_decay -= 1;
    }
}
void oled_task_user(void) {
    // sleep if it has been long enough since we last got a char
    if (timer_elapsed32(oled_sleep_timer) > SLEEP_TIMEOUT) {
        oled_off();
        return;
    } else {
        oled_on();
    }

    // time for the next frame?
    if (timer_elapsed(anim_timer) > FRAME_TIMEOUT) {
        anim_timer = timer_read();
        render_frame();
    }


