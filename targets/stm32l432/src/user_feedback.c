#include "user_feedback.h"

#include "bsp.h"
#include "gpio.h"

#include "u2f_compat.h"

static bool first_request_accepted = false;

/**
 * Confirm user presence by getting touch button, or device insertion.
 * Returns: '0' - user presence confirmed, '1' otherwise
 */
static int8_t _u2f_get_user_feedback(BUTTON_STATE_T target_button_state, bool blink)
{
    uint32_t t;
    uint8_t user_presence = 0;

    // Accept first request in the first SELF_ACCEPT_MAX_T_MS after power cycle.
    // Solution only for a short touch request, not for configuration changes.
    if (!first_request_accepted && (get_ms() < SELF_ACCEPT_MAX_T_MS)
        && (target_button_state == BST_PRESSED_REGISTERED) ){
        first_request_accepted = true;
        led_off();
        return 0;
    }

    // Reject all requests, if device is not ready yet for touch button feedback,
    // or if the touch is already consumed
    if (button_press_is_consumed() || button_get_press_state() < BST_META_READY_TO_USE)
        return 1;

    if (blink == true && led_is_blinking() == false)
        led_blink(10, LED_BLINK_PERIOD);
    else if (blink == false)
        led_off();

    t = get_ms();
    while(button_get_press_state() != target_button_state)	// Wait to push button
    {
        led_blink_manager();                               // Run led driver to ensure blinking
        button_manager();                                 // Run button driver
        if (get_ms() - t > U2F_MS_USER_INPUT_WAIT    // 100ms elapsed without button press
            && !button_press_in_progress())			// Button press has not been started
            break;                                    // Timeout
        u2f_delay(10);
#ifdef FAKE_TOUCH
        if (get_ms() - t > 1010) break; //1212
#endif
    }

#ifndef FAKE_TOUCH
    if (button_get_press_state() == target_button_state)
#else //FAKE_TOUCH
        if (true)
#endif
    {
        // Button has been pushed in time
        user_presence = 1;
        button_press_set_consumed();
        led_off();
#ifdef SHOW_TOUCH_REGISTERED
        //show short confirming animation
		t = get_ms();
		while(get_ms() - t < 110){
			led_on();
			u2f_delay(12);
			led_off();
			u2f_delay(25);
		}
		led_off();
#endif
    } else {                                          // Button hasnt been pushed within the timeout
        user_presence = 0;                                     // Return error code
    }


    return user_presence? 0 : 1;
}

int8_t u2f_get_user_feedback(){
    return _u2f_get_user_feedback(BST_PRESSED_REGISTERED, true);
}

int8_t u2f_get_user_feedback_extended_wipe(){
    return _u2f_get_user_feedback(BST_PRESSED_REGISTERED_EXT, false);
}
