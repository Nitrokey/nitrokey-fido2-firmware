
import pytest
from fido2.client import Fido2Client
from fido2.ctap1 import CTAP1
from fido2.ctap2 import *
from fido2.hid import CtapHidDevice
from fido2.utils import Timeout

from helpers import Helper


@pytest.fixture(scope="module")
def device(request=None):
    d = Device()
    d.find_device()
    return d



def ForceU2F(client, device):
    client.ctap = CTAP1(device)
    client.pin_protocol = None
    client._do_make_credential = client._ctap1_make_credential
    client._do_get_assertion = client._ctap1_get_assertion


class Device(Helper):
    def __init__(self,):
        self.origin = 'https://examplo.org'
        self.host = 'examplo.org'

    def find_device(self,):
        dev_list = list(CtapHidDevice.list_devices())
        # print(dev_list)
        if not dev_list:
            return
        dev = dev_list[0]
        if not dev:
            raise RuntimeError('No FIDO device found')
        self.dev = dev
        self.client = Fido2Client(dev, self.origin)
        self.ctap = self.client.ctap2

        # consume timeout error
        # cmd,resp = self.recv_raw()

    def send_data(self, cmd, data):
        if type(data) != type(b''):
            data = struct.pack('%dB' % len(data), *[ord(x) for x in data])
        with Timeout(1.0) as event:
            return self.dev.call(cmd, data, event)

    def send_raw(self, data, cid=None):
        if cid is None:
            cid = self.dev._dev.cid
        elif type(cid) != type(b''):
            cid = struct.pack('%dB' % len(cid), *[ord(x) for x in cid])
        if type(data) != type(b''):
            data = struct.pack('%dB' % len(data), *[ord(x) for x in data])
        data = cid + data
        l = len(data)
        if l != 64:
            pad = '\x00' * (64 - l)
            pad = struct.pack('%dB' % len(pad), *[ord(x) for x in pad])
            data = data + pad
        data = list(data)
        assert len(data) == 64
        self.dev._dev.InternalSendPacket(Packet(data))

    def cid(self,):
        return self.dev._dev.cid

    def set_cid(self, cid):
        if type(cid) not in [type(b''), type(bytearray())]:
            cid = struct.pack('%dB' % len(cid), *[ord(x) for x in cid])
        self.dev._dev.cid = cid

    def recv_raw(self,):
        with Timeout(1.0) as t:
            cmd, payload = self.dev._dev.InternalRecv()
        return cmd, payload

    def check_error(self, data, err=None):
        assert len(data) == 1
        if err is None:
            if data[0] != 0:
                raise CtapError(data[0])
        elif data[0] != err:
            raise ValueError('Unexpected error: %02x' % data[0])
