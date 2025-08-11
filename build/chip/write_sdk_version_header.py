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
import os
import subprocess

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


def git_version() -> str:
    # Define the Git command to get a descriptive version string:
    # --tags: use annotated tags if available
    # --always: fallback to commit hash if no tags are found
    cmd = ["git", "describe", "--always", "--tags"]

    return subprocess.check_output(cmd).decode("utf-8").strip()


def spec_version(root) -> str:
    spec_version_file = os.path.join(root, "SPECIFICATION_VERSION")
    with open(spec_version_file, "r") as spec_version:
        return spec_version.read().strip()


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate files with the SDK version.")
    parser.add_argument("--output", required=True, help="Name of the output files")
    parser.add_argument("--gen-dir", required=True, help="Directory where the output file should be created")
    parser.add_argument("--root", required=True, help="Project root path")
    parser.add_argument("--git", action="store_true", help="Use version obtained from git")

    args = parser.parse_args()

    os.makedirs(args.gen_dir, exist_ok=True)

    if args.git:
        try:
            ver = git_version()
        except (subprocess.CalledProcessError, FileNotFoundError):
            # In case Git is not installed, fall back to version from spec file
            ver = spec_version(args.root)
    else:
        ver = spec_version(args.root)

    output = os.path.join(args.gen_dir, '{}.h'.format(args.output))
    content = HEADER + '#pragma once\nextern const char* const CHIP_SDK_VERSION;\n'

    # Do not write to the file if it exists to prevent relinking
    if not os.path.exists(output):
        with open(output, "w") as header:
            header.write(content)

    output = os.path.join(args.gen_dir, '{}.cpp'.format(args.output))
    content = HEADER + '#include "{}.h"\nconst char* const CHIP_SDK_VERSION = "{}";\n'.format(args.output, ver)

    # Check if the file already exists and has the same content
    if os.path.exists(output):
        with open(output, "r") as cpp:
            if content == cpp.read():
                # If version didn't change, skip writing to avoid relinking
                return

    with open(output, "w") as cpp:
        cpp.write(content)


if __name__ == "__main__":
    main()
