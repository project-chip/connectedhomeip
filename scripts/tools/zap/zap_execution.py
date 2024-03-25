# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import re
import subprocess
import sys
from typing import Tuple

# The version MUST be of the form `YYYY.M.D'
#
# Use scripts/tools/zap/version_update.py to manage ZAP versioning as many
# files may need updating for versions
#
MIN_ZAP_VERSION = '2024.3.14'


class ZapTool:
    def __init__(self):
        self.check_version = True

        # Accepted environment variables, in order:
        #
        # ZAP_DEVELOPMENT_PATH - the path to a zap development environment. This is
        #                        a zap checkout, used for local development
        # ZAP_INSTALL_PATH     - the path where zap-cli exists. This is if zap-cli
        #                        is NOT in the current path
        if 'ZAP_DEVELOPMENT_PATH' in os.environ:
            self.zap_start = ['node', 'src-script/zap-start.js']
            self.working_directory = os.environ['ZAP_DEVELOPMENT_PATH']
            # When using ZAP_DEVELOPMENT_PATH, we should generally not need to
            # setup a version (dirties the tree and conflicts with parallel
            # execution
            # HOWEVER, if the intent is exactly to check the version, this should not be done
            # and this set should be reverted
            os.environ['ZAP_SKIP_REAL_VERSION'] = '1'
            self.check_version = False  # cannot check versions without dirtying tree
        elif 'ZAP_INSTALL_PATH' in os.environ:
            self.zap_start = [os.path.join(
                os.environ['ZAP_INSTALL_PATH'], 'zap-cli')]
            self.working_directory = None
        else:
            self.zap_start = ['zap-cli']
            self.working_directory = None

    def version_check(self, min_version=None):
        if not min_version:
            min_version = MIN_ZAP_VERSION

        if not self.check_version:
            print("Unable to check version using %r. Assuming version is ok.", self.zap_start)
            return

        try:
            version = ''
            version_re = re.compile(r'Version:\s*(\d+\.\d+\.\d+)')

            for line in subprocess.check_output(self.zap_start + ['--version'], cwd=self.working_directory).splitlines():
                try:
                    m = version_re.match(line.decode('utf-8'))
                    if m:
                        version = m.group(1)
                        break
                except UnicodeDecodeError:
                    # Version line will not be unicode ... skip over odd lines
                    pass

            if not version:
                print(
                    f"Failed to find `Version: ....` line from {self.zap_start} --version")
                sys.exit(1)
        except FileNotFoundError as e:
            print(
                f'FAILED TO EXECUTE ZAP GENERATION: {e.strerror} - "{e.filename}"')
            print('*'*80)
            print('* You may need to install zap. Please ensure one of these applies:')
            print(
                '* - `zap-cli` is in $PATH. Install from https://github.com/project-chip/zap/releases')
            print('*   see docs/guides/BUILDING.md for details')
            print('* - `zap-cli` is in $ZAP_INSTALL_PATH. Use this option if you')
            print('*   installed zap but do not want to update $PATH')
            print('* - Point $ZAP_DEVELOPMENT_PATH to your local copy of zap that you')
            print('*   develop on (to use a developer build of zap)')
            print('*'*80)
            sys.exit(1)

        def parse_version_string(str):
            return list(map(lambda component: int(component), str.split('.')))

        if parse_version_string(version) < parse_version_string(MIN_ZAP_VERSION):
            print(f"Checking ZAP from {self.zap_start}:")
            print(
                f"  !!! Version validation failed: required at least {MIN_ZAP_VERSION}, got {version} instead")
            sys.exit(1)

    def run(self, cmd: str, *extra_args: Tuple[str]):
        try:
            subprocess.check_call(
                self.zap_start + [cmd] + list(extra_args), cwd=self.working_directory)
        except FileNotFoundError as e:
            print(
                f'FAILED TO EXECUTE ZAP GENERATION: {e.strerror} - "{e.filename}"')
            print('*'*80)
            print('* You may need to install zap. Please ensure one of these applies:')
            print(
                '* - `zap-cli` is in $PATH. Install from https://github.com/project-chip/zap/releases')
            print('*   see docs/guides/BUILDING.md for details')
            print('* - `zap-cli` is in $ZAP_INSTALL_PATH. Use this option if you')
            print('*   installed zap but do not want to update $PATH')
            print('* - Point $ZAP_DEVELOPMENT_PATH to your local copy of zap that you')
            print('*   develop on (to use a developer build of zap)')
            print('*'*80)
            sys.exit(1)
