#!/usr/bin/env python
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

import os
import subprocess
import sys
import textwrap

# Copy two dependencies to the base output dir.
subprocess.run(["cp", sys.argv[4], "."])
subprocess.run(["cp", sys.argv[5], "."])

# Create TestSomething.flashbundle.txt
flashbundle_contents = textwrap.dedent(f"""
    {os.path.basename(sys.argv[2])}
    {os.path.basename(sys.argv[3])}
    {os.path.basename(sys.argv[4])}
    {os.path.basename(sys.argv[5])}
""").strip()
try:
    with open(sys.argv[1], 'w') as flashbundle_file:
        flashbundle_file.write(flashbundle_contents)
except OSError as exception:
    print(exception, sys.stderr)
    sys.exit(1)
