#!/usr/bin/env -S python3 -B

# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import sys
import xmlrpc.client

IP = '127.0.0.1'
PORT = 9000

if sys.platform == 'linux':
    IP = '10.10.10.5'

# Passing in sys.argv[2:] gets rid of the script name and key to the apps register. The remaining
# values in the list are key-value pairs, e.g. [option1, value1, option2, value2, ...]
with xmlrpc.client.ServerProxy('http://' + IP + ':' + str(PORT) + '/', allow_none=True) as proxy:
    proxy.start(sys.argv[1], sys.argv[2:])
