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

import argparse
import multiprocessing
import os
import subprocess
import sys
from pathlib import Path

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


zapFilesToSkip = {
    # examples/chef/sample_app_util/test_files/sample_zap_file.zap is
    # not a real .zap file; it's input to generating .zap files.  So
    # the path to zcl.json in it is just wrong, and we should skip it.
    "examples/chef/sample_app_util/test_files/sample_zap_file.zap",
}


def getTargets():
    ROOTS_TO_SEARCH = [
        './examples',
        './src/darwin',
        './src/controller/data_model',
        './scripts/tools/zap/tests/inputs',
    ]

    targets = []
    for root in ROOTS_TO_SEARCH:
        for filepath in Path(root).rglob('*.zap'):
            path = str(filepath)
            if path in zapFilesToSkip:
                continue
            targets.append(path)

    return targets


def runArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Convert all .zap files to the current zap version')
    parser.add_argument('--run-bootstrap', default=None, action='store_true',
                        help='Automatically run ZAP bootstrap. By default the bootstrap is not triggered')
    parser.add_argument('--parallel', action='store_true')
    parser.add_argument('--no-parallel', action='store_false', dest='parallel')
    parser.add_argument('--dry-run', action='store_true', dest='dry_run')
    parser.set_defaults(parallel=True)
    parser.set_defaults(dry_run=False)

    return parser.parse_args()


def convertOne(target):
    """
    Helper method that may be run in parallel to convert a single target.
    """
    subprocess.check_call(['./scripts/tools/zap/convert.py'] + [target])


def main():
    args = runArgumentsParser()
    checkPythonVersion()

    if args.run_bootstrap:
        subprocess.check_call(os.path.join(CHIP_ROOT_DIR, "scripts/tools/zap/zap_bootstrap.sh"), shell=True)

    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets()

    if args.dry_run:
        for target in targets:
            print(f"Should convert {target}")
        sys.exit(0)

    if args.parallel:
        # Ensure each zap run is independent
        os.environ['ZAP_TEMPSTATE'] = '1'
        with multiprocessing.Pool() as pool:
            for _ in pool.imap_unordered(convertOne, targets):
                pass
    else:
        for target in targets:
            convertOne(target)


if __name__ == '__main__':
    main()
