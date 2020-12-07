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

# This is a wrapper for running happy tests.

import argparse
import os
import subprocess
import sys
import tempfile

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
parser.add_argument('--silent', dest='silent', type=bool, nargs='?', default=False,
                    help='Set this will mute output when the test finished successfully')

if __name__ == '__main__':
    if os.getuid() != 0:
        os.execvpe("unshare", ["unshare", "--map-root-user",
                               "-n", "-m", "python3"] + sys.argv, test_environ)
        print("Failed to run script in new user namespace", file=sys.stderr)
        exit(1)
    if os.system("mount --make-private /") != 0 or os.system("mount -t tmpfs tmpfs /run") != 0:
        print("Failed to setup private mount points", file=sys.stderr)
        exit(1)
    args = parser.parse_args()
    # GN will run Python in venv, which will break happy test
    if args.ci:
        test_environ["HAPPY_LOG_DIR"] = "/tmp/happy_test_logs"
    test_environ["TEST_BIN_DIR"] = args.bin_dir
    test_environ["HAPPY_MAIN_CONFIG_FILE"] = os.path.realpath(
        os.path.join(CHIP_PATH, "src/test_driver/happy/conf/main_conf.json"))
    if args.silent:
        fp, fname = tempfile.mkstemp()
        run_res = subprocess.run(["python3", args.test_script],
                                 stdout=fp, stderr=fp, env=test_environ)
        if run_res.returncode != 0:
            with open(fname, 'rb') as test_output:
                os.write(sys.stderr.fileno(), test_output.read())
        exit(run_res.returncode)
    else:
        os.execvpe("python3", ["python3", args.test_script], test_environ)
