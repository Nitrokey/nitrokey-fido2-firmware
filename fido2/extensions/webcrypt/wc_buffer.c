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


#include "wc_buffer.h"
#include "ext_webcrypt.h"
#include "log.h"
#include "wc_commands.h"
#include "wc_origin.h"
#include "wc_state.h"
#include <cbor.h>
#include <crypto.h>
#include <ctap_errors.h>
#include <device.h>
#include <extensions/extensions.h>
#include <storage.h>
#include <util.h>

void init_buffer(buffer1024 *const buf) {
  buf->allocated_size = 1024;
  buf->data_length = 0;
}

void buffer_clear(buffer1024 *buf) {
  assert(buf != NULL);
  memset(buf->buffer, 0, buf->allocated_size);
  init_buffer(buf);
}

bool buffer_writeback(buffer1024 *buf, uint8_t *data, uint16_t size) {
  if (size + buf->data_length >= buf->allocated_size) {
    return false;
  }
  memmove(buf->buffer + buf->data_length, data, size);
  buf->data_length+=size;
  return true;
}

bool buffer_writeback_output(buffer1024 *buf, uint8_t *data, uint16_t size) {
  if (size + buf->data_length >= buf->allocated_size) {
    return false;
  }
  memmove(buf->buffer_st.data_first_byte + buf->data_length, data, size);
  buf->data_length+=size;
  buf->buffer_st.data_len+=size;
  return true;
}

void buffer_writeback_init(buffer1024 *buf) {
  buffer_clear(buf);
}

void buffer_writeback_init_output(buffer1024 *buf) {
  buffer_clear(buf);
  buf->data_length = 0;
}
