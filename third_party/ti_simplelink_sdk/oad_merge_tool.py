#!/usr/bin/env python

# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

"""Simple hexmerge script for combining the BIM and App

This script provides a basic utility to combine the OAD application binary file with the Boot Image Manager hex file. The output is a combined hex file that can be programmed on the target and run.

Run with:
    python oad_merge_tool.py <App bin> <BIM hex> <output>
"""

import sys

import intelhex

oad_bin_file = sys.argv[1]
bim_hex_file = sys.argv[2]
combined_hex = sys.argv[3]

# merge binary executable with bim hex file
ota_image = intelhex.IntelHex()
ota_image.fromfile(oad_bin_file, format='bin')

bim_hex = intelhex.IntelHex()
bim_hex.fromfile(bim_hex_file, format='hex')

ota_image.merge(bim_hex)

ota_image.tofile(combined_hex, format='hex')
