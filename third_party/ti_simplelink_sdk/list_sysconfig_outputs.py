#!/usr/bin/env python3
# Copyright (c) 2020 Project CHIP Authors
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

"""Query SysConfig for its list of generated output files.

This script is called by GN's exec_script() at generation time. It invokes
the SysConfig CLI with --listGeneratedFiles and prints the basename of each
generated .c and .h file, one per line.

Usage:
    python list_sysconfig_outputs.py <sysconfig_root> [sysconfig_args...]

The arguments after <sysconfig_root> are passed directly to sysconfig_cli,
with --listGeneratedFiles automatically appended.
"""

import os
import subprocess
import sys


def main():
    if len(sys.argv) < 2:
        print("Usage: list_sysconfig_outputs.py <sysconfig_root> [args...]",
              file=sys.stderr)
        sys.exit(1)

    sysconfig_root = sys.argv[1]

    # Select OS-specific helper script
    if sys.platform.startswith('linux'):
        cli = os.path.join(sysconfig_root, 'sysconfig_cli.sh')
    elif sys.platform.startswith('win'):
        cli = os.path.join(sysconfig_root, 'sysconfig_cli.bat')
    else:
        cli = os.path.join(sysconfig_root, 'sysconfig_cli.sh')

    # Build command: cli + caller-provided args + --listGeneratedFiles
    cmd = [cli] + sys.argv[2:] + ['--listGeneratedFiles']

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print("SysConfig --listGeneratedFiles failed:\n" + e.stderr,
              file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print("SysConfig CLI not found at: " + cli, file=sys.stderr)
        sys.exit(1)

    # Parse output: one file path per line. SysConfig may output lines with a
    # status prefix on subsequent runs ("Unchanged /path/file.c...") or plain
    # file paths on the first run (when no output files exist yet).
    # Extract basenames and filter to .c and .h files only.
    # (.opt files are consumed via cflags, not as source files)
    for line in result.stdout.strip().splitlines():
        line = line.strip()
        if not line:
            continue

        # Strip optional status prefix ("Unchanged ", "Created ", "Updated ")
        # then take the last whitespace-separated token as the candidate path.
        if line.startswith(('Unchanged ', 'Created ', 'Updated ')):
            parts = line.split(None, 1)
            candidate = parts[1] if len(parts) > 1 else ''
        else:
            # First-run format: plain path or other line; take the last token.
            candidate = line.split()[-1]

        basename = os.path.basename(candidate.rstrip('.'))

        if basename.endswith('.c') or basename.endswith('.h'):
            print(basename)


if __name__ == '__main__':
    main()
