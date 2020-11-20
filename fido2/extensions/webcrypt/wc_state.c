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


#include "wc_state.h"
#include "ext_webcrypt.h"
#include "storage.h"
#include "wc_device.h"
#include <device.h>
#include <log.h>

webcryptState_t WC_STATE;
bool WC_STATE_is_initialized(void) {
  return WC_STATE.is_initialized == INITIALIZED_MARKER;
}

void WC_STATE_init(void) {
  memset(&WC_STATE, 0xFF, sizeof(WC_STATE));
  WC_STATE.storage_master_key_set = 0;

  if (ctap_generate_rng(WC_STATE.PKBDF2_SALT, sizeof(WC_STATE.PKBDF2_SALT)) !=
      1) {
    printf2(TAG_ERR, "Error, rng failed\n");
    feedback_show_error();
  }
  if (ctap_generate_rng((uint8_t *)&WC_STATE.storage_master_key_enc,
                        sizeof(WC_STATE.storage_master_key_enc)) != 1) {
    printf2(TAG_ERR, "Error, rng failed\n");
    feedback_show_error();
  }
    if (ctap_generate_rng((uint8_t *)&WC_STATE.master_secret,
                          sizeof(WC_STATE.master_secret)) != 1) {
        printf2(TAG_ERR, "Error, rng failed\n");
        feedback_show_error();
    }

    if (ctap_generate_rng((uint8_t *)&WC_STATE.encryption_secret,
                          sizeof(WC_STATE.encryption_secret)) != 1) {
        printf2(TAG_ERR, "Error, rng failed\n");
        feedback_show_error();
    }

  WC_STATE.version = WC_STATE_CONFIG_VERSION;
  WC_STATE.is_initialized = WC_STATE_INITIALIZED_MARKER;
}

void wc_load_internal_state(void) {
  device_wc_load_internal_state(WC_STATE_MAIN_PAGE);
  dump_arr(TAG_STOR, WC_STATE.master_secret);
  dump_arr(TAG_STOR, WC_STATE.PKBDF2_SALT);
  dump_arr(TAG_STOR, WC_STATE.encryption_secret);
  if (WC_STATE_is_initialized()) {
    printf1(TAG_STOR, "Main WC STATE reported initialized.\n");
    return;
  }
  printf1(TAG_STOR,
          "Main WC STATE reported not initialized. Checking backup.\n");
  // Try recovery
  device_wc_load_internal_state(WC_STATE_BACKUP_PAGE);

  if (!WC_STATE_is_initialized()) {
    // Recovery seems to not be initialized as well. Initialize all.
    printf1(TAG_STOR, "WC STATE is not initialized\n");
    WC_STATE_init();
    wc_save_internal_state();
  } else {
    printf1(
        TAG_STOR,
        "Backup reported initialized. Overwriting main WC STATE with backup\n");
    device_wc_save_internal_state(WC_STATE_MAIN_PAGE);
  }
  dump_arr(TAG_STOR, WC_STATE.master_secret);
  dump_arr(TAG_STOR, WC_STATE.PKBDF2_SALT);
  dump_arr(TAG_STOR, WC_STATE.encryption_secret);
}

void wc_save_internal_state(void) {
  device_wc_save_internal_state(WC_STATE_MAIN_PAGE);
  device_wc_save_internal_state(WC_STATE_BACKUP_PAGE);
}
