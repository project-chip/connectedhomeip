#!/usr/bin/env python
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

"""
Invokes the setUpAndroidDeps Gradle task, and formats the outputted BUILD.gn.
"""

import os
import subprocess
import sys


def main():
    chip_root = os.getenv('PW_PROJECT_ROOT')
    android_deps_dir = os.path.join(chip_root, 'third_party/android_deps')
    gradlew = 'gradlew' if sys.platform != 'win32' else 'gradlew.bat'
    gradle_executable = os.path.join(android_deps_dir, gradlew)
    subprocess.check_call(
        [gradle_executable, '-p', android_deps_dir, 'setUpAndroidDeps'])
    subprocess.check_call(
        ['gn', 'format', os.path.join(android_deps_dir, 'BUILD.gn')])


if __name__ == '__main__':
    sys.exit(main())
