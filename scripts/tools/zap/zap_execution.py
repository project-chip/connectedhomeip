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
import subprocess

from typing import Tuple


class ZapTool:
    def __init__(self):
        # Accepted environment variables, in order:
        #
        # ZAP_DEVELOPMENT_PATH - the path to a zap development environment. This is
        #                        a zap checkout, used for local development
        # ZAP_INSTALL_PATH     - the path where zap-cli exists. This is if zap-cli
        #                        is NOT in the current path
        if 'ZAP_DEVELOPMENT_PATH' in os.environ:
            self.zap_start = ['node', 'src-script/zap-start.js']
            self.working_directory = os.environ['ZAP_DEVELOPMENT_PATH']
        elif 'ZAP_INSTALL_PATH' in os.environ:
            self.zap_start = [os.path.join(os.environ['ZAP_INSTALL_PATH'], 'zap-cli')]
            self.working_directory = None
        else:
            self.zap_start = ['zap-cli']
            self.working_directory = None

    def run(self, cmd: str, *extra_args: Tuple[str]):
        try:
            subprocess.check_call(self.zap_start + [cmd] + list(extra_args), cwd=self.working_directory)
        except FileNotFoundError as e:
            print(f'FAILED TO EXECUTE ZAP GENERATION: {e.strerror} - "{e.filename}"')
            print('*'*80)
            print('* You may need to install zap. Please ensure one of these applies:')
            print('* - `zap-cli` is in $PATH. Install from https://github.com/project-chip/zap/releases')
            print('*   see docs/guides/BUILDING.md for details')
            print('* - `zap-cli` is in $ZAP_INSTALL_PATH. Use this option if you')
            print('*   installed zap but do not want to update $PATH')
            print('* - Point $ZAP_DEVELOPMENT_PATH to your local copy of zap that you')
            print('*   develop on (to use a developer build of zap)')
            print('*'*80)
            sys.exit(1)
