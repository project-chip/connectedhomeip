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


def main():
    with xmlrpc.client.ServerProxy('http://' + IP + ':' + str(PORT) + '/', allow_none=True) as proxy:
        proxy.waitForMessage(sys.argv[1], sys.argv[2:])


if __name__ == "__main__":
    main()
