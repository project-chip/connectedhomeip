#!/bin/sh

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Print CHIP logs on stdout
dlogutil CHIP &

# Install lighting Matter app
pkgcmd -i -t tpk -p /mnt/chip/org.tizen.matter.*
# Launch lighting Matter app
app_launcher -s org.tizen.matter.example.lighting

# TEST: pair app using network commissioning
/mnt/chip/chip-tool pairing onnetwork 1 20202021
# TEST: turn on light
/mnt/chip/chip-tool onoff on 1 1
# TEST: turn off light
/mnt/chip/chip-tool onoff off 1 1
