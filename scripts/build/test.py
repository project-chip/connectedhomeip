#!/usr/bin/env python3

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

import click
import coloredlogs
import difflib
import logging
import os
import subprocess
import sys
import time

from builders.host import ConcretePlatformName

from typing import List

SCRIPT_ROOT = os.path.dirname(__file__)


def build_expected_output(root: str, out: str) -> List[str]:
    with open(os.path.join(SCRIPT_ROOT, 'expected_all_platform_commands.txt'), 'rt') as f:
        for l in f.readlines():
            yield l.replace("{root}", root).replace("{out}", out).replace('{real_platform}', ConcretePlatformName())


def build_actual_output(root: str, out: str) -> List[str]:
    # Fake out that we have a project root
    binary = os.path.join(SCRIPT_ROOT, 'build_examples.py')

    runenv = {}
    runenv.update(os.environ)
    runenv.update({
        'PW_PROJECT_ROOT': root,
        'ANDROID_NDK_HOME': 'TEST_ANDROID_NDK_HOME',
        'ANDROID_HOME': 'TEST_ANDROID_HOME',
    })

    retval = subprocess.run([
        binary,
        '--platform', 'all',
        '--log-level', 'FATAL',
        '--dry-run',
        '--repo', root,
        '--out-prefix', out,
        'build'
    ], stdout=subprocess.PIPE, check=True, encoding='UTF-8', env=runenv)

    return [l + '\n' for l in retval.stdout.split('\n')]


def main():
    coloredlogs.install(level=logging.INFO,
                        fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')

    ROOT = '/TEST/BUILD/ROOT'
    OUT = '/OUTPUT/DIR'

    expected = [l for l in build_expected_output(ROOT, OUT)]
    actual = [l for l in build_actual_output(ROOT, OUT)]

    diffs = [line for line in difflib.unified_diff(expected, actual)]

    if diffs:
        logging.error("DIFFERENCE between expected and generated output")
        for l in diffs:
            logging.warning("  " + l.strip())
        sys.exit(1)


if __name__ == '__main__':
    main()
