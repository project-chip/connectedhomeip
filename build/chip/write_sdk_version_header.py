#!/usr/bin/env python3

# Copyright (c) 2025 Project CHIP Authors
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
import sys
import subprocess
import uuid
import os

HEADER = """/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// This file is generated automatically by write_sdk_version_header.py
"""

def main() -> int:
    parser = argparse.ArgumentParser(description="Generate header file with the SDK version.")
    parser.add_argument("--output", required=True, help="Name of the output header file")
    parser.add_argument("--gen-dir", required=True, help="Directory where the output file should be created")

    args = parser.parse_args()

    output = os.path.join(args.gen_dir, args.output)
    os.makedirs(args.gen_dir, exist_ok=True)

    cmd = ["git", "describe", "--always", "--tags"]
    try:
        cmd = ["git", "describe", "--always", "--tags"]
        git_ver = subprocess.check_output(cmd).decode("utf-8").strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        git_ver = ""

    with open(output, "w") as header:
        header.write(HEADER)
        header.write(f'#define CHIP_SDK_VERSION "{git_ver}"\n')

    return 0

main()
