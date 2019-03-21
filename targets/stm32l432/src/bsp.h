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

#ifndef BSP_H_
#define BSP_H_

#include "device.h"
#include <stdint.h>
#define data

//extern SI_SEGMENT_VARIABLE(myUsbDevice, USBD_Device_TypeDef, MEM_MODEL_SEG);

//SI_SBIT(U2F_BUTTON,       SFR_P0, 1);
//SI_SBIT(U2F_LED,          SFR_P0, 6);
//SI_SBIT(U2F_BUTTON_RESET, SFR_P0, 7);

extern uint8_t U2F_BUTTON_RESET;
extern uint8_t U2F_BUTTON;
extern uint8_t U2F_LED;

/*
 * U2F_BUTTON_RESET is a MTPM pin. Requires HIGH state to keep
 * NORMAL power mode on MTCH101
 * */


#define IS_BUTTON_PRESSED()      (U2F_BUTTON == 0)
#define LED_ON()                 { U2F_LED = 0; }
#define LED_OFF()                { U2F_LED = 1; }
#define BUTTON_RESET_ON()        { U2F_BUTTON_RESET = 0; }
#define BUTTON_RESET_OFF()       { U2F_BUTTON_RESET = 1; }
#define IS_LED_ON()              (U2F_LED == 0)

//
//#define GetEp(epAddr)            (&myUsbDevice.ep0 + epAddr)
//#ifndef DISABLE_WATCHDOG
//#define watchdog()	             (WDTCN = 0xA5)
//#else
//#define watchdog()
//#endif
//#define reboot()	             (RSTSRC = 1 << 4)
#define get_ms()                  millis()

//void u2f_delay  (uint32_t ms);
//void usb_write  (uint8_t* buf, uint8_t len);




#endif /* BSP_H_ */
