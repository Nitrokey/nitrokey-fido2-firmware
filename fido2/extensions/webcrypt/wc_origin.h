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


#ifndef NITROKEY_FIDO2_WC_ORIGIN_H
#define NITROKEY_FIDO2_WC_ORIGIN_H

#include "wallet.h"
#include <assert.h>
#include <cbor.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include APP_CONFIG
#include "wc_device.h"

typedef struct {
  uint8_t name[32];
  bool set;
  uint32_t valid_until;
} Origin;


void webcrypt_set_origin(uint8_t* origin_hash, RequestSource rs);
void webcrypt_clear_origin();
uint8_t webcrypt_get_origin(uint8_t* buf, size_t buf_size);
bool origin_is_valid();
void origin_invalidate();
void origin_timeout_update();
bool origin_is_set();
bool origin_is_current();
void origin_set();


#endif // NITROKEY_FIDO2_WC_ORIGIN_H
