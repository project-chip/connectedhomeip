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

"""Simple hexmerge script for combining the BIM, Factory Data, and App

This script provides a basic utility to combine the Matter application hex file and Factory Data hex file. The output is a combined hex file that can be programmed on the target and run.

Run with:
    python factory_data_merge_tool.py <Matter hex> <Factory data hex> <output>
"""

import sys

import intelhex

hex_file = sys.argv[1]
factory_data = sys.argv[2]
combined_hex = sys.argv[3]

# merge matter hex file and factory hex file
matter_hex = intelhex.IntelHex()
matter_hex.fromfile(hex_file, format='hex')

factory_data_hex = intelhex.IntelHex()
factory_data_hex.fromfile(factory_data, format='hex')

matter_hex.merge(factory_data_hex, overlap='replace')
matter_hex.tofile(combined_hex, format='hex')
