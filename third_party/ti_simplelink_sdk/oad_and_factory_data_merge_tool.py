#!/usr/bin/env python

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Simple hexmerge script for combining MCUBoot, Factory Data, and App

This script provides a basic utility to combine the OAD application binary file with the MCUBoot hex file and Factory Data hex file. The output is a combined hex file that can be programmed on the target and run.

Run with:
    python oad_and_factory_data_merge_tool.py <App hex (CC13x4)> <MCUBoot (CC13x4) hex> <Factory data hex> <output>
"""

import sys

import intelhex

oad_bin_file = sys.argv[1]
bim_hex_file = sys.argv[2]
factory_data = sys.argv[3]
combined_hex = sys.argv[4]

# merge binary executable with bim hex file and factory hex file
ota_image = intelhex.IntelHex()
if (oad_bin_file.endswith('hex')):
    ota_image.fromfile(oad_bin_file, format='hex')

else:
    ota_image.fromfile(oad_bin_file, format='bin')

bim_hex = intelhex.IntelHex()
bim_hex.fromfile(bim_hex_file, format='hex')

factory_data_hex = intelhex.IntelHex()
factory_data_hex.fromfile(factory_data, format='hex')

ota_image.merge(bim_hex, overlap='error')
ota_image.merge(factory_data_hex, overlap='replace')

ota_image.tofile(combined_hex, format='hex')
