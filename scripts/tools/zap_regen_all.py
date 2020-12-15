#!/usr/bin/env python3
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import sys
import os
from pathlib import Path

# Check python version
if sys.version_info[0] < 3:
    print("Must use Python 3. Current version is " + str(sys.version_info[0]))
    exit(1)

# Check if we are in top of CHIP folder
scripts_path = os.path.join(os.getcwd(), 'scripts/tools/zap_regen_all.py')
if not os.path.exists(scripts_path):
    print(os.getcwd())
    print('This script must be called from the root of the chip directory')
    exit(1)

os.system("./scripts/tools/zap_generate_chip.sh")

for path in Path('./examples').rglob('*.zap'):
    os.system("./scripts/tools/zap_generate.sh " + str(path))
