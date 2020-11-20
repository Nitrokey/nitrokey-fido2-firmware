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
import logging

logging.basicConfig(format='* %(relativeCreated)6dms %(filename)s:%(lineno)d %(message)s', level=logging.INFO)
log = logging.getLogger('comm')


def set_debug_messages2(a: int):
    if a == 1:
        logging.basicConfig(format='* %(relativeCreated)6dms %(filename)s:%(lineno)d %(message)s', level=logging.INFO)
    elif a == 0:
        logging.basicConfig(level=logging.INFO, format='%(asctime)s %(message)s',
                            handlers=[logging.FileHandler("activation.log")])
        # handlers=[logging.FileHandler("activation.log"), logging.StreamHandler()])
