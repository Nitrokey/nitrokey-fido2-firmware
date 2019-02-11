import os
import time
from typing import List, Any
from logger import print

from fido2.client import ClientError
from fido2.ctap import CtapError


def helper_ctap_reset(device):
    try:
        device.ctap.reset()
    except CtapError as e:
        print('Warning, reset failed: ', e)


def helper_test_fido2(device, PIN: str = None):
    creds: List[Any] = []
    exclude_list: List[Any] = []
    rp = {'id': device.host, 'name': 'ExaRP'}
    user = {'id': b'usee_od', 'name': 'AB User'}
    challenge: str = 'Y2hhbGxlbmdl'

    # make two fake IDs for exclude list
    device.helper_populate_exclude_list(exclude_list)

    # test make credential
    print('make 3 credentials')
    device.helper_make_credentials(PIN, challenge, creds, rp, user)
    print('PASS')

    if PIN is not None:
        print('make credential with wrong pin code')
        device.helper_make_credential_with_wrong_PIN(PIN, challenge, rp, user)
        print('PASS')

    print('make credential with exclude list')
    cred = device.helper_make_credential_with_exclude_list(PIN, challenge, creds, exclude_list, rp, user)
    print('PASS')

    print('make credential with exclude list including real credential')
    device.helper_make_credential_with_exclude_list_real(PIN, challenge, cred, exclude_list, rp, user)
    print('PASS')

    for i, x in enumerate(creds):
        print('get assertion %d' % i)
        allow_list = device.helper_get_one_assertion(PIN, challenge, rp, x)
        print('PASS')

    if PIN is not None:
        print('get assertion with wrong pin code')
        device.helper_get_assertion_wrong_pin(PIN, allow_list, challenge, rp)
        print('PASS')

    print('get multiple assertions')
    device.helper_get_multiple_assertions(PIN, challenge, creds, i, rp)
    print('PASS')








