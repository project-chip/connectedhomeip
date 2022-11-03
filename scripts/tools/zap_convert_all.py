#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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


def runBootstrap():
    subprocess.check_call(os.path.join(CHIP_ROOT_DIR, "scripts/tools/zap/zap_bootstrap.sh"), shell=True)


def main():
    args = runArgumentsParser()
    checkPythonVersion()
    if args.run_bootstrap:
        runBootstrap()
    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets()
    for target in targets:
        subprocess.check_call(['./scripts/tools/zap/convert.py'] + target)


if __name__ == '__main__':
    main()
