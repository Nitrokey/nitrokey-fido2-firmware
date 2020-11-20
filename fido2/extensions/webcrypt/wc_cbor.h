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


#ifndef NITROKEY_FIDO2_WC_CBOR_H
#define NITROKEY_FIDO2_WC_CBOR_H

#include <cbor.h>
typedef uint8_t Data_t;

struct Cbor_kv_t{
    char* key;
    Data_t * data;
    size_t data_len;
};
#define len(arr)   (sizeof(arr)/sizeof((arr)[0]))

CborError wc_cbor_encode_kv_byte_string_arr_enc(CborEncoder *enc, const struct Cbor_kv_t* arr, size_t arr_len);

CborError buffer_writeback_cbor_encode(const struct Cbor_kv_t* arr, size_t arr_len);
CborError cbor_value_map_find_value_arr(CborValue *map, struct Cbor_kv_t * arr, size_t arr_len);
CborError buffer_parse_cbor(struct Cbor_kv_t * arr_in, size_t arr_len);

#endif//NITROKEY_FIDO2_WC_CBOR_H
