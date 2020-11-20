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


#include "wc_cbor.h"
#include "log.h"
#include "wc_buffer.h"

CborError wc_cbor_encode_kv_byte_string(CborEncoder *encMap, char* key, Data_t* data, size_t data_len){
    CborError ret = CborNoError;
    ret = cbor_encode_text_stringz(encMap, key);
    s_assertrc(ret == CborNoError, ret);
    ret = cbor_encode_byte_string(encMap, data, data_len);
    s_assertrc(ret == CborNoError, ret);
    return ret;
}


CborError wc_cbor_encode_kv_byte_string_arr(CborEncoder *encMap, const struct Cbor_kv_t* arr, size_t arr_len){
    CborError ret = CborNoError;
    for (size_t i = 0; i < arr_len; ++i) {
        ret = wc_cbor_encode_kv_byte_string(encMap, arr[i].key, arr[i].data, arr[i].data_len);
        s_assertrc(ret == CborNoError, ret);
    }
    return CborNoError;
}
CborError wc_cbor_encode_kv_byte_string_arr_enc(CborEncoder *enc, const struct Cbor_kv_t* arr, size_t arr_len) {
    CborError ret = CborNoError;
    CborEncoder encMap;
    //    ret = cbor_encoder_create_map(&enc, &encMap, CborIndefiniteLength);
    ret = cbor_encoder_create_map(enc, &encMap, arr_len);
    s_assertrc(ret == CborNoError, ret);
    ret = wc_cbor_encode_kv_byte_string_arr(&encMap, arr, arr_len);
    s_assertrc(ret == CborNoError, ret);
    ret = cbor_encoder_close_container(enc, &encMap);
    s_assertrc(ret == CborNoError, ret);
    return ret;
}



CborError buffer_writeback_cbor_encode(const struct Cbor_kv_t* arr, size_t arr_len){
    CborEncoder enc;
    Data_t *cborbuf = g_buffer_out.buffer_st.data_first_byte;
    CborError ret = CborNoError;

    buffer_writeback_init_output(&g_buffer_out);
    cbor_encoder_init(&enc, cborbuf, g_buffer_out.allocated_size-3, 0);
    ret = wc_cbor_encode_kv_byte_string_arr_enc(&enc, arr, arr_len);
    s_assertrc(ret == CborNoError, ret);

    const size_t bufsize = cbor_encoder_get_buffer_size(&enc, cborbuf);
    g_buffer_out.data_length += bufsize;
    g_buffer_out.buffer_st.data_len += bufsize;
    return ret;
}


CborError cbor_value_map_find_value_arr(CborValue *map, struct Cbor_kv_t * arr, size_t arr_len){
    CborError ret = CborNoError;

    CborValue found_it;
    for (size_t i = 0; i < arr_len; ++i) {
        ret = cbor_value_map_find_value(map, arr[i].key, &found_it);
        s_assertrc(ret == CborNoError, ret);
        printf1(TAG_WEBCRYPT, "Looking for key %s\n", arr[i].key);
        const bool cbor_map_key_found = cbor_value_is_valid(&found_it);
        s_assertrc(cbor_map_key_found, CborInvalidType);
        ret = cbor_value_copy_byte_string(&found_it, arr[i].data, &arr[i].data_len, NULL);
        s_assertrc(ret == CborNoError, ret);
    }

    return ret;
}

CborError buffer_parse_cbor(struct Cbor_kv_t * arr_in, size_t arr_len){
    CborError ret = CborNoError;
    CborParser parser;
    CborValue it;
    cbor_parser_init(g_buffer_in.buffer_st.data_first_byte, g_buffer_in.buffer_st.data_len, 0, &parser, &it);

    const bool is_map = cbor_value_is_map(&it);
    s_assertrc(is_map, CborInvalidType);

    ret = cbor_value_map_find_value_arr(&it, arr_in, arr_len);
    s_assertrc(ret == CborNoError, ret);
    return ret;
}
