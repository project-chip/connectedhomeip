#!/usr/bin/env python
# Copyright (c) 2021 Project CHIP Authors
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
