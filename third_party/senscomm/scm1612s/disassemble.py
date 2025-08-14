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

import sys
import json
import logging
import os
import subprocess

def main(in_file: str, out_file: str):
#    print(in_file)
#    print(out_file)
    cmd = ['riscv32-elf-objdump', '-S', '-d', in_file]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    outs, errs = p.communicate()
#    print(outs.decode())

    with open(out_file, "w") as f:
        f.write(outs.decode())

if __name__ == '__main__':
    main(in_file=sys.argv[1], out_file=sys.argv[2])
