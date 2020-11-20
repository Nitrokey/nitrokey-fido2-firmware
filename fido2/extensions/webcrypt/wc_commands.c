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


#include "wc_commands.h"
#include "ext_webcrypt.h"
#include "log.h"
#include "wc_buffer.h"
#include "wc_cbor.h"
#include "wc_keys.h"
#include "wc_origin.h"
#include "wc_state.h"
#include <cbor.h>
#include <crypto.h>
#include <ctap_errors.h>
#include <device.h>
#include <pbkdf2.h>
#include <sha2.h>
#include <storage.h>
#include <u2f.h>
#include <uECC.h>
#include <util.h>

typedef uint8_t Data_t;
typedef uint16_t HandleLen;
uint8_t cmd_sign();
uint8_t cmd_generate_key();
int8_t u2f_load_key(struct u2f_key_handle * kh, uint8_t khl, uint8_t * appid);
int8_t u2f_new_keypair(struct u2f_key_handle * kh, uint8_t * appid, uint8_t * pubkey);

#ifdef NK_TEST_MODE
uint8_t cmd_ping() {
  printf1(TAG_WEBCRYPT, "Webcrypt CMD_TEST_PING\n");
  const bool success =
      buffer_writeback(&g_buffer_out, g_buffer_in.buffer,
                       g_buffer_in.data_length);
  if (!success) {
    return ERR_FAILED_LOADING_DATA;
  }
  return ERR_SUCCESS;
}

uint8_t cmd_test_reboot() {
    printf2(TAG_ERR, "Not implemented\n");
    return ERR_SUCCESS;
}

uint8_t test_clear_webcrypt_data() {
    printf2(TAG_ERR, "Not implemented\n");
    return 0;
}
#endif

const char *CMD_to_str(const enum CommandID id) {
  if (id >= CMD__MAX_SIZE) {
    return "INVALID CMD";
  }

#define M(d)                                                                   \
  if (id == d)                                                                 \
    return #d;
  M(CMD_STATUS);
  M(CMD_TEST_PING);
  M(CMD_TEST_REBOOT);

    M(CMD_LOGIN);
    M(CMD_LOGOUT);
    M(CMD_FACTORY_RESET);
    M(CMD_PIN_ATTEMPTS);
    M(CMD_INITIALIZE_SEED);
    M(CMD_RESTORE_FROM_SEED);
    M(CMD_GENERATE_KEY);
    M(CMD_SIGN);
    M(CMD_DECRYPT);
    M(CMD_GENERATE_KEY_FROM_DATA);
#undef M
  return "UNKNOWN CMD";
}

bool check_password(uint8_t *password, uint16_t password_len) {
    // TODO implement session tokens
    return true;
}

/**
 * Listing command ID here would allow to call it without login / temporary token authorization
 * @param c command id
 * @return true, if command is allowed to run without login
 */
bool command_allowed_without_login(const enum CommandID c){
    enum CommandID allowed_commands[] = {
            CMD_LOGIN, CMD_LOGOUT, CMD_PIN_ATTEMPTS, CMD_FACTORY_RESET,
            CMD_TEST_REBOOT, CMD_TEST_PING};
    for (int i = 0; i < len(allowed_commands); ++i) {
        if (c == allowed_commands[i]) {
            return true;
        }
    }
    return false;
}

/**
 * Commands execution routing. Session check.
 * @param output output buffer to be written to
 * @param output_size output buffer size
 * @param cmd WC cmd structure
 * @return ERR_SUCCESS on success, command execution error otherwise
 */
