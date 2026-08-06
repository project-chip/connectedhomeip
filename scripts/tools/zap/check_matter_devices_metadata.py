#!/usr/bin/env python3
#
# Copyright (c) 2026 Project CHIP Authors
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

"""
Verifies that changes to matter-devices.xml are not manual edits.
When matter-devices.xml is modified, the Alchemy metadata in the header
(Alchemy version, Git describe/SHA, or Parameters) must also change.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

DEFAULT_TARGET = "src/app/zap-templates/zcl/data-model/chip/matter-devices.xml"


def check_metadata_changes(root: Path, target_file: str, diff_base: str) -> int:
    # Ensure git can run in current directory
    subprocess.run(["git", "config", "--global", "--add", "safe.directory", str(root)], cwd=root, check=False)

    # Check if target file was modified
    res = subprocess.run(
        ["git", "diff", "--name-only", diff_base, "--", target_file],
        capture_output=True, text=True, cwd=root
    )
    if not res.stdout.strip():
        print(f"No changes to {target_file}. Check passed.")
        return 0

    print(f"{target_file} was modified. Checking for Alchemy metadata changes...")

    diff_res = subprocess.run(
        ["git", "diff", diff_base, "--", target_file],
        capture_output=True, text=True, check=True, cwd=root
    )
    diff_text = diff_res.stdout

    has_alchemy_change = bool(re.search(r'^\+\s*Alchemy:', diff_text, re.MULTILINE))
    has_git_change = bool(re.search(r'^\+\s*Git:', diff_text, re.MULTILINE))
    has_params_change = bool(re.search(r'^\+\s*Parameters:', diff_text, re.MULTILINE))

    print(f"  Alchemy version changed: {has_alchemy_change}")
    print(f"  Spec Git version changed: {has_git_change}")
    print(f"  Parameters changed:      {has_params_change}")

    if not (has_alchemy_change or has_git_change or has_params_change):
        print(f"\nERROR: {target_file} was modified without updating the Alchemy, Git, or Parameters metadata in the header.")
        print(f"Manual edits to {target_file} are not allowed. Please regenerate it using Alchemy.")
        return 1

    print("Alchemy metadata was updated. Manual edit check passed.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify that changes to matter-devices.xml include updated Alchemy metadata."
    )
    parser.add_argument(
        "--root",
        default=".",
        help="Repository root directory (default: .)",
    )
    parser.add_argument(
        "--target-file",
        default=DEFAULT_TARGET,
        help=f"Target XML file to check (default: {DEFAULT_TARGET})",
    )
    parser.add_argument(
        "--diff-base",
        default="HEAD^",
        help="Git ref (branch, tag, or SHA) to diff against (default: HEAD^)",
    )
    args = parser.parse_args()

    root = Path(args.root).resolve()
    return check_metadata_changes(root, args.target_file, args.diff_base)


if __name__ == "__main__":
    sys.exit(main())
