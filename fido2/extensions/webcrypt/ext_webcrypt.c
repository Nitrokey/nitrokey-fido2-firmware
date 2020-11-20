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


#include "ext_webcrypt.h"
#include "log.h"
#include "wc_buffer.h"
#include "wc_commands.h"
#include "wc_origin.h"
#include "wc_state.h"
#include <cbor.h>
#include <crypto.h>
#include <ctap_errors.h>
#include <device.h>
#include <storage.h>
#include <util.h>

#define MAX_PACKET_NUM (30)
buffer1024 g_buffer_in;
buffer1024 g_buffer_out;

//#include "user_feedback.h"
int8_t u2f_get_user_feedback(); // FIXME add proper header

bool ext_webcrypt_touch_button_confirmation() {
  //    FIXME user separate color for the custom request confirmation
  bool pressed = u2f_get_user_feedback() == 0;
  return pressed;
}

bool check_if_packet_in_sequence(ext_webcrypt_cmd *cmd){
  static uint8_t last_packet_no = 0;

  if (cmd->packet_no == 0) {
    last_packet_no = cmd->packet_no;
    return true;
  }

  if (!(cmd->packet_no == last_packet_no ||
        cmd->packet_no == last_packet_no + 1)) {
    return false;
  }

  last_packet_no = cmd->packet_no;
  return true;
}


typedef enum {
    PROTO_NOT_SET,
    PROTO_IDLE,
    PROTO_READY_TO_READ
} PROTO_STATE;

PROTO_STATE protoState;

bool check_protocol_parameters(ext_webcrypt_cmd *cmd){
    return true;
  s_assertrc(cmd->packet_count > 0, false);
  s_assertrc(cmd->packet_count < MAX_PACKET_NUM, false);
  s_assertrc(cmd->this_chunk_length > 0, false);
  s_assertrc(cmd->chunk_size > 0, false);
  s_assertrc(cmd->this_chunk_length <= cmd->chunk_size, false);
  s_assertrc(cmd->this_chunk_length <= sizeof(cmd->data_first_byte), false);

  // Do not allow to change chunk size once started
  static uint8_t chunk_size = 0;
  if (cmd->packet_no == 0) {
    chunk_size = cmd->chunk_size;
  } else {
    s_assertrc(cmd->chunk_size == chunk_size, false);
  }

  return true;
}

uint8_t ext_webcrypt_read_request(uint8_t *output, uint8_t output_size,
                                  ext_webcrypt_cmd *cmd) {
  assert(cmd != NULL);
  assert(output != NULL);
  s_assertrc(cmd->command_id == COMM_CMD_READ, CTAP1_ERR_INVALID_COMMAND);
  s_assertrc((cmd->packet_no >= 0 && cmd->packet_no <= MAX_PACKET_NUM),
             CTAP1_ERR_INVALID_SEQ);
  s_assertrc(g_buffer_out.buffer_st.data_len > 0, CTAP1_ERR_LOCK_REQUIRED);
  s_assertrc(g_buffer_out.data_length > 3, CTAP1_ERR_LOCK_REQUIRED);
  s_assertrc(check_protocol_parameters(cmd), CTAP1_ERR_INVALID_SEQ);

  // Make sure read from and write to device are on the same origin. Successful
  // read and timeout clear output buffer, read requests are blocked until one
  // of the former happens. Each write request resets origin. Timeout is
  // measured from the last successful access.

  if (origin_is_set() && !origin_is_valid()) {
    buffer_clear(&g_buffer_out);
    origin_invalidate();
    protoState = PROTO_IDLE;
    return CTAP1_ERR_TIMEOUT;
  }

  if (origin_is_set() && !origin_is_current()) {
    // forbid access from different origin
    printf1(TAG_WEBCRYPT, "Invalid origin\n");
    return CTAP1_ERR_CHANNEL_BUSY;
  }

  if (!check_if_packet_in_sequence(cmd)) {
    // access out of sequence - clear all
    buffer_clear(&g_buffer_out);
    origin_invalidate();
    protoState = PROTO_IDLE;
    return CTAP1_ERR_INVALID_SEQ;
  }

  // FIXME check buffer boundaries in read, and the output buffer size
  const uint8_t out_data_len = MIN(cmd->chunk_size, output_size);
  const uint16_t offset = MIN(cmd->chunk_size * cmd->packet_no + out_data_len,
                              g_buffer_out.allocated_size);
  if (offset >= g_buffer_out.allocated_size) {
    printf2(TAG_ERR, "Invalid read request offset\n");
    return CTAP1_ERR_INVALID_LENGTH;
  }
  memmove(output, g_buffer_out.buffer + offset - out_data_len, out_data_len);

#ifdef WC_CLEAR_ON_SUCC_READ
  if (offset > g_buffer_out.data_length + 1) {
    // clear out buffer and origin on successful read
    protoState = PROTO_IDLE;
    buffer_clear(&g_buffer_out);
    origin_invalidate();
    printf1(TAG_WEBCRYPT, "Buffers cleared on succ read\n");
  }
#endif

  webcrypt_clear_origin();

  printf1(TAG_WEBCRYPT, "Webcrypt OUT [%d, %d]: ", offset - out_data_len,
          out_data_len);
  dumpbytes(output, out_data_len);
  return CTAP1_ERR_SUCCESS;
}

