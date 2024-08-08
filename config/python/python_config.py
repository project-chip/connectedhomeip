#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
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

import argparse
import json
import os
import pathlib
import shutil
import subprocess
from typing import Optional


class Config:
    def __init__(self, python_exe: Optional[pathlib.Path] = None):
        self._paths: Optional[dict] = None
        self._version: Optional[list] = None

        if python_exe:
            self.python_exe = python_exe
        else:
            cipd_install_dir = os.getenv("PW_PYTHON_CIPD_INSTALL_DIR")
            if not cipd_install_dir:
                raise RuntimeError("PW_PYTHON_CIPD_INSTALL_DIR not set")
            self.python_exe = os.path.join(cipd_install_dir, "bin", "python3")
            if not os.path.exists(self.python_exe):
                self.python_exe = shutil.which("python3")
                if not self.python_exe:
                    raise FileNotFoundError("Python3 not found")

    @property
    def paths(self) -> dict:
        if not self._paths:
            out = subprocess.check_output(
                [self.python_exe, "-c", "from sysconfig import get_paths; import json; print(json.dumps(get_paths()))"], text=True
            ).strip()
            self._paths = json.loads(out)
        return self._paths

    @property
    def version(self) -> list[str]:
        if not self._version:
            self._version = (
                subprocess.check_output(
                    [self.python_exe, "-c", "import sys; print(sys.version_info.major, sys.version_info.minor)"], text=True
                )
                .strip()
                .split(" ")
            )
        return self._version

    @property
    def include(self) -> str:
        return self.paths["include"]


def parse_args():
    parser = argparse.ArgumentParser(description="Generate Python config")
    parser.add_argument("--python", type=pathlib.Path, help="Python executable to use", required=False)
    return parser.parse_args()


def main():
    args = parse_args()
    config = Config(args.python)
    print(
        json.dumps(
            {
                "version": {
                    "major": config.version[0],
                    "minor": config.version[1],
                },
                "include": config.include,
            }
        )
    )


if __name__ == "__main__":
    main()
