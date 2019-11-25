#ifndef LED_COLORS_H
#define LED_COLORS_H

// FIDO2 reset
#define LED_COLOR_DATA_DELETION     (0xFF0000)
// firmware update, settings update
#define LED_COLOR_SYSTEM            (0xFFFF00)
// touch accepted for the request
#define LED_COLOR_TOUCH_CONSUMED    (0x00FF00)
// regular color for all other operations
#define LED_COLOR_REGULAR           (0xFFFFFF)
// initial blink just before being ready
#define LED_COLOR_INIT              (LED_COLOR_REGULAR)

#endif //LED_COLORS_H
