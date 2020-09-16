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
import argparse

CHIP_PATH = os.path.realpath(os.path.join(
    os.path.dirname(__file__), "../.."))
HAPPY_TEST_PATH = os.path.join(CHIP_PATH, "src/test_driver/happy/tests")
EXEC_ENVIRON = os.environ.copy()

parser = argparse.ArgumentParser(description='Warpper to run happy tests')
parser.add_argument('test_script', type=str,
                    help='The name of test')
parser.add_argument('--test-bin-dir', dest='bin_dir', type=str, nargs='?', default='.',
                    help='The path of test binaries')

if __name__ == '__main__':
    args = parser.parse_args()
    print(args)
    EXEC_ENVIRON["TEST_BIN_DIR"] = args.bin_dir
    os.execvpe("python3", ["python3", args.test_script], EXEC_ENVIRON)
