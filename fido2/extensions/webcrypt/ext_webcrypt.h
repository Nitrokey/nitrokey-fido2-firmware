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


#ifndef FIDO2_EXT_WEBCRYPT_H
#define FIDO2_EXT_WEBCRYPT_H

#include "wallet.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include APP_CONFIG
#include <cbor.h>

#define STATIC_ASSERT(test_for_true)                                           \
  _Static_assert((test_for_true), "(" #test_for_true ") failed")

#define WC_STATE_CONFIG_VERSION   (0xA5)
#define WC_STATE_INITIALIZED_MARKER   (0xA5)

enum CMD_ID {
  COMM_CMD_WRITE = 0x01, // send command
  COMM_CMD_READ = 0x02,  // receive result
};

enum ERROR_ID {
  ERR_SUCCESS = 0x00,
  ERR_REQ_AUTH = 0xF0,
  ERR_INVALID_PIN = 0xF1,
  ERR_NOT_ALLOWED = 0xF2,
  ERR_BAD_FORMAT = 0xF3,
  ERR_USER_NOT_PRESENT = 0xF4,
  ERR_FAILED_LOADING_DATA = 0xF5,
  ERR_INVALID_CHECKSUM = 0xF6,
  ERR_ALREADY_IN_DATABASE = 0xF7,
  ERR_NOT_FOUND = 0xF8,
  ERR_ASSERT_FAILED = 0xF9,
  ERR_INTERNAL_ERROR = 0xFA,
  ERR_INVALID_COMMAND = 0xFF,
};

enum CommandID {
  CMD_STATUS = 0x00, // TODO discuss should it be non-zero
  CMD_TEST_PING = 0x01,
  CMD_TEST_CLEAR = 0x02,
  CMD_TEST_REBOOT = 0x03,
  CMD_LOGIN = 0x04,
  CMD_LOGOUT = 0x05,
  CMD_FACTORY_RESET = 0x06,
  CMD_PIN_ATTEMPTS = 0x07,

  CMD_INITIALIZE_SEED = 0x10,
  CMD_RESTORE_FROM_SEED = 0x11,
  CMD_GENERATE_KEY = 0x12,
  CMD_SIGN = 0x13,
  CMD_DECRYPT = 0x14,
  CMD_GENERATE_KEY_FROM_DATA = 0x15,
  CMD__MAX_SIZE,
};

typedef struct ext_webcrypt_cmd {
  uint8_t command_id;
  uint8_t packet_no;
  uint8_t packet_count;
  uint8_t chunk_size;
  uint8_t this_chunk_length;
  uint8_t data_first_byte[255 - 20 - 5];
} ext_webcrypt_cmd;

#define TMP_PASSW_LENGTH (16)
#define PIN_LENGTH (64)

typedef struct Password {
  uint8_t password[TMP_PASSW_LENGTH];
  bool value_set;
  uint32_t time_set;
} Password;

typedef struct buffer {
  uint8_t *buffer;
  uint32_t data_length;
} buffer;


typedef struct CommandHeader {
  uint8_t command_id; // enum CommandID
} CommandHeader;


uint8_t ext_webcrypt_write_request(uint8_t *output, uint8_t output_size,
                                  ext_webcrypt_cmd *cmd);

uint8_t ext_webcrypt_read_request(uint8_t *output, uint8_t output_size,
                                 ext_webcrypt_cmd *cmd);



#define AUTH_TOKEN_TIME_MS (60 * 1000)

uint8_t PIN_validate_request();
void feedback_show_error();

typedef enum wc_state_source_t {
  WC_STATE_MAIN_PAGE,
  WC_STATE_BACKUP_PAGE
} wc_state_source_t;

typedef enum RequestSource{
  RS_NOT_SET,
  RS_U2F,
  RS_FIDO2,
  RS_NFC,
  RS_BT,
  RS_MAX
} RequestSource;

int8_t bridge_u2f_to_webcrypt(uint8_t *output, uint8_t *keyh, int keylen);
void ext_webcrypt_init();

#endif // FIDO2_EXT_WEBCRYPT_H
