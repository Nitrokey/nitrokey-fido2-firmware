/*
 * Copyright (c) 2018, Nitrokey UG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *
 * gpio.c
 * 		This file contains the GPIO access functions.
 * 		It provides abstractions for access to the LED as well as the user button.
 *
 */
#include "bsp.h"
#include "gpio.h"

#include "u2f_compat.h"
#include "log.h"

// Enable, if device should handle touch button HW clearing
#define BUTTON_HW_CLEARING

#define LOG_STATE_CHANGE


uint32_t        button_press_t;                   // Timer for TaskButton() timings
BUTTON_STATE_T  button_state = BST_INITIALIZING;    // Holds the actual registered logical state of the button
BUTTON_STATE_T  button_state_old = BST_INITIALIZING;    // Holds the actual registered logical state of the button

static data uint32_t  led_blink_tim = 0;                    // Timer for TaskLedBlink() timings
static data uint16_t  led_blink_period_t;                // Period time register
static data uint16_t  led_blink_ON_t;                // ON time register
static data uint8_t   led_blink_num;                    // Blink number counter, also an indicator if blinking is on
static data uint32_t led_default_color = LED_COLOR_REGULAR;

static data uint32_t  button_manager_start_t = 0;
static data bool  first_10_seconds = true;

bool is_in_first_10_seconds(void){
  return first_10_seconds;
}

void button_manager (void) {                          // Requires at least a 750ms long button press to register a valid user button press

  if (first_10_seconds && millis() > 10 * 1000) {
    first_10_seconds = false;
  }

  if (button_state == BST_INITIALIZING){
		if (button_manager_start_t == 0){
			button_manager_start_t = get_ms();
      BUTTON_RESET_OFF();
			return;
		}
		if (get_ms() - button_manager_start_t <= U2F_MS_INIT_BUTTON_PERIOD){
			return;
		}
		button_state = BST_INITIALIZING_READY_TO_CLEAR;
	}
	if (button_state == BST_INITIALIZING_READY_TO_CLEAR){
		return;
	}

	if (IS_BUTTON_PRESSED_RAW()) {                        // Button's physical state: pressed
		switch (button_state) {                        // Handle press phase
		    case BST_UNPRESSED: {                     // It happened at this moment
				button_state  = BST_PRESSED_RECENTLY;  // Update button state
				button_press_t = get_ms();              // Start measure press time
		    }break;
		    case BST_PRESSED_RECENTLY: {              // Button is already pressed, press time measurement is ongoing
				if (get_ms() - button_press_t >= BUTTON_MIN_PRESS_T_MS) { // Press time reached the critical value to register a valid user touch
				    button_state = BST_PRESSED_REGISTERED; // Update button state
				}
		    }break;
		    case BST_PRESSED_CONSUMED:
                        break;
                    case BST_PRESSED_CONSUMED_ACTIVE:
                        if (get_ms() - button_press_t >= 2000) {
                          button_state = BST_PRESSED_CONSUMED;
                        }
                        break;
		    case BST_PRESSED_REGISTERED:
				if (get_ms() - button_press_t >= BUTTON_MAX_PRESS_T_MS) {
					button_state = BST_PRESSED_REGISTERED_TRANSITIONAL;
				}
				break;
		    case BST_PRESSED_REGISTERED_TRANSITIONAL:
		    	if (get_ms() - button_press_t >= BUTTON_MIN_PRESS_T_MS_EXT) {
					button_state = BST_PRESSED_REGISTERED_EXT;
				}
		    	break;
		    case BST_PRESSED_REGISTERED_EXT:
		    	if (get_ms() - button_press_t >= BUTTON_MIN_PRESS_T_MS_EXT+2000) {
                            button_state = BST_PRESSED_REGISTERED_EXT_INVALID;
                        }
		    	break;
		    default:
		    	break;
		}
	} else {                                          // Button is unprssed
		button_state = BST_UNPRESSED;                  // Update button state
	}

#ifdef LOG_STATE_CHANGE
	if (button_state != button_state_old){
//        printf1(TAG_BUTTON, "State changed: %02d => %02d\n", button_state_old, button_state);
        printf1(TAG_BUTTON, "State changed: %s (%02d) => %s (%02d)\n", button_state_to_string(button_state_old), button_state_old,
                                                          button_state_to_string(button_state), button_state);
	    button_state_old = button_state;
    }
#endif

}

#ifdef LOG_STATE_CHANGE
#define m(x)  { case x: return #x; }
char * button_state_to_string(BUTTON_STATE_T state){
  switch (state) {
    m(BST_INITIALIZING)
    m(BST_INITIALIZING_READY_TO_CLEAR)
    m(BST_META_READY_TO_USE)
    m(BST_UNPRESSED)
    m(BST_PRESSED_RECENTLY)
    m(BST_PRESSED_REGISTERED)
    m(BST_PRESSED_REGISTERED_TRANSITIONAL)
    m(BST_PRESSED_REGISTERED_EXT)
    m(BST_PRESSED_REGISTERED_EXT_INVALID)
    m(BST_PRESSED_CONSUMED)
    m(BST_PRESSED_CONSUMED_ACTIVE)
    m(BST_MAX_NUM)
    default:
        return "unknown button state";
  }
}
#undef m
#endif

uint8_t button_get_press (void) {
	return ((button_state == BST_PRESSED_REGISTERED || button_state == BST_PRESSED_CONSUMED_ACTIVE)? 1 : 0);
}

BUTTON_STATE_T button_get_press_state (void) {
	return button_state;
}

