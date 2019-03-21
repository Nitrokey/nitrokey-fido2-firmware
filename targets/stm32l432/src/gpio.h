/*
 * Copyright (c) 2016, Conor Patrick
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

 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdbool.h>

#define BUTTON_MIN_PRESS_T_MS    500
#define BUTTON_MIN_PRESS_T_MS_EXT    (10*1000)
#define BUTTON_MAX_PRESS_T_MS    (3*1000)

// set time after the power on, during which a single U2F or configuration
// request would be accepted
#define SELF_ACCEPT_MAX_T_MS    (2*1000)

#define LED_BLINK_T_ON           ((uint16_t) (LED_BLINK_PERIOD/2))                                 // ms
#define LED_BLINK_T_OFF          ((uint16_t) (led_blink_period_t - led_blink_ON_t))  // ms
#define LED_BLINK_PERIOD         ((uint16_t) 780)                                 // ms
#define LED_BLINK_NUM_INF        255


#define U2F_MS_CLEAR_BUTTON_PERIOD			(20*1000)
#define U2F_MS_INIT_BUTTON_PERIOD			(2*1000)

void button_manager (void);
uint8_t button_get_press (void);
uint8_t button_get_press_extended (void);

uint8_t button_press_in_progress(void);
void button_press_set_consumed(void);
uint8_t button_press_is_consumed(void);
void _clear_button_press(bool forced);
void clear_button_press();

// debug / status functions
uint8_t last_button_cleared_time_delta();
uint8_t last_button_pushed_time_delta();

void led_on (void);
void led_off (void);
bool led_is_blinking(void);
void led_blink (uint8_t blink_num, uint16_t period_t);
void led_blink_manager (void);
void led_change_ON_time(uint16_t ON_time);

typedef enum {
	BST_INITIALIZING,			// wait for the charge to settle down
	BST_INITIALIZING_READY_TO_CLEAR,	// ready for clearing
	BST_META_READY_TO_USE,			// META state (never used), to ease testing,
								// if button is ready (e.g. >READY) or not (<READY)
	BST_UNPRESSED,				// ready to use
	BST_PRESSED_RECENTLY,		// touch registration is started
	BST_PRESSED_REGISTERED,		// touch registered, normal press period
	BST_PRESSED_REGISTERED_TRANSITIONAL,		// touch registered, normal press, but timeouted
	BST_PRESSED_REGISTERED_EXT, // touch registered, extended press period
	BST_PRESSED_CONSUMED,		// touch registered and consumed, but button still not released

	BST_MAX_NUM
} BUTTON_STATE_T;

BUTTON_STATE_T button_get_press_state (void);

#endif /* GPIO_H_ */
