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

#ifndef NITROKEY_FIDO2_WC_KEYS_H
#define NITROKEY_FIDO2_WC_KEYS_H

#include "stdint.h"
#include "u2f.h"


uint8_t wc_new_keypair_from_hash(struct u2f_key_handle *kh, uint8_t *appid, uint8_t *pubkey, uint8_t *key_src_hash);
void wc_crypto_ecc256_shared_secret(const uint8_t * pubkey, const uint8_t * privkey, uint8_t * shared_secret);
void wc_crypto_ecc256_load_key(uint8_t * data, int len, uint8_t * data2, int len2);
void wc_crypto_ecc256_sign_safe(uint8_t * data, int len, uint8_t * sig, size_t sig_buf_len);

#endif//NITROKEY_FIDO2_WC_KEYS_H
