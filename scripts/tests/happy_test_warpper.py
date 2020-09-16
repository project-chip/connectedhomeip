#!/usr/bin/env python

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

# This is a warpper for running happy tests

import os
import sys
import argparse

CHIP_PATH = os.path.realpath(os.path.join(
    os.path.dirname(__file__), "../.."))
HAPPY_TEST_PATH = os.path.join(CHIP_PATH, "src/test_driver/happy/tests")

test_environ = os.environ.copy()

parser = argparse.ArgumentParser(description='Warpper to run happy tests')
parser.add_argument('test_script', type=str,
                    help='The name of test')
parser.add_argument('--test-bin-dir', dest='bin_dir', type=str, nargs='?', default='.',
                    help='The path of test binaries')
parser.add_argument('--ci', dest='ci', type=bool, nargs='?', default=False,
                    help='Set this if running script under venv but happy is installed globally')

if __name__ == '__main__':
    args = parser.parse_args()
    print(args, file=sys.stderr)
    print(test_environ, file=sys.stderr)
    # GN will run Python in venv, which will break happy test
    if args.ci:
        if test_environ.get("VIRTUAL_ENV", None) != None:
            del test_environ["VIRTUAL_ENV"]
        test_environ["PATH"] = ":".join([s for s in test_environ.get(
            "PATH", "").split(":") if not os.path.realpath(s).startswith(os.path.realpath(os.path.join(CHIP_PATH, "third_party/pigweed/repo")))])
    test_environ["TEST_BIN_DIR"] = args.bin_dir
    os.execvpe("python3", ["python3", args.test_script], test_environ)
