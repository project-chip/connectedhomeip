#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
"""Generate script to flash or erase a device."""

import importlib
import sys

platform = importlib.import_module(sys.argv[1] + '_firmware_utils')
del sys.argv[1]

if __name__ == '__main__':
    sys.exit(platform.Flasher().make_wrapper(sys.argv))
