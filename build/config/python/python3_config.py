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
import os
import re
import shutil
import subprocess


def is_subpath(path: str, parent: str):
    """Check if path is a subpath of parent."""
    try:
        return os.path.commonpath([path, parent]) == parent
    except ValueError:
        return False


def find_command(command: str, exclude: list[str]):
    """Extended shutil.which() with exclude parameters."""
    for path in os.getenv("PATH", os.defpath).split(os.pathsep):
        if any(is_subpath(path, x) for x in exclude):
            continue
        app = shutil.which(command, path=path)
        if app is not None:
            return app
    return None


class Python3Config:

    # List of python3-config commands to try in order. This is needed because
    # not all environments have python3-config symlinked in the PATH.
    COMMANDS = [
        "python3-config",
        "python3.12-config",
        "python3.11-config",
        "python3.10-config",
        "python3.9-config",
    ]

    def __init__(self):

        exclude = []

        # Exclude directory with python3 installed by CIPD manager in Matter
        # build environment. The python3-config provided by CIPD is broken - it
        # returns wrong include/link paths and does not provide python shared
        # library.
        if os.getenv("PW_PYTHON_CIPD_INSTALL_DIR"):
            exclude.append(os.getenv("PW_PYTHON_CIPD_INSTALL_DIR"))

        for command in self.COMMANDS:
            command = find_command(command, exclude=exclude)
            if command is not None:
                self._python_config_exe = command
                break
        else:
            raise RuntimeError("Could not find python3-config in PATH")

    def _get_config(self, *args):
        return subprocess.check_output([self._python_config_exe, *args], text=True)

    def get_cflags(self):
        return [x for x in self._get_config("--cflags").split()
                if not x.startswith("-I")]

    def get_include_dirs(self):
        return [re.sub(r"^-I", "", x)
                for x in self._get_config("--includes").split()]

    def get_lib_dirs(self):
        return [re.sub(r"^-L", "", x)
                for x in self._get_config("--ldflags").split()
                if x.startswith("-L")]

    def get_libs(self):
        return [re.sub(r"^-l", "", x)
                for x in self._get_config("--libs").split()
                if x.startswith("-l")]

    def get_libs_embed(self):
        return [re.sub(r"^-l", "", x)
                for x in self._get_config("--libs", "--embed").split()
                if x.startswith("-l")]


if __name__ == '__main__':
    config = Python3Config()
    print(json.dumps({
        'cflags': config.get_cflags(),
        'include_dirs': config.get_include_dirs(),
        'lib_dirs': config.get_lib_dirs(),
        'libs': config.get_libs(),
        'libs_embed': config.get_libs_embed(),
    }))
