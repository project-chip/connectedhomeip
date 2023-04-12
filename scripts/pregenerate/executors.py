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

import logging
import os
import shlex
import subprocess


class ShellRunner:
    def run(self, cmd, cwd=None):
        logging.debug(f"Executing {cmd}")
        subprocess.check_call(cmd, cwd=cwd)

    def ensure_directory_exists(self, dir):
        os.makedirs(dir, exist_ok=True)


class DryRunner:
    def run(self, cmd, cwd=None):
        if cwd:
            logging.info(f"DRY-RUN: {shlex.join(cmd)} in {cwd}")
        else:
            logging.info(f"DRY-RUN: {shlex.join(cmd)}")

    def ensure_directory_exists(self, dir):
        logging.info(f"DRY-RUN: mkdir {dir}")
