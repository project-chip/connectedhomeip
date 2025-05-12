#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
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

import subprocess
import os
import sys

# Get the compiler name from GN args or default to "gcc"
compiler = sys.argv[1] if len(sys.argv) > 1 else "gcc"

try:
    # Run the compiler to get crtbegin.o path
    output = subprocess.check_output([compiler, "-print-file-name=crtbegin.o"])
    path = output.decode().strip()

    # Return the directory path, properly GN-quoted
    dir_path = os.path.dirname(path)
    print(f'"{dir_path}"')  # 👈 key: quote the output for GN
    #print(dir_path)
except Exception as e:
    # Return a valid empty GN string to avoid lexing errors
    print('""')
    sys.exit(1)
