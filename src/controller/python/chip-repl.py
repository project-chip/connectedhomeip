#!/usr/bin/env python

#
#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

from IPython import embed
import chip
import chip.logging
import coloredlogs
import logging


def main():
    # The chip imports at the top level will be visible in the ipython REPL.

    coloredlogs.install(level='DEBUG')
    chip.logging.RedirectToPythonLogging()

    # trace/debug logging is not friendly to an interactive console. Only keep errors.
    logging.getLogger().setLevel(logging.ERROR)

    embed(header='''
Welcome to the CHIP python REPL utilty.

Usage examples:

######## Enable detailed logging if needed #########

import logging
logging.getLogger().setLevel(logging.DEBUG)

######## List available BLE adapters #########
import chip.ble
print(chip.ble.GetAdapters())

######## Discover chip devices #########
import chip.ble

chip.ble.DiscoverAsync(2000, lambda *x: print('Discovered: %r' % (x,)), lambda: print('Done'))

for device in chip.ble.DiscoverSync(2000):
    print(device)

####### Commision a BLE device #########

import chip.ble.commissioning

device = chip.ble.commissioning.Connect(discriminator=3840, pin=20202021)
if device.needsNetworkCredentials:
  device.ConnectToWifi("ssid", "password")

######## Thread provisioning ########

import chip.ble.commissioning
device = chip.ble.commissioning.Connect(discriminator=3840, pin=20202021)

# Thread data is an opaque blob, but it can be build with internal constructs
# starting from a memset(0) equivalent
from chip.internal.thread import ThreadNetworkInfo

data = ThreadNetworkInfo.parse(b'\\x00'*ThreadNetworkInfo.sizeof())
data.NetworkName = "OpenThread"
data.ExtendedPANId = b"\\xde\\xad\\x00\\xbe\\xef\\x00\\xca\\xfe"
data.MasterKey = b"\\x00\\x11\\x22\\x33\\x44\\x55\\x66\\x77\\x88\\x99\\xAA\\xBB\\xCC\\xDD\\xEE\\xFF"
data.PANId = 0xabcd
data.Channel = 15


if device.needsNetworkCredentials:
  device.ConnectToThread(ThreadNetworkInfo.build(data))

######## Node discovery ########

import chip.discovery

chip.discovery.FindAddressAsync(123, 456, lambda x: print("%r", x))

print(chip.discovery.FindAddress(123, 456)

    '''.strip())


if __name__ == "__main__":
    main()