class Helper(object):
    def test_fido2_simple(self, pin_token=None):
        creds = []
        exclude_list = []
        rp = {'id': self.host, 'name': 'ExaRP'}
        user = {'id': b'usee_od', 'name': 'AB User'}
        challenge = 'Y2hhbGxlbmdl'
        PIN = pin_token

        self.helper_populate_exclude_list(exclude_list)

        print('MC')
        t1 = time.time() * 1000
        attest, data = self.client.make_credential(
            rp, user, challenge, pin=PIN, exclude_list=[]
        )
        t2 = time.time() * 1000
        attest.verify(data.hash)
        print('Register valid (%d ms)' % (t2 - t1))

        cred = attest.auth_data.credential_data
        creds.append(cred)

        allow_list = [{'id': creds[0].credential_id, 'type': 'public-key'}]
        t1 = time.time() * 1000
        assertions, client_data = self.client.get_assertion(
            rp['id'], challenge, allow_list, pin=PIN
        )
        t2 = time.time() * 1000
        assertions[0].verify(client_data.hash, creds[0].public_key)

        print('Assertion valid (%d ms)' % (t2 - t1))


    def helper_get_one_assertion(self, PIN, challenge, rp, x):
        allow_list = [{'id': x.credential_id, 'type': 'public-key'}]
        assertions, client_data = self.client.get_assertion(
            rp['id'], challenge, allow_list, pin=PIN
        )
        assertions[0].verify(client_data.hash, x.public_key)
        return allow_list

    def helper_get_assertion_wrong_pin(self, PIN, allow_list, challenge, rp):
        try:
            assertions, client_data = self.client.get_assertion(
                rp['id'], challenge, allow_list, pin=PIN + ' '
            )
        except CtapError as e:
            assert e.code == CtapError.ERR.PIN_INVALID
        except ClientError as e:
            assert e.cause.code == CtapError.ERR.PIN_INVALID

    def helper_get_multiple_assertions(self, PIN, challenge, creds, i, rp):
        allow_list = [{'id': x.credential_id, 'type': 'public-key'} for x in creds]
        assertions, client_data = self.client.get_assertion(
            rp['id'], challenge, allow_list, pin=PIN
        )
        for ass, cred in zip(assertions, creds):
            i += 1
            ass.verify(client_data.hash, cred.public_key)
            print('%d verified' % i)

    def helper_make_credential_with_exclude_list_real(self, PIN, challenge, cred, exclude_list, rp, user):
        real_excl = [{'id': cred.credential_id, 'type': 'public-key'}]
        try:
            attest, data = self.client.make_credential(
                rp, user, challenge, pin=PIN, exclude_list=exclude_list + real_excl
            )
            raise RuntimeError('Exclude list did not return expected error')
        except CtapError as e:
            assert e.code == CtapError.ERR.CREDENTIAL_EXCLUDED
        except ClientError as e:
            assert e.cause.code == CtapError.ERR.CREDENTIAL_EXCLUDED

    def helper_make_credential_with_exclude_list(self, PIN, challenge, creds, exclude_list, rp, user):
        attest, data = self.client.make_credential(
            rp, user, challenge, pin=PIN, exclude_list=exclude_list
        )
        attest.verify(data.hash)
        cred = attest.auth_data.credential_data
        creds.append(cred)
        return cred

    def helper_make_credential_with_wrong_PIN(self, PIN, challenge, rp, user):
        try:
            attest, data = self.client.make_credential(
                rp, user, challenge, pin=PIN + ' ', exclude_list=[]
            )
        except CtapError as e:
            assert e.code == CtapError.ERR.PIN_INVALID
        except ClientError as e:
            assert e.cause.code == CtapError.ERR.PIN_INVALID

    def helper_make_credentials(self, PIN, challenge, creds, rp, user):
        for i in range(0, 3):
            attest, data = self.client.make_credential(
                rp, user, challenge, pin=PIN, exclude_list=[]
            )
            attest.verify(data.hash)
            cred = attest.auth_data.credential_data
            creds.append(cred)
            print(cred)

    @staticmethod
    def helper_populate_exclude_list(exclude_list: List[dict]):
        fake_id1 = os.urandom(150)
        fake_id2 = os.urandom(73)
        exclude_list.append({'id': fake_id1, 'type': 'public-key'})
        exclude_list.append({'id': fake_id2, 'type': 'public-key'})

    def helper_test_fido2(self, PIN: str = None):
        creds: List[Any] = []
        exclude_list: List[Any] = []
        rp = {'id': self.host, 'name': 'ExaRP'}
        user = {'id': b'usee_od', 'name': 'AB User'}
        challenge: str = 'Y2hhbGxlbmdl'

        # make two fake IDs for exclude list
        self.helper_populate_exclude_list(exclude_list)

        # test make credential
        print('make 3 credentials')
        self.helper_make_credentials(PIN, challenge, creds, rp, user)
        print('PASS')

        if PIN is not None:
            print('make credential with wrong pin code')
            self.helper_make_credential_with_wrong_PIN(PIN, challenge, rp, user)
            print('PASS')

        print('make credential with exclude list')
        cred = self.helper_make_credential_with_exclude_list(PIN, challenge, creds, exclude_list, rp, user)
        print('PASS')

        print('make credential with exclude list including real credential')
        self.helper_make_credential_with_exclude_list_real(PIN, challenge, cred, exclude_list, rp, user)
        print('PASS')

        for i, x in enumerate(creds):
            print('get assertion %d' % i)
            allow_list = self.helper_get_one_assertion(PIN, challenge, rp, x)
            print('PASS')

        if PIN is not None:
            print('get assertion with wrong pin code')
            self.helper_get_assertion_wrong_pin(PIN, allow_list, challenge, rp)
            print('PASS')

        print('get multiple assertions')
        self.helper_get_multiple_assertions(PIN, challenge, creds, i, rp)
        print('PASS')

    def helper_ctap_reset(self):
        try:
            self.ctap.reset()
        except CtapError as e:
            print('Warning, reset failed: ', e)

    def helper_test_PIN_management(self, PIN: str):
        print('Set a pin code')
        self.client.pin_protocol.set_pin(PIN)
        print('PASS')

        print('Illegally set pin code again')
        try:
            self.client.pin_protocol.set_pin(PIN)
        except CtapError as e:
            assert e.code == CtapError.ERR.NOT_ALLOWED
        print('PASS')

        print('Change pin code')
        PIN2: str = PIN + '_pin2'
        self.client.pin_protocol.change_pin(PIN, PIN2)
        PIN = PIN2
        print('PASS')

        print('Change pin code using wrong pin')
        try:
            self.client.pin_protocol.change_pin(PIN.replace('a', 'b'), '1234')
        except CtapError as e:
            assert e.code == CtapError.ERR.PIN_INVALID
        print('PASS')

        return PIN