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


#ifndef NITROKEY_FIDO2_WC_COMMANDS_H
#define NITROKEY_FIDO2_WC_COMMANDS_H

#include <stdint.h>
#include "ext_webcrypt.h"


uint8_t cmd_decrypt();
uint8_t cmd_status(void);
uint8_t cmd_login();
uint8_t cmd_logout();
uint8_t cmd_factory_reset();
uint8_t cmd_pin_attempts();
uint8_t cmd_initialize_seed();
uint8_t cmd_restore_from_seed();
uint8_t cmd_generate_key_from_data();
uint8_t cmd_ping();
uint8_t cmd_test_reboot();
uint8_t parse_execute(uint8_t *output, uint8_t output_size,
                      ext_webcrypt_cmd *cmd);
#endif // NITROKEY_FIDO2_WC_COMMANDS_H
