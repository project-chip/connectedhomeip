#
#    Copyright (c) 2021-2025 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
import json
import os
import shutil
import subprocess
import traceback

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../../..'))


def getClangFormatBinaryChoices():
    """
    Returns an ordered list of paths that may be suitable clang-format versions
    """
    PW_CLANG_FORMAT_PATH = 'cipd/packages/pigweed/bin/clang-format'

    if 'PW_ENVIRONMENT_ROOT' in os.environ:
        yield os.path.join(os.environ["PW_ENVIRONMENT_ROOT"], PW_CLANG_FORMAT_PATH)

    dot_name = os.path.join(CHIP_ROOT_DIR, '.environment', PW_CLANG_FORMAT_PATH)
    if os.path.exists(dot_name):
        yield dot_name

    os_name = shutil.which('clang-format')
    if os_name:
        yield os_name


def getClangFormatBinary():
    """Fetches the clang-format binary that is to be used for formatting.

    Tries to figure out where the pigweed-provided binary is (via cipd)
    """
    for binary in getClangFormatBinaryChoices():
        # Running the binary with `--version` yields a string of the form:
        # "Fuchsia clang-format version 17.0.0 (https://llvm.googlesource.com/llvm-project 6d667d4b261e81f325756fdfd5bb43b3b3d2451d)"
        #
        # the SHA at the end generally should match pigweed version

        try:
            version_string = subprocess.check_output([binary, '--version']).decode('utf8')

            pigweed_config = json.load(
                open(os.path.join(CHIP_ROOT_DIR, 'third_party/pigweed/repo/pw_env_setup/py/pw_env_setup/cipd_setup/pigweed.json')))
            clang_config = [p for p in pigweed_config['packages'] if p['path'].startswith('fuchsia/third_party/clang/')][0]

            # Tags should be like:
            #   ['git_revision:895b55537870cdaf6e4c304a09f4bf01954ccbd6']
            prefix, sha = clang_config['tags'][0].split(':')

            if sha not in version_string:
                print('WARNING: clang-format may not be the right version:')
                print('   PIGWEED TAG:    %s' % clang_config['tags'][0])
                print('   ACTUAL VERSION: %s' % version_string)
        except Exception:
            print("Failed to validate clang version.")
            traceback.print_last()

        return binary

    raise Exception('Could not find a suitable clang-format')
