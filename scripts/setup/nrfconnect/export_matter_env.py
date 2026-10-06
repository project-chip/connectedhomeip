#!/usr/bin/env python3

#
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Write a cmd.exe script that applies the Matter environment from Git Bash."""

import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: export_matter_env.py <chip_root> <output_cmd>", file=sys.stderr)
        return 1

    chip_root = Path(sys.argv[1]).resolve()
    output_cmd = Path(sys.argv[2])
    chip_root_bash = chip_root.as_posix()
    bash_command = (
        f"source '{chip_root_bash}/scripts/activate.sh' >/dev/null 2>&1 && "
        "python3 -c \"import os; "
        "[print(f'{k}={v}') for k, v in os.environ.items()]\""
    )

    process = subprocess.run(
        ["bash", "-lc", bash_command],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    if process.returncode != 0:
        print(process.stdout, file=sys.stderr)
        print(
            "Failed to activate the Matter build environment via Git Bash.",
            file=sys.stderr,
        )
        return process.returncode

    lines = ["@echo off"]
    for line in process.stdout.splitlines():
        if "=" not in line:
            continue
        key, _, value = line.partition("=")
        value = value.replace("%", "%%")
        lines.append(f'set "{key}={value}"')

    output_cmd.write_text("\r\n".join(lines) + "\r\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