uint8_t parse_execute(uint8_t *output, uint8_t output_size,
                      ext_webcrypt_cmd *cmd) {
  uint8_t ret = 0;
  CommandHeader header;
  header.command_id = g_buffer_in.buffer_st.command_id;
    g_buffer_out.buffer_st.command_id = header.command_id;
    printf1(TAG_WEBCRYPT, "Webcrypt Command: %s\n",
          CMD_to_str(header.command_id));

  //  Check the temporary password, and reject the command, if it is invalid.
  if (!command_allowed_without_login(header.command_id) &&
      check_password(g_buffer_in.buffer_st.data_first_byte,
                     g_buffer_in.buffer_st.data_len) != true) {
    printf1(TAG_WEBCRYPT,
            "Webcrypt Authorization required for command: %s / %x\n",
            CMD_to_str(header.command_id), header.command_id);
    return ERR_REQ_AUTH;
  }

  switch (header.command_id) {
  case CMD_STATUS: {
    ret = cmd_status();
    break;
  }
#ifdef NK_TEST_MODE
  case CMD_TEST_CLEAR: {
    ret = test_clear_webcrypt_data();
    break;
  }
  case CMD_TEST_PING: {
    ret = cmd_ping();
    break;
  }
  case CMD_TEST_REBOOT:
    ret = cmd_test_reboot();
    break;
#endif
  case CMD_LOGIN:
      ret = cmd_login();
      break;
  case CMD_LOGOUT:
      ret = cmd_logout();
      break;
  case CMD_FACTORY_RESET:
      ret = cmd_factory_reset();
      break;
  case CMD_PIN_ATTEMPTS:
      ret = cmd_pin_attempts();
      break;
  case CMD_INITIALIZE_SEED:
      ret = cmd_initialize_seed();
      break;
  case CMD_RESTORE_FROM_SEED:
      ret = cmd_restore_from_seed();
      break;
  case CMD_GENERATE_KEY:
      ret = cmd_generate_key();
      break;
  case CMD_GENERATE_KEY_FROM_DATA:
      ret = cmd_generate_key_from_data();
      break;
  case CMD_SIGN:
      ret = cmd_sign();
      break;
  case CMD_DECRYPT:
      ret = cmd_decrypt();
      break;
  case CMD__MAX_SIZE:
  default: {
    ret = ERR_INVALID_COMMAND;
    break;
  }
  }
  printf1(TAG_WEBCRYPT, "Webcrypt Finished command: %s => 0x%02X\n",
          CMD_to_str(header.command_id), ret);
  return ret;
}

/**
 * Generates key from the incoming hash data.
 * IN: HASH[32] - 1-32 bytes of input data for generating the key
 * OUT: PUBKEY[64] - ECC PUBKEY
 * OUT: KEYHANDLE[48] - internal key handle
 * @return ERR_BAD_FORMAT on CBOR parsing error, ERR_FAILED_LOADING_DATA on making CBOR response
 */
