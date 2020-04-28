// Copyright 2019 SoloKeys Developers
//
// Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
// http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
// http://opensource.org/licenses/MIT>, at your option. This file may not be
// copied, modified, or distributed except according to those terms.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx.h"

#include "cbor.h"
#include "device.h"
#include "ctaphid.h"
#include "util.h"
#include "log.h"
#include "ctap.h"
#include APP_CONFIG
#include "memory_layout.h"
#include "init.h"

// TODO insert the target bootloader address here dynamically
#define TARGET_BOOTLOADER   (0x80001234)
typedef void (*pFunction)(void);

void BOOT_boot(void)
{
  uint32_t *bootAddress = (uint32_t *)(TARGET_BOOTLOADER);
  /* Set new vector table */
  SCB->VTOR = TARGET_BOOTLOADER;

  /* Read new SP and PC from vector table */
  __set_MSP(bootAddress[0]);
  ((pFunction)bootAddress[1])();
}

int main()
{
    BOOT_boot();
    return 0;
}
