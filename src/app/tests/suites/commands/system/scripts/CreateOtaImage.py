#!/usr/bin/env -S python3 -B

# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import os
import subprocess
import sys

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', '..', '..', '..'))

otaImageFilePath = sys.argv[1]
rawImageFilePath = sys.argv[2]
rawImageContent = ' '.join(sys.argv[3:])


def main():
    # Write the raw image content
    with open(rawImageFilePath, 'w') as rawFile:
        rawFile.write(rawImageContent)

    # Add an OTA header to the raw file
    otaImageTool = DEFAULT_CHIP_ROOT + '/src/app/ota_image_tool.py'
    cmd = [otaImageTool, 'create', '-v', '0xDEAD', '-p', '0xBEEF', '-vn', '2',
           '-vs', "2.0", '-da', 'sha256', rawImageFilePath, otaImageFilePath]
    s = subprocess.Popen(cmd)
    s.wait()
    if s.returncode != 0:
        raise Exception('Cannot create OTA image file')


if __name__ == "__main__":
    main()
