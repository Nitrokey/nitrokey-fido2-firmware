#
# Copyright (c) 2020 Nitrokey GmbH.
#
# This file is part of Nitrokey Webcrypt
# (see https://github.com/Nitrokey/nitrokey-webcrypt).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
#
import functools
import hmac
import struct
from hashlib import sha256

import ecdsa
import pytest
from Crypto.Cipher import AES
from ecdsa import NIST256p
from ecdsa.ecdh import ECDH
from pynitrokey.fido2.client import NKFido2Client as NKFido2Client
from tinyec import registry

from conftest import TEST_DATA, fixture_data_big, fixture_data_random, TEST_DATA_SMALL, cbor_loads
from webcrypt.communication import device_receive, device_send, send_and_receive, set_temporary_password
from webcrypt.helpers import compare_cbor_dict, log_data
from webcrypt.llog import log
from webcrypt.types import Command

SALT = b'salt' * 4
curve = registry.get_curve('secp256r1')


def test_setup(run_on_hardware):
    pass


# @pytest.mark.skip
@pytest.mark.parametrize("test_data", [
    TEST_DATA,
    TEST_DATA_SMALL,
    dict(test='A' * 5),
    fixture_data_big(),
    fixture_data_random(),
    dict(key=b'Z' * 400),  # ping data should be structure independent
    dict(key=b'Z' * 900),  # ping data should be structure independent
    dict(key=b'Z' * (1024 - 71 - 1 - 8)),  # ping data should be structure independent FIXME handle edge case
])
def test_ping(nkfido2_client: NKFido2Client, test_data: dict):
    """Sends arbitrary dict structure over the wire, and receives the same data"""
    assert device_send(nkfido2_client, test_data, Command.TEST_PING)
    commandID, read_data_bytes = device_receive(nkfido2_client)
    compare_cbor_dict(read_data_bytes, test_data)
    assert commandID == Command.TEST_PING.value[0]


@pytest.mark.parametrize("ping_len", [
    100,
    # 512,
    980,
    # 990,
    # *list(range(981, 991, 1)),
])
def test_ping_2(nkfido2_client: NKFido2Client, ping_len: int):
    assert ping_len < 2000
    """Sends arbitrary dict structure over the wire, and receives the same data"""
    d = dict(k=b'A' * ping_len)
    success, read_data_bytes = send_and_receive(nkfido2_client, Command.TEST_PING, d)
    compare_cbor_dict(read_data_bytes, d)


def helper_login(nkfido2_client: NKFido2Client, PIN: bytes, expected_error=None):
    s, data = send_and_receive(nkfido2_client, Command.LOGIN, dict(PIN=PIN), expected_error=expected_error)
    if not expected_error:
        d = cbor_loads(data)[0]
        set_temporary_password(d['_TP'])


@functools.lru_cache(maxsize=None)
def test_setup_session(nkfido2_client):
    log.debug('Setting session')

STATE = {
    "PUBKEY": b"",
    "KEYHANDLE": b"",
}


def test_generate(nkfido2_client):
    read_data = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY)
    log.debug(read_data)
    assert isinstance(read_data, dict)
    assert check_keys_in_received_dictionary(read_data, ["PUBKEY", "KEYHANDLE"])
    global STATE
    STATE = read_data


def test_generate_from_data(nkfido2_client):
    data = {"HASH": b"test"}
    read_data = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    log.debug(read_data)
    assert isinstance(read_data, dict)
    assert check_keys_in_received_dictionary(read_data, ["PUBKEY", "KEYHANDLE"])
    global STATE
    STATE = read_data

    read_data = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    assert read_data["PUBKEY"] == STATE["PUBKEY"]

    data = {"HASH": b"test2"}
    read_data = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    assert read_data["PUBKEY"] != STATE["PUBKEY"]

    data = {"HASH": b"test"}
    read_data = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    assert read_data["PUBKEY"] == STATE["PUBKEY"]


@pytest.mark.parametrize("curve", [
    pytest.param('secp256k1',
                 marks=pytest.mark.xfail(reason='curve must be enabled in the firmware to work')),
    pytest.param('secp256r1',
                 marks=pytest.mark.xfail(reason='not implemented')),
])
def test_sign(nkfido2_client, curve):
    assert "KEYHANDLE" in STATE, "test_generate needs to be run first"

    message = b"test_message"
    hash_data = sha256(message).digest()
    data = {'HASH': hash_data, "KEYHANDLE": STATE["KEYHANDLE"]}
    read_data = send_and_receive_cbor(nkfido2_client, Command.SIGN, data)
    log.debug(read_data)
    assert isinstance(read_data, dict)
    assert check_keys_in_received_dictionary(read_data, ["INHASH", "SIGNATURE"])
    assert hash_data == read_data["INHASH"]

    signature__hex = read_data["SIGNATURE"].hex()
    pubkey__hex = STATE["PUBKEY"].hex()
    log.debug([signature__hex, pubkey__hex, message])
    if curve == 'secp256k1':
        vk = ecdsa.VerifyingKey.from_string(bytes.fromhex(pubkey__hex), curve=ecdsa.SECP256k1,
                                            hashfunc=sha256)
        assert vk.verify(bytes.fromhex(signature__hex), message)
    elif curve == 'secp256r1':
        pytest.fail("Not implemented")
    else:
        assert False, 'Unsupported curve option'


