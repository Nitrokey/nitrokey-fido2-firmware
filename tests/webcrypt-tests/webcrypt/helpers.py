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
from collections import Counter
from pprint import pprint

from fido2 import cbor


def compare_cbor_dict(read_data_bytes: bytes, test_data: dict) -> None:
    assert read_data_bytes, "Read data are empty"
    print(f'Record stats: CBOR length {len(read_data_bytes)}')
    read_data = cbor.decode(read_data_bytes)
    print(Counter(read_data_bytes))
    pprint(read_data)
    assert not isinstance(read_data, int)
    for k, v in test_data.items():
        k: bytes
        if k.startswith('_'): continue
        assert read_data[k] == v
    assert '_TP' not in read_data
