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


#include "wc_origin.h"
#include "ext_webcrypt.h"
#include "log.h"
#include <cbor.h>
#include <crypto.h>
#include <ctap_errors.h>
#include <device.h>
#include <storage.h>
#include <util.h>


const uint32_t origin_read_timeout = 10 * 1000;
static uint8_t *global_appid = NULL;
static RequestSource g_wc_request_source = RS_NOT_SET;
static Origin current_origin;

void webcrypt_set_origin(uint8_t *origin_hash, RequestSource rs) {
  global_appid = origin_hash;
  g_wc_request_source = rs;
}
void webcrypt_clear_origin() { global_appid = NULL; }

uint8_t webcrypt_get_origin(uint8_t* buf, size_t buf_size){
    s_assertrc(buf_size == 32, 1);
    memmove(buf, global_appid, 32);
    return 0;
}

void origin_timeout_update() {
  current_origin.valid_until = millis() + origin_read_timeout;
}

void origin_set() {
  memmove(current_origin.name, global_appid, 32);
  current_origin.set = true;
  origin_timeout_update();
}

/**
 * Provided origin is the same as the current one, and is set
 */
bool origin_is_current() {
  const bool current = current_origin.set == true && memcmp(global_appid, current_origin.name, 32) == 0;
  if (current) {
    origin_timeout_update();
  }
  return current;
}

bool origin_is_set() {
  return current_origin.set;
}

void origin_invalidate() { current_origin.set = false; }

/**
 * Origin is set and not timed out
 */

bool origin_is_valid() {
  origin_timeout_update();
  return current_origin.set && millis() < current_origin.valid_until;
}
