#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

'''Python wrapper to call make based builds within the SDK
'''
import subprocess
import sys

subprocess.run(['make', '-j'] + sys.argv[1:], check=True)
