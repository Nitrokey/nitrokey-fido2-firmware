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

#include "wc_keys.h"
#include "crypto.h"
#include "device.h"
#include "wc_state.h"
#include <log.h>
#include <stdlib.h>
#include <uECC.h>

#define wc_static_assert(condition, msg)       _Static_assert((condition),"Failed assertion: "#condition " - " #msg)

void wc_generate_private_key(uint8_t * data, int len, uint8_t * data2, int len2, uint8_t * privkey)
{
    crypto_sha256_hmac_init(WC_STATE.master_secret, sizeof(WC_STATE.master_secret), privkey);
    crypto_sha256_update(data, len);
    crypto_sha256_update(data2, len2);
    crypto_sha256_update(WC_STATE.master_secret, sizeof(WC_STATE.master_secret));
    crypto_sha256_hmac_final(WC_STATE.master_secret, sizeof(WC_STATE.master_secret), privkey);

    crypto_aes256_init(WC_STATE.encryption_secret, NULL);
    crypto_aes256_encrypt(privkey, 32);
    wc_static_assert(sizeof(WC_STATE.encryption_secret) == 32, "invalid aes key size");
}


void wc_crypto_ecc256_derive_public_key(uint8_t * data, int len, uint8_t * x, uint8_t * y)
{
    uint8_t privkey[32];
    uint8_t pubkey[64];

    wc_generate_private_key(data,len,NULL,0,privkey);

    memset(pubkey,0,sizeof(pubkey));
    uECC_compute_public_key(privkey, pubkey, _es256_curve);
    memmove(x,pubkey,32);
    memmove(y,pubkey+32,32);
}

static void wc_make_auth_tag(struct u2f_key_handle * kh, uint8_t * appid, uint8_t *out_tag)
{
    uint8_t hashbuf[32];
    crypto_sha256_hmac_init(WC_STATE.master_secret, sizeof(WC_STATE.master_secret), hashbuf);
    crypto_sha256_update(kh->key, sizeof(kh->key));
    crypto_sha256_update(appid, U2F_APPLICATION_SIZE);
    crypto_sha256_hmac_final(WC_STATE.master_secret, sizeof(WC_STATE.master_secret), hashbuf);
    memmove(out_tag, hashbuf, CREDENTIAL_TAG_SIZE);
}

uint8_t wc_new_keypair_from_hash(struct u2f_key_handle *kh, uint8_t *appid, uint8_t *pubkey, uint8_t *key_src_hash) {
    if (key_src_hash == NULL) {
        ctap_generate_rng(kh->key, U2F_KEY_HANDLE_KEY_SIZE);
    } else {
        memmove(kh->key, key_src_hash, sizeof(kh->key));
    }
    wc_make_auth_tag(kh, appid, kh->tag);

    wc_crypto_ecc256_derive_public_key((uint8_t*)kh, U2F_KEY_HANDLE_SIZE, pubkey, pubkey+32);
    return 0;
}

static const uint8_t *wc_g_signing_key = NULL;
static int wc_g_key_len = 0;

void wc_crypto_ecc256_shared_secret(const uint8_t * pubkey, const uint8_t * privkey, uint8_t * shared_secret)
{
    if (privkey == NULL) {
        privkey = wc_g_signing_key;
    }
    if (uECC_shared_secret(pubkey, privkey, shared_secret, _es256_curve) != 1)
    {
        printf2(TAG_ERR, "Error, uECC_shared_secret failed\n");
        exit(1); // FIXME add ret code
    }
}

void wc_crypto_ecc256_load_key(uint8_t * data, int len, uint8_t * data2, int len2)
{
    static uint8_t privkey[32];
    wc_generate_private_key(data,len,data2,len2,privkey);
    wc_g_signing_key = privkey;
    wc_g_key_len = 32;
}
void wc_crypto_ecc256_sign(uint8_t * data, int len, uint8_t * sig);

void wc_crypto_ecc256_sign_safe(uint8_t * data, int len, uint8_t * sig, size_t sig_buf_len){
    if(sig_buf_len != 64){
        printf2(TAG_ERR, "error, uECC failed, invalid buffer len\n");
        exit(1); // FIXME add ret code
    }
    wc_crypto_ecc256_sign(data, len, sig);
}

void wc_crypto_ecc256_sign(uint8_t * data, int len, uint8_t * sig)
{
//    uint8_t tmp[2*32 + 64];
//    uECC_HashContext ectx = {{&crypto_sha256_init, &crypto_sha256_update, &crypto_sha256_final, 64, 32, tmp}};
//    if ( uECC_sign_deterministic(_signing_key, data, len, &ectx, sig, _es256_curve)== 0)
    if ( uECC_sign(wc_g_signing_key, data, len, sig, _es256_curve) == 0)
    {
        printf2(TAG_ERR, "error, uECC failed\n");
        exit(1); // FIXME add ret code
    }
}
