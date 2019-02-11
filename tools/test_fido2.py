import sys
import time
from random import randint

import pytest
from fido2.client import ClientError
from fido2.ctap import CtapError
from fido2.ctap2 import Info
from logger import print
from helpers import helper_ctap_reset


@pytest.mark.basic
def test_ctap_reset(device):
    # FIXME confirm success condition
    device.ctap.reset()


@pytest.mark.basic
def test_ctap_info(device):
    info_reply: Info = device.ctap.get_info()
    assert info_reply
    assert 'U2F_V2' in info_reply.versions
    assert 'FIDO_2_0' in info_reply.versions
    assert info_reply.max_msg_size == 1200
    assert info_reply.options['rk']
    # print(info_reply)


@pytest.mark.fido2
def test_fido2(device):
    helper_ctap_reset(device)
    device.helper_test_fido2(None)


@pytest.mark.fido2
def test_pin_management(device):
    PIN: str = '1122aabbwfg0h9g !@#=='
    PIN = device.helper_test_PIN_management(PIN)
    try:
        device.test_fido2_simple('abcd3')
    except ClientError as e:
        assert e.cause.code == CtapError.ERR.PIN_INVALID

    device.test_fido2_simple(PIN)

    # print('Re-run make_credential and get_assertion tests with pin code')
    device.helper_test_fido2(PIN)

    # print('Reset device')
    device.helper_ctap_reset()


@pytest.mark.fido2
def test_fido2_brute_force(device):
    creds = []
    exclude_list = []
    rp = {'id': device.host, 'name': 'ExaRP'}
    user = {'id': b'usee_od', 'name': 'AB User'}
    PIN = None
    abc = 'abcdefghijklnmopqrstuvwxyz'
    abc += abc.upper()

    total_t1 = time.time() * 1000
    first_counter: int = None

    device.ctap.reset()

    ATTEMPTS_COUNT = 100  # 2048**2
    for i in range(0, ATTEMPTS_COUNT+1):
        creds = []
        challenge = ''.join([abc[randint(0, len(abc) - 1)] for x in range(0, 32)])
        device.helper_populate_exclude_list(exclude_list)
        attest = None
        assertions = None

        for i in range(0, 1):
            rt1 = time.time() * 1000
            attest, data = device.client.make_credential(
                rp, user, challenge, pin=PIN, exclude_list=[]
            )
            # print(attest.auth_data.counter)
            rt2 = time.time() * 1000
            attest.verify(data.hash)
            # print('Register valid (%d ms)' % (t2 - t1))

        cred = attest.auth_data.credential_data
        creds.append(cred)

        for i in range(0, 1):
            allow_list = [{'id': creds[0].credential_id, 'type': 'public-key'}]
            at1 = time.time() * 1000
            assertions, client_data = device.client.get_assertion(
                rp['id'], challenge, allow_list, pin=PIN
            )
            at2 = time.time() * 1000
            if first_counter is None:
                first_counter = assertions[0].auth_data.counter
            assertions[0].verify(client_data.hash, creds[0].public_key)
            # print(assertions[0].auth_data.counter)

            # print('Assertion valid (%d ms)' % (t2 - t1))
    assert assertions[0].auth_data.counter > 0
    assert assertions[0].auth_data.counter - first_counter == ATTEMPTS_COUNT*2
    total_t2 = time.time() * 1000
    total_time = total_t2 - total_t1
    average_time_per_iteration = (total_time) / ATTEMPTS_COUNT
    print('All operations time: {} ms ({} per iteration)'.format(round(total_time,4), round(average_time_per_iteration, 4)))
    assert average_time_per_iteration < 100
    # print('Register valid (%d ms)'.format((total_t2 - total_t1) / ATTEMPTS_COUNT))
    # print('Assertion valid (%d ms)'.format((total_t2 - total_t1) / ATTEMPTS_COUNT))
