#!/usr/bin/env -S python3 -B

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
