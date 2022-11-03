#!/usr/bin/env python3

# SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import coloredlogs
import difflib
import logging
import os
import subprocess
import sys

from typing import List

SCRIPT_ROOT = os.path.dirname(__file__)


def build_expected_output(root: str, out: str) -> List[str]:
    with open(os.path.join(SCRIPT_ROOT, 'expected_test_cmakelists.txt'), 'rt') as f:
        for l in f.readlines():
            yield l.replace("{root}", root).replace("{out}", out)


def build_actual_output(root: str, out: str) -> List[str]:
    # Fake out that we have a project root
    binary = os.path.join(SCRIPT_ROOT, '../gn_to_cmakelists.py')
    project = os.path.join(SCRIPT_ROOT, "test_project.json")
    cmake = os.path.join(SCRIPT_ROOT, "../../../out/CMakeLists.txt")
    retval = subprocess.run([
        binary,
        project,
    ], stdout=subprocess.PIPE, check=True, encoding='UTF-8', )

    with open(cmake, 'rt') as f:
        for l in f.readlines():
            yield l


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
