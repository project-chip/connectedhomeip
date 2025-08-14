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

#from runner import ShellRunner

#sys.path.append(os.path.abspath(os.path.dirname(__file__)))


# Supported log levels, mapping string values required for argument
# parsing into logging constants
#__LOG_LEVELS__ = {
#    'debug': logging.DEBUG,
#    'info': logging.INFO,
#    'warn': logging.WARN,
#    'fatal': logging.FATAL,
#}

def main(in_file: str, out_file: str):
#    print(in_file)
#    print(out_file)
    cmd = ['riscv32-elf-nm', '-S', in_file]
    p1 = subprocess.Popen(cmd, stdout=subprocess.PIPE)

    cmd = ['grep', '-v', '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)']
    p2 = subprocess.Popen(cmd, stdin=p1.stdout, stdout=subprocess.PIPE)
    p1.stdout.close()

    cmd = ['sort']
    p3 = subprocess.Popen(cmd, stdin=p2.stdout, stdout=subprocess.PIPE)
    p2.stdout.close()

    outs, errs = p3.communicate()
#    print(outs.decode())

    with open(out_file, "w") as f:
        f.write(outs.decode())

if __name__ == '__main__':
    main(in_file=sys.argv[1], out_file=sys.argv[2])