bool button_ready_to_work(void){
  return button_get_press_state() > BST_META_READY_TO_USE;
}

uint8_t button_get_press_extended (void) {
	return ((button_state == BST_PRESSED_REGISTERED_EXT)? 1 : 0);
}

uint8_t button_press_in_progress(void){
	return ( (button_state > BST_UNPRESSED)? 1 : 0);
}

void button_press_set_consumed(void){
	button_state = BST_PRESSED_CONSUMED;
}

uint8_t button_press_is_consumed(void){
	return ((button_state == BST_PRESSED_CONSUMED)? 1 : 0);
}

volatile uint8_t LED_STATE = 0;

void led_set_default_color(uint32_t color){
  led_default_color = color;
}

void led_reset_default_color(void) {
  led_default_color = LED_COLOR_REGULAR;
}

void led_on_color(uint32_t color) {
    led_rgb(color);
	LED_STATE = 1;
}

void led_on(void) {
    LED_ON();                                         // LED physical state -> ON
    LED_STATE = 1;
}

void led_off (void) {
	LED_OFF();                                        // LED physical state -> OFF
    LED_STATE = 0;
}

void stop_blinking(void){
    led_blink_num = 0;
    led_off();
}

bool led_is_blinking(void){
	return led_blink_num != 0;
}

void led_change_ON_time(uint16_t ON_time){
	led_blink_ON_t = ON_time;
}

void led_blink (uint8_t blink_num, uint16_t period_t) {
	led_blink_num     	= blink_num;
	led_blink_period_t 	= period_t;
	led_blink_ON_t = LED_BLINK_T_ON;

	if ( (button_ready_to_work() && (get_ms() - led_blink_tim >= LED_BLINK_T_OFF) )
			|| led_blink_num == 1)
        led_on();
	if (!sanity_check_passed)
		led_blink_num = LED_BLINK_NUM_INF;

	led_blink_tim     	= get_ms();
    printf1(TAG_BUTTON, "Blinking set to %d %d\n", blink_num, period_t);
}

void led_blink_manager (void) {
    if (button_get_press_state() == BST_INITIALIZING_READY_TO_CLEAR) {
        led_on_color(LED_COLOR_INIT);
        return;
    }

	if (button_get_press_state() < BST_META_READY_TO_USE && led_blink_num != 1 && sanity_check_passed)
		return;

    if (button_get_press_state() == BST_PRESSED_CONSUMED) {
        stop_blinking();
        led_on_color(LED_COLOR_TOUCH_CONSUMED);
        return;
    }
    if (button_get_press_state() == BST_PRESSED_REGISTERED) {
        // TODO limit to state, where no request is coming
        stop_blinking();
        led_on_color(LED_COLOR_CHARGED);
        return;
    }

	if (led_blink_num) {                                     // LED blinking is on
		if (IS_LED_ON()) {                                 // ON state
			if (get_ms() - led_blink_tim >= led_blink_ON_t) { // ON time expired
                led_off();                                 // LED physical state -> OFF
				if (led_blink_num) {                         // It isnt the last blink round: initialize OFF state:
					led_blink_tim   = get_ms();		       // Init OFF timer
					if (led_blink_num != LED_BLINK_NUM_INF) {              // Not endless blinking:
						led_blink_num--;                     // Update the remaining blink num
					}
                    if (led_blink_num == 0) {
                        printf1(TAG_BUTTON, "Blinking finished\n");
                    }
				}
			}
		} else {                                           // OFF state
			if (get_ms() - led_blink_tim >= LED_BLINK_T_OFF) { // OFF time expired
                led_on();                                  // LED physical state -> ON
				led_blink_tim   = get_ms();		           // Init ON timer
			}
		}
	} else {
	    led_off();
	}
}

static void set_button_cleared(){
  printf1(TAG_BUTTON, "Marking button cleared -> ready to use\n");
  button_state = BST_UNPRESSED;
}


static uint32_t last_button_cleared_time = 0;

uint8_t last_button_cleared_time_delta(){
//	range [0.0-25.5] [s/10]
	return (get_ms() - last_button_cleared_time)/100;
}

uint8_t last_button_pushed_time_delta(){
//	range [0.0-25.5] [s/10]
	return (get_ms() - button_press_t)/100;
}

void clear_button_press(){
	_clear_button_press(false);
}

void _clear_button_press(bool forced){
	if(!forced){
		// do not clear if enough time has not passed, unless button is ready to be cleared
		if (button_get_press_state() != BST_INITIALIZING_READY_TO_CLEAR
				&& (get_ms() - last_button_cleared_time < U2F_MS_CLEAR_BUTTON_PERIOD) )
			return;
		// do not clear, when:
		if (button_get_press_state() == BST_INITIALIZING			// button is not ready for clear yet
				|| button_get_press_state() > BST_UNPRESSED	// button is pressed by the user
				){
			return;
		}
	}

	last_button_cleared_time = get_ms();
	led_off();

#ifdef BUTTON_HW_CLEARING
    uint32_t t0 = get_ms();
    printf1(TAG_BUTTON, "Real clearing\n");
    BUTTON_RESET_ON();
	do {
		u2f_delay(6); 				//6ms activation time + 105ms maximum sleep in NORMAL power mode
        if (get_ms() - t0 > 2000) {
            printf1(TAG_BUTTON, "Clearing timed-out\n");
            break;
        }
	} while (IS_BUTTON_PRESSED_RAW()); // Wait to release button
	BUTTON_RESET_OFF();
#endif

	if (button_get_press_state() == BST_INITIALIZING_READY_TO_CLEAR){
		set_button_cleared();
	}

}
