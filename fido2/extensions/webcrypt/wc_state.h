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


#ifndef NITROKEY_FIDO2_WC_STATE_H
#define NITROKEY_FIDO2_WC_STATE_H
#include "ext_webcrypt.h"
#include <stdint.h>

typedef struct AES_key_t {
  uint8_t key[32];
  uint8_t IV_SALT[32];
  uint8_t HMAC[32];
} AES_key_t;

typedef struct {
  uint8_t is_initialized;
  uint8_t version;
  uint8_t storage_master_key_set;
  uint8_t _reserved;
  uint8_t PKBDF2_SALT[8];
  uint8_t master_secret[32];
  uint8_t encryption_secret[32]; // has to be 256 bits / 32 bytes
  AES_key_t storage_master_key_enc;
  buffer internal_data_buf;
} webcryptState_t;

extern webcryptState_t WC_STATE;
void wc_load_internal_state(void);
void wc_save_internal_state(void);
void WC_STATE_init(void);

#endif // NITROKEY_FIDO2_WC_STATE_H