uint8_t cmd_generate_key_from_data() {
    struct u2f_key_handle key_handle;
    Data_t pubkey[64];
    Data_t appid[32];
    Data_t in_hash[32] = {};
    Data_t in_hash_mix[32] = {};

    CborError ret = CborNoError;
    ret = webcrypt_get_origin(appid, sizeof(appid));
    s_assertrc(ret == 0, ERR_INTERNAL_ERROR);
    dump_arr(TAG_WEBCRYPT, appid);

    struct Cbor_kv_t arr_in[] = {
            {"HASH", in_hash, sizeof(in_hash)},
    };
    ret = buffer_parse_cbor(arr_in, len(arr_in));
    s_assertrc(ret == CborNoError, ERR_BAD_FORMAT);

    cf_pbkdf2_hmac(in_hash, sizeof(in_hash),
                   WC_STATE.PKBDF2_SALT, sizeof(WC_STATE.PKBDF2_SALT),
                   100,
                   in_hash_mix, sizeof(in_hash_mix), // TODO check PBKDF2 impl for using in_hash directly
                    &cf_sha256);

    memmove(in_hash, in_hash_mix, sizeof(in_hash));
    // TODO to check design correctness

    ret = wc_new_keypair_from_hash(&key_handle, appid, pubkey, in_hash);
    s_assertrc(ret == 0, ERR_FAILED_LOADING_DATA);
    dump_arr(TAG_WEBCRYPT, pubkey);
    dump_arrl(TAG_WEBCRYPT, (uint8_t *) &key_handle, sizeof(key_handle));

    struct Cbor_kv_t arr[] = {
            {"PUBKEY", pubkey, sizeof(pubkey)},
            {"KEYHANDLE", (Data_t *) &key_handle, sizeof(key_handle)},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    g_buffer_out.buffer_st.command_id = CMD_GENERATE_KEY_FROM_DATA;
    return ERR_SUCCESS;
}

/**
 * Restore secrets from seed. Simply copies incoming data to Webcrypt structures.
 * IN: MASTER[32] - main secret for the keys creation
 * IN: SALT[8] - PBKDF2 salt for the keys creation
 * OUT: HASH[32] - sha256sum hash of incoming data
 * @return ERR_BAD_FORMAT on invalid length of the input or invalid CBOR, ERR_FAILED_LOADING_DATA on error while preparing response
 */
uint8_t cmd_restore_from_seed() {
    WC_STATE_init();
    CborError ret = CborNoError;
    struct Cbor_kv_t arr_in[] = {
            {"MASTER", WC_STATE.master_secret, sizeof(WC_STATE.master_secret)},
            {"SALT", WC_STATE.PKBDF2_SALT, sizeof(WC_STATE.PKBDF2_SALT)},
    };
    ret = buffer_parse_cbor(arr_in, len(arr_in));
    s_assertrc(ret == CborNoError, ERR_BAD_FORMAT);
    s_assertrc(arr_in[0].data_len == 32, ERR_BAD_FORMAT);
    s_assertrc(arr_in[1].data_len == 8, ERR_BAD_FORMAT);
    wc_save_internal_state();

    Data_t input_hash[32] = {};
    crypto_sha256_init();
    crypto_sha256_update(WC_STATE.master_secret, sizeof(WC_STATE.master_secret));
    crypto_sha256_update(WC_STATE.PKBDF2_SALT, sizeof(WC_STATE.PKBDF2_SALT));
    crypto_sha256_final(input_hash);


    struct Cbor_kv_t arr[] = {
            {"HASH", input_hash, sizeof(input_hash)},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    g_buffer_out.buffer_st.command_id = CMD_RESTORE_FROM_SEED;
    return ERR_SUCCESS;
}

/**
 * Reinitialize the WC structures, including master seed, with random data.
 * @return ERR_FAILED_LOADING_DATA on failing in building the reply
 * @todo remove output / make it debug only
 */
uint8_t cmd_initialize_seed() {
    WC_STATE_init();
    wc_save_internal_state();
    struct Cbor_kv_t arr[] = {
            {"MASTER", WC_STATE.master_secret, sizeof(WC_STATE.master_secret)},
            {"SALT", WC_STATE.PKBDF2_SALT, sizeof(WC_STATE.PKBDF2_SALT)},
    };
    CborError ret;
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    g_buffer_out.buffer_st.command_id = CMD_INITIALIZE_SEED;
    return ERR_SUCCESS;
}

/**
 * Return FIDO2 PIN counter value.
 * OUT: PIN_ATTEMPTS[1] - PIN attempts counter value
 * @return ERR_FAILED_LOADING_DATA on failing in building the reply
 */
uint8_t cmd_pin_attempts() {
    CborError ret;
    uint8_t attempts = ctap_leftover_pin_attempts();
    struct Cbor_kv_t arr[] = {
            {"PIN_ATTEMPTS", (Data_t*)&attempts, sizeof(attempts)},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    g_buffer_out.buffer_st.command_id = CMD_PIN_ATTEMPTS;
    return ERR_SUCCESS;
}

uint8_t cmd_factory_reset() {
    // TODO doubled by initialize seed?
    return 1;
}

uint8_t cmd_logout() {
    return 1;
}

uint8_t cmd_login() {
    return 1;
}

/**
 * Sign incoming data.
 * IN: HASH[32] - hash of the data to sign
 * IN: KEYHANDLE[48] - internal key handle to use for getting the key for signing
 * OUT: SIGNATURE[64] - ECC signature
 * OUT: INHASH[32] - incoming hash for confirmation
 * @return ERR_BAD_FORMAT on invalid length of the input or invalid CBOR, ERR_FAILED_LOADING_DATA on error while preparing response
 */
uint8_t cmd_sign() {
    Data_t key_handle[U2F_KEY_HANDLE_SIZE] = {};
    Data_t in_hash[32] = {};
    Data_t out_sig[64]={};
//    crypto_ecc256_sign(in_hash, 32, out_sig);
//    u2f_authenticate_credential(&req->kh, req->khl, req->app)
//    u2f_load_key(&key_handle, sizeof(key_handle), appid);

    CborError ret = CborNoError;
    struct Cbor_kv_t arr_in[] = {
            {"HASH", in_hash, sizeof(in_hash)},
            {"KEYHANDLE", key_handle, sizeof(key_handle)},
    };
    ret = buffer_parse_cbor(arr_in, len(arr_in));
    s_assertrc(ret == CborNoError, ERR_BAD_FORMAT);

    wc_crypto_ecc256_load_key(key_handle, sizeof(key_handle), NULL, 0);
    wc_crypto_ecc256_sign_safe(in_hash, sizeof(in_hash), out_sig, sizeof(out_sig));


    struct Cbor_kv_t arr[] = {
            {"SIGNATURE", out_sig, sizeof(out_sig)},
            {"INHASH", in_hash, sizeof(in_hash)},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    g_buffer_out.buffer_st.command_id = CMD_SIGN;
    return ERR_SUCCESS;
}


uint8_t cmd_generate_key() {
    struct u2f_key_handle key_handle;
    Data_t pubkey[64];
    Data_t appid[32];

    CborError ret = CborNoError;
    ret = webcrypt_get_origin(appid, sizeof(appid));
    s_assertrc(ret == 0, ERR_INTERNAL_ERROR);

    dump_arr(TAG_WEBCRYPT, appid);

    ret = wc_new_keypair_from_hash(&key_handle, appid, pubkey, NULL);
    s_assertrc(ret == 0, ERR_FAILED_LOADING_DATA);
    dump_arr(TAG_WEBCRYPT, pubkey);
    dump_arrl(TAG_WEBCRYPT, (uint8_t *) &key_handle, sizeof(key_handle));

    struct Cbor_kv_t arr[] = {
            {"PUBKEY", pubkey, sizeof(pubkey)},
            {"KEYHANDLE", (Data_t *) &key_handle, sizeof(key_handle)},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    g_buffer_out.buffer_st.command_id = CMD_GENERATE_KEY;
    return 0;
}

int memcmp_safe(const void *s1, const void *s2, size_t n){
    // TODO implement: replace memcmp with constant time check
    return memcmp(s1, s2, n);
}

/**
 * ECDH decryption
 * 1. generate private key from the keyhandle
 * 2. get public ephemereal key from the data (length could differ depending on algo)
 * 3. create shared key from keyhandle_ecc_priv*ephem_ecc_pub
 * 4. run AES decryption
 * @return 0 on success, error num otherwise
 */
uint8_t cmd_decrypt() {
    Data_t key_handle[U2F_KEY_HANDLE_SIZE] = {};
    Data_t data[128] = {};
    Data_t ephem_ecc_pub[64] = {}; // without initial 0x04 for der octet string, ECC256
    Data_t shared_secret[32] = {};
    Data_t hmac[32] = {};
    Data_t hmac_calc[32] = {};
    uint16_t data_len = 0;

    CborError ret = CborNoError;
    struct Cbor_kv_t arr_in[] = {
            {"DATA", data, sizeof(data)},
            {"KEYHANDLE", key_handle, sizeof(key_handle)},
            {"HMAC", hmac, sizeof(hmac)},
            {"ECCEKEY", ephem_ecc_pub, sizeof(ephem_ecc_pub)},
    };
    ret = buffer_parse_cbor(arr_in, len(arr_in));
    s_assertrc(ret == CborNoError, ERR_BAD_FORMAT);

    s_assertrc(memcmp(arr_in[0].key, "DATA", sizeof("DATA")) == 0, 1);
    data_len = arr_in[0].data_len;
    s_assertrc(data_len < 1024*2, ERR_BAD_FORMAT); // TODO check limit
    wc_crypto_ecc256_load_key(key_handle, sizeof(key_handle), NULL, 0);
    wc_crypto_ecc256_shared_secret(ephem_ecc_pub, NULL, shared_secret);
    dump_arr(TAG_DUMP, shared_secret);

    crypto_sha256_hmac_init(shared_secret, sizeof(shared_secret), hmac_calc);
    crypto_sha256_update(data, data_len);
    crypto_sha256_update(ephem_ecc_pub, sizeof(ephem_ecc_pub));
    crypto_sha256_update((Data_t *) &data_len, sizeof(data_len));
    crypto_sha256_update(key_handle, sizeof(key_handle));
    crypto_sha256_hmac_final(shared_secret, sizeof(shared_secret), hmac_calc);

    const bool hmac_correct = memcmp_safe(hmac, hmac_calc, sizeof(hmac)) == 0;
    s_assertrc(hmac_correct, ERR_FAILED_LOADING_DATA);
    dump_arr(TAG_DUMP, hmac_calc);

    crypto_aes256_init(shared_secret, NULL);
    crypto_aes256_decrypt(data, data_len);

    data_len -= data[data_len-1]; // TODO prove correctness and error resistance
    struct Cbor_kv_t arr[] = {
            {"DATA", data, data_len},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);

    g_buffer_out.buffer_st.command_id = CMD_DECRYPT;
    return 0;
}

static bool unlocked = false;
static Data_t wc_version[] = "1";
static Data_t slots[] = "1";
/**
 * Return device state
 * (Unlocked, version, available resident key slots)
 */
uint8_t cmd_status(void) {
    // TODO implement status command
    uint8_t attempts = ctap_leftover_pin_attempts();

    CborError ret = CborNoError;
    struct Cbor_kv_t arr[] = {
            {"UNLOCKED", (Data_t *) &unlocked, sizeof(unlocked)},
            {"VERSION", wc_version, sizeof(wc_version)},
            {"SLOTS", slots, sizeof(slots)},
            {"PIN_ATTEMPTS", (Data_t*)&attempts, sizeof(attempts)},
    };
    ret = buffer_writeback_cbor_encode(arr, len(arr));
    s_assertrc(ret == CborNoError, ERR_FAILED_LOADING_DATA);
    return 0;
}
