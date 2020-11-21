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
import os

import cbor
import pytest
from pynitrokey.fido2.client import NKFido2Client as NKFido2Client

from webcrypt.helpers import log_data

REAL_HARDWARE = False


def pytest_addoption(parser):
    parser.addoption(
        "--hardware", action="store_true", help="Run test on hardware"
    )


@pytest.fixture
def run_on_hardware(request):
    global REAL_HARDWARE
    REAL_HARDWARE = request.config.getoption("--hardware")
    return REAL_HARDWARE


@pytest.fixture(scope='session')
def nkfido2_client(request) -> NKFido2Client:
    import pynitrokey
    REAL_HARDWARE = request.config.getoption("--hardware")
    if not REAL_HARDWARE:
        pynitrokey.fido2.force_udp_backend()
    nkfido2_client = pynitrokey.fido2.client.NKFido2Client()
    nkfido2_client.find_device()
    log_data(f'\nExchange selected: {nkfido2_client.exchange}\n')
    return nkfido2_client


def cbor_dumps(x) -> bytes:
    return cbor.dumps(x)


def cbor_loads(x):
    return cbor.loads(x)


TEST_DATA_SMALL = dict(ww=b'ww', a=b'A' * 5)
TEST_DATA = dict(ww=b'ww', xx=b'xx', cc=b'cc')


@pytest.fixture
def test_data() -> dict:
    return TEST_DATA


def fixture_data_big() -> dict:
    t = TEST_DATA.copy()
    t['Key111'] = b'A' * 300
    t['Key222'] = b'B' * 300
    return t


def fixture_data_random() -> dict:
    t = TEST_DATA.copy()
    t['Key111'] = os.urandom(150)
    t['Key222'] = os.urandom(150)
    t['xxx'] = os.urandom(30)
    return t

