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

# sys.argv[1] contains the key to the apps register
with xmlrpc.client.ServerProxy('http://' + IP + ':' + str(PORT) + '/', allow_none=True) as proxy:
    proxy.reboot(sys.argv[1])
