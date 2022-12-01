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

import os
from pathlib import Path
import sys
import subprocess
import argparse

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


zapFilesToSkip = [
    # examples/chef/sample_app_util/test_files/sample_zap_file.zap is
    # not a real .zap file; it's input to generating .zap files.  So
    # the path to zcl.json in it is just wrong, and we should skip it.
    "examples/chef/sample_app_util/test_files/sample_zap_file.zap",
]


def getTargets():
    targets = []
    targets.extend([[str(filepath)]
                   for filepath in Path('./examples').rglob('*.zap')
                    if str(filepath) not in zapFilesToSkip])
    targets.extend([[str(filepath)]
                   for filepath in Path('./src/darwin').rglob('*.zap')
                    if str(filepath) not in zapFilesToSkip])
    targets.extend([[str(filepath)] for filepath in Path(
        './src/controller/data_model').rglob('*.zap')
        if str(filepath) not in zapFilesToSkip])
    return targets


def runArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Convert all .zap files to the current zap version')
    parser.add_argument('--run-bootstrap', default=None, action='store_true',
                        help='Automatically run ZAP bootstrap. By default the bootstrap is not triggered')
    return parser.parse_args()


def main():
    args = runArgumentsParser()
    checkPythonVersion()

    if args.run_bootstrap:
        subprocess.check_call(os.path.join(CHIP_ROOT_DIR, "scripts/tools/zap/zap_bootstrap.sh"), shell=True)

    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets()
    for target in targets:
        subprocess.check_call(['./scripts/tools/zap/convert.py'] + target)


if __name__ == '__main__':
    main()
