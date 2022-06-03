#include "acrylic65.h"
#include <unistd.h>

#define _MAIN 0
#define _FN 1
#define MEDIA_KEY_DELAY 10

int timer = 0;
char wpm_text[5];
int x = 31;
int currwpm = 0;
int vert_count = 0; 

//=============  USER CONFIG PARAMS  ===============
float max_wpm = 110.0f; //WPM value at the top of the graph window
int graph_refresh_interval = 80; //in milliseconds
int graph_area_fill_interval = 5; //determines how dense the horizontal lines under the graph line are; lower = more dense
int vert_interval = 3; //determines frequency of vertical lines under the graph line
bool vert_line = false; //determines whether to draw vertical lines
int graph_line_thickness = 2; //determines thickness of graph line in pixels
//=============  END USER PARAMS  ===============

enum custom_keycodes { // custom keystroke to mute microphone using encoder tap//
    MUTEMIC = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	[_MAIN] = LAYOUT(
		KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, MUTEMIC, 
		KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME, KC_PGUP, 
		KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_DEL, KC_PGDN, 
		KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_LSFT, KC_UP, 
		KC_LCTL, KC_LALT, KC_SPC, KC_LALT, MO(1), KC_LEFT, KC_DOWN, KC_RGHT),

	[_FN] = LAYOUT(
		KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, BL_TOGG, KC_NO, 
		KC_NO, RGB_TOG, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_MNXT, 
		KC_NO, RGB_MOD, RGB_HUI, RGB_SAI, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_VAI, KC_NO, KC_MSTP, KC_MPRV, 
		KC_NO, RGB_RMOD, RGB_HUD, RGB_SAD, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_VAD, KC_NO, KC_NO, 
		KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO, KC_NO),

};

#ifdef ENCODER_ENABLE
#include "encoder.h"
void encoder_update_user(int8_t index, bool clockwise) {
  if (index == 0) { /* First encoder */
    if (clockwise) {
      tap_code(KC_AUDIO_VOL_UP);
    } else {
      tap_code(KC_AUDIO_VOL_DOWN);
    }
  }
}

#endif

#ifdef OLED_DRIVER_ENABLE
void oled_task_user(void) {
	    oled_write_P(PSTR("Acrylic65 Layer: "), false);

    switch (get_highest_layer(layer_state)) {
        case _MAIN:
            oled_write_P(PSTR("MAIN "), false);
            break;
        case _FN:
            oled_write_P(PSTR("FN "), false);
            break;
    }
	//get current WPM value
	currwpm = get_current_wpm();
	
	//check if it's been long enough before refreshing graph
	if(timer_elapsed(timer) > graph_refresh_interval){
	
		// main calculation to plot graph line
		x = 32 - ((currwpm / max_wpm) * 32);
		
		//first draw actual value line
		for(int i = 0; i <= graph_line_thickness - 1; i++){
			oled_write_pixel(1, x + i, true);
		}
		
		
		//then fill in area below the value line
		if(vert_line){
			if(vert_count == vert_interval){
				vert_count = 0;
				while(x <= 32){
					oled_write_pixel(1, x, true);
					x++;
				}
			} else {
				for(int i = 32; i > x; i--){
					if(i % graph_area_fill_interval == 0){
						oled_write_pixel(1, i, true);
					}
				}
				vert_count++;
			}
		} else {
			for(int i = 32; i > x; i--){
				if(i % graph_area_fill_interval == 0){
					oled_write_pixel(1, i, true);
				}
			}
		}
		
		//then move the entire graph one pixel to the right
		oled_pan(false); 
		
		//refresh the timer for the next iteration
		timer = timer_read();
		
	}
	
	//format current WPM value into a printable string
	sprintf(wpm_text,"%i", currwpm);
	
	//formatting for triple digit WPM vs double digits, then print WPM readout
	if(currwpm >= 100){
		oled_set_cursor(14, 3);
		oled_write("WPM: ", false);
		oled_set_cursor(18, 3);
		oled_write(wpm_text, false);
	} else if (currwpm >= 10){
		oled_set_cursor(15, 3);
		oled_write("WPM: ", false);
		oled_set_cursor(19, 3);
		oled_write(wpm_text, false);
	} else if (currwpm > 0) {
		oled_set_cursor(16, 3);
		oled_write("WPM: ", false);
		oled_set_cursor(20, 3);
		oled_write(wpm_text, false);		
	}
	
}

#endif

void matrix_init_user(void) {
}

void matrix_scan_user(void) {
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case MUTEMIC:
        if (record->event.pressed) {
            // Mutes Microphone
            SEND_STRING(SS_DOWN(X_LCTRL)SS_DOWN(X_F10)SS_UP(X_LCTRL)SS_UP(X_F10));
		}
        break;
    }
    return true;
};