void ext_webcrypt_init() {
    printf2(TAG_WEBCRYPT, "Calling initialization\n");
    wc_load_internal_state();
}

bool check_if_packet_repeated(ext_webcrypt_cmd *cmd){
  static uint8_t current_hash[32] = {};
  static uint8_t previous_hash[32] = {};

  // skip check for packet 0
  if(cmd->packet_no == 0)
    return false;

  // TODO check only header first instead of the full request
  crypto_sha256_init();
  crypto_sha256_update((uint8_t *)cmd, 5);
  crypto_sha256_update(cmd->data_first_byte, cmd->this_chunk_length);
  crypto_sha256_final(current_hash);
  const bool repeated = memcmp(current_hash, previous_hash, sizeof(current_hash)) == 0;
  if (repeated) {
    return true;
  }
  memmove(previous_hash, current_hash, sizeof(current_hash));
  return false;
}

uint8_t ext_webcrypt_write_request(uint8_t *output, uint8_t output_size,
                                   ext_webcrypt_cmd *cmd) {
  uint8_t ret = 0;
  assert(cmd != NULL);
  assert(output != NULL);
  s_assertrc(cmd->command_id == COMM_CMD_WRITE, CTAP1_ERR_INVALID_COMMAND);
  s_assertrc((cmd->packet_no >= 0 && cmd->packet_no <= MAX_PACKET_NUM),
             CTAP1_ERR_INVALID_SEQ);
  s_assertrc(check_protocol_parameters(cmd), CTAP1_ERR_INVALID_SEQ);

  static uint8_t previous_result = 0;

  if (cmd->packet_no == 0) {
    buffer_clear(&g_buffer_in);
    buffer_clear(&g_buffer_out);
    previous_result = 0;
    origin_set();
    check_if_packet_in_sequence(cmd);
    memset(output, 0, output_size);
  }

  if (cmd->packet_no > 0 && !origin_is_current()) {
    // forbid access from different origin
    printf1(TAG_WEBCRYPT, "Invalid origin\n");
    return CTAP1_ERR_CHANNEL_BUSY;
  }

  if (check_if_packet_in_sequence(cmd)==false) {
    // access out of sequence - clear all
    buffer_clear(&g_buffer_out);
    origin_invalidate();
    protoState = PROTO_IDLE;
    return CTAP1_ERR_INVALID_SEQ;
  }

  // check if this is not the same USB packet again
  if (check_if_packet_repeated(cmd)) {
    printf1(TAG_ERR, "Same packet as before, skip\n");
    output[0] = previous_result;
    return CTAP1_ERR_SUCCESS;
  };

  // packet is not repeated - continue execution
  const uint16_t offset = cmd->chunk_size * cmd->packet_no;
  if (offset + cmd->this_chunk_length >= g_buffer_in.allocated_size) {
    printf2(TAG_ERR, "Invalid write request offset\n");
    return CTAP1_ERR_INVALID_LENGTH;
  }

  if (cmd->packet_no == 0) {
    buffer_writeback_init(&g_buffer_in);
  }
  buffer_writeback(&g_buffer_in, cmd->data_first_byte, cmd->this_chunk_length);

  printf1(TAG_WEBCRYPT, "Webcrypt IN [%d, %d] -> [%d]: ", offset,
          cmd->this_chunk_length, g_buffer_in.data_length);
  dumpbytes(g_buffer_in.buffer + offset, cmd->this_chunk_length);

  const bool should_parse = cmd->packet_no == cmd->packet_count - 1;

  if (should_parse) {
    memset(output, 0, output_size);
    buffer_writeback_init(&g_buffer_out);
    ret = parse_execute(output + 1, output_size - 1, cmd);
    previous_result = ret;
    printf1(TAG_WEBCRYPT, "Webcrypt IN complete\n");
    dumpbytes(g_buffer_in.buffer, g_buffer_in.data_length + 3);

    buffer_clear(&g_buffer_in);
    protoState = PROTO_READY_TO_READ;
  }

  webcrypt_clear_origin();

  output[0] = ret;
  return CTAP1_ERR_SUCCESS;
}

int8_t bridge_u2f_to_webcrypt(uint8_t *output, uint8_t *keyh, int keylen) {
    int8_t ret = 0;
    wallet_request *req = (wallet_request *)keyh;
    webcrypt_request *const webcrypt = (webcrypt_request *) keyh;
    ext_webcrypt_cmd *const cmd = (ext_webcrypt_cmd *) (webcrypt->payload);

    switch (cmd->command_id) {
        case COMM_CMD_WRITE: {
            ret = ext_webcrypt_write_request(output, 71, cmd);
            break;
        }
        case COMM_CMD_READ: {
            ret = ext_webcrypt_read_request(output, 71, cmd);
            break;
        }
        default: {
            printf2(TAG_ERR, "Invalid storage command: 0x%x\n", req->operation);
            ret = CTAP1_ERR_INVALID_COMMAND;
            dump_hex1(TAG_ERR, req->payload, keylen);
            printf("\n");
            break;
        }
    }
    return ret;
}
