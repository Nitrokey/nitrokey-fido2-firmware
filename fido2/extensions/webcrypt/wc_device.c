/*
 * Copyright (c) 2020 Nitrokey GmbH
 *
 * This file is part of Nitrokey Webcrypt.
 * https://github.com/Nitrokey/nitrokey-webcrypt
 *
 * Nitrokey Webcrypt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Nitrokey Webcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nitrokey Webcrypt. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0
 */


#include "wc_device.h"
#include "../../../targets/stm32l432/src/flash.h"
#include "../../../targets/stm32l432/src/memory_layout.h"
#include "log.h"
#include "wc_state.h"

#ifdef NK_SIMULATION
#include <stdlib.h>
uint8_t simulated_memory[256*1024] = {};
struct flash_memory_st const * const flash_memory = (flash_memory_st*) simulated_memory;

void save_mem_to_file() {
    // saving for inspection
    FILE *f;
    uint16_t ret;
    const char *fn = "webcrypt.bin";
    f = fopen(fn, "wb+");
    if (f == NULL) {
        perror("fopen (ignored)");
        return;
    }
    ret = fwrite(&WC_STATE, 1, sizeof(WC_STATE), f);
    fclose(f);
    if (ret != sizeof(WC_STATE)) {
        perror("fwrite");
        exit(1);
    }
    printf1(TAG_WEBCRYPT, "State saved to file %s\n", fn);
}

void flash_erase_page(uint8_t page)
{
    assert(page<128);
    printf2(TAG_WEBCRYPT, "Erasing page %d\n", page);
    memset(simulated_memory+page*2048, 0xFF, 2048);
}

/**
 *
 * @param addr absolute location in flash/memory
 * @param data data
 * @param sz size
 */
void flash_write(uint32_t addr, uint8_t * data, size_t sz){
    addr -= (size_t )simulated_memory;
    assert(addr+sz < sizeof(simulated_memory));
    printf2(TAG_WEBCRYPT, "Writing to flash in range %p-%p (pages %d-%d)\n", addr, addr+sz, addr/2048, (addr+sz)/2048);
    memmove(addr+simulated_memory, data, sz);
    save_mem_to_file();
}
#else
struct flash_memory_st const * const flash_memory = (flash_memory_st*) FLASH_BEGIN;
#endif


void device_wc_save_internal_state(wc_state_source_t type){
    if (type != WC_STATE_MAIN_PAGE) {
        printf2(TAG_ERR, "Not implemented: %s (backup)\n", __FUNCTION__);
        return;
    }
    const size_t page = (size_t)((void *) flash_memory->webcrypt_data - (void *) flash_memory) / 2048;
    assert(page<128);
    flash_erase_page(page);
    flash_write((size_t)(flash_memory->webcrypt_data), (uint8_t *) &WC_STATE, sizeof(WC_STATE));
}
void device_wc_load_internal_state(wc_state_source_t type){
    if (type != WC_STATE_MAIN_PAGE) {
        printf2(TAG_ERR, "Not implemented: %s (backup)\n", __FUNCTION__);
        return;
    }
    memmove((uint8_t *) &WC_STATE, flash_memory->webcrypt_data, sizeof(WC_STATE));
}

