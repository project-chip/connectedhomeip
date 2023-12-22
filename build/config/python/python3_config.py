#!/usr/bin/env python3

# Copyright (c) 2023 Project CHIP Authors
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

import json
import re
import subprocess


class Python3Config:

    @staticmethod
    def _get_config(*args):
        return subprocess.check_output(["python3-config", *args], text=True)

    def get_include_dirs(self):
        return [re.sub(r"^-I", "", x)
                for x in self._get_config("--includes").split()]

    def get_lib_dirs(self):
        return [re.sub(r"^-L", "", x)
                for x in self._get_config("--ldflags").split()
                if x.startswith("-L")]

    def get_libs(self):
        return [re.sub(r"^-l", "", x)
                for x in self._get_config("--libs").split()]

    def get_libs_embed(self):
        return [re.sub(r"^-l", "", x)
                for x in self._get_config("--libs", "--embed").split()]


if __name__ == '__main__':
    config = Python3Config()
    print(json.dumps({
        'include_dirs': config.get_include_dirs(),
        'lib_dirs': config.get_lib_dirs(),
        'libs': config.get_libs(),
        'libs_embed': config.get_libs_embed(),
    }))
