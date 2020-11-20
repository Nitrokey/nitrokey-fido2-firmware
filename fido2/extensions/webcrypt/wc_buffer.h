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


#ifndef NITROKEY_FIDO2_WC_BUFFER_H
#define NITROKEY_FIDO2_WC_BUFFER_H

#include "wallet.h"
#include <assert.h>
#include <cbor.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include APP_CONFIG


typedef struct buffer1024 {
  uint16_t data_length;
  uint16_t allocated_size;
  union {
    uint8_t buffer[1024];
    struct {
      uint16_t data_len;
      uint8_t command_id;
      uint8_t data_first_byte[1024-2-1];
    } buffer_st;
  };
} buffer1024;

#include <assert.h>
static_assert(sizeof(buffer1024) == 1024+2+2, "Invalid size of the struct");

extern buffer1024 g_buffer_in;
extern buffer1024 g_buffer_out;

void buffer_clear(buffer1024 *buf);
void init_buffer(buffer1024 *buf);
void buffer_writeback_init(buffer1024 *buf);
void buffer_writeback_init_output(buffer1024 *buf);
bool buffer_writeback(buffer1024 *buf, uint8_t *data, uint16_t size);
bool buffer_writeback_output(buffer1024 *buf, uint8_t *data, uint16_t size);

#endif // NITROKEY_FIDO2_WC_BUFFER_H
