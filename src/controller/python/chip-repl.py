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

def main():
    # The chip import at the top level will be visible in the ipython REPL.
    embed(header = '''
Welcome to the CHIP python REPL utilty.

Usage examples:

import chip.ble

######## List available BLE adapters #########
print(chip.ble.GetAdapters())

######## Discover chip devices #########

chip.ble.DiscoverAsync(2000, lambda *x: print('Discovered: %r' % (x,)), lambda: print('Done'))

for device in chip.ble.DiscoverSync(2000):
    print(device)

    '''.strip())

if __name__ == "__main__":
    main()