def round_to_next(x, n):
    return x + n - x % n


@pytest.mark.parametrize("param", [
    (16, 16, 32),
    (15, 16, 16),
    (1, 16, 16),
    (0, 16, 16),
])
def test_helper_round(param):
    (x, n, result) = param
    assert result == round_to_next(x, n)


def encrypt_AES(msg, secretKey):
    # PKCS#7 padding
    len_rounded = round_to_next(len(msg), 16)
    msg = msg.ljust(len_rounded, int.to_bytes(len_rounded - len(msg), 1, 'little'))
    log.debug(f'msg={msg}')
    aesCipher = AES.new(secretKey, AES.MODE_CBC, IV=b'\0' * 16)
    ciphertext = aesCipher.encrypt(msg)
    return ciphertext


def test_decrypt(nkfido2_client):
    assert "KEYHANDLE" in STATE, "test_generate needs to be run first"

    msg = b'Text to be encrypted by ECC public key and ' \
          b'decrypted by its corresponding ECC private key'
    log.debug(f"original msg: {msg}")

    ecdh = ECDH(curve=NIST256p)
    ecdh.generate_private_key()
    local_public_key = ecdh.get_public_key()
    ecdh.load_received_public_key_bytes(STATE["PUBKEY"])
    secretKey = ecdh.generate_sharedsecret_bytes()
    ephem_pub_bin = local_public_key.to_string()
    ciphertext = encrypt_AES(msg, secretKey)

    data_len = struct.pack("<H", len(ciphertext))

    log.debug(secretKey.hex())
    h = hmac.new(secretKey, digestmod='sha256')
    h.update(ciphertext)
    h.update(ephem_pub_bin)
    h.update(data_len)
    h.update(STATE["KEYHANDLE"])
    hmac_res = h.digest()

    data = {
        'DATA': ciphertext,
        "KEYHANDLE": STATE["KEYHANDLE"],
        "HMAC": hmac_res,
        "ECCEKEY": ephem_pub_bin,
    }

    log.debug(data)

    read_data = send_and_receive_cbor(nkfido2_client, Command.DECRYPT, data)
    log.debug(read_data)

    assert isinstance(read_data, dict)
    assert check_keys_in_received_dictionary(read_data, ["DATA"])

    log.debug(f"decrypted msg device: {read_data['DATA']}")
    assert msg in read_data["DATA"]


def test_status(nkfido2_client: NKFido2Client):
    read_data = send_and_receive_cbor(nkfido2_client, Command.STATUS)
    log.debug(read_data)
    assert check_keys_in_received_dictionary(read_data, ["UNLOCKED", "VERSION", "SLOTS"])


def send_and_receive_cbor(*args, **kwargs):
    success, read_data_bytes = send_and_receive(*args, **kwargs)
    read_data = cbor_loads(read_data_bytes)
    return read_data


def check_keys_in_received_dictionary(data: dict, keys: list):
    return all(x in data for x in keys)


def test_initialize_simple(nkfido2_client: NKFido2Client):
    read_data = send_and_receive_cbor(nkfido2_client, Command.INITIALIZE_SEED)
    assert check_keys_in_received_dictionary(read_data, ["MASTER", "SALT"])


def test_initialize(nkfido2_client: NKFido2Client):
    data = {"HASH": b"test"}
    key1 = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    key1b = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    assert key1["PUBKEY"].hex() == key1b["PUBKEY"].hex()
    send_and_receive_cbor(nkfido2_client, Command.INITIALIZE_SEED)
    key2 = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data)
    assert key1["PUBKEY"].hex() != key2["PUBKEY"].hex()


def test_restore_simple(nkfido2_client: NKFido2Client):
    data = {"MASTER": b'1' * 32, "SALT": b'2' * 8}
    read_data = send_and_receive_cbor(nkfido2_client, Command.RESTORE_FROM_SEED, data)
    log.debug(read_data)
    assert check_keys_in_received_dictionary(read_data, ["HASH"])


@pytest.mark.parametrize("test_input", [
    (b'0' * 32, b'0' * 8),  # firstly set all to zero
    (b'0' * 32, b'1' * 8),  # check if changing only salts changes the generated keys
    (b'0' * 32, b'0' * 8),  # reset all to zero again
    (b'1' * 32, b'0' * 8),  # check if changing only master changes the generated keys
])
def test_restore(nkfido2_client: NKFido2Client, test_input):
    master, salt = test_input
    data_key = {"HASH": b"test"}
    data = {"MASTER": master, "SALT": salt}
    key1 = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data_key)
    key1b = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data_key)
    assert key1["PUBKEY"].hex() == key1b["PUBKEY"].hex()
    send_and_receive_cbor(nkfido2_client, Command.RESTORE_FROM_SEED, data)
    key2 = send_and_receive_cbor(nkfido2_client, Command.GENERATE_KEY_FROM_DATA, data_key)
    assert key1["PUBKEY"].hex() != key2["PUBKEY"].hex()


def test_PIN_attempts(nkfido2_client: NKFido2Client):
    read_data = send_and_receive_cbor(nkfido2_client, Command.PIN_ATTEMPTS)
    assert check_keys_in_received_dictionary(read_data, ["PIN_ATTEMPTS"])
    log.debug(read_data)
    assert int.from_bytes(read_data["PIN_ATTEMPTS"], 'little', signed=False) == 8
