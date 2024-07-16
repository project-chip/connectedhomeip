#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os
import re
import sys
from pathlib import Path

from chiptest import AllChipToolYamlTests

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

# TODO: These tests need to be re-written. Please see https://github.com/project-chip/connectedhomeip/issues/32620
KNOWN_BAD_UNIT_TESTING = set(('Test_TC_S_2_2.yaml', 'Test_TC_S_2_3.yaml'))


def _is_cert_test(path):
    return "certification" in os.path.dirname(path)


def main():
    bad_tests = set()
    for test in AllChipToolYamlTests(use_short_run_name=False):
        with open(test.run_name, "r") as f:
            # Unit testing cluster is disallowed in cert tests, but permissible in general integration tests
            unit_test_lines = {}
            if _is_cert_test(test.run_name):
                unit_test_lines = {lineno: line.strip() for lineno, line in enumerate(
                    f) if re.search('cluster: "Unit Testing"', line)}
            if unit_test_lines:
                print(
                    f'Found certification test using Unit Testing cluster: {test.name}')
                for line, val in unit_test_lines.items():
                    print(f'\t{line+1}: {val}')
                bad_tests.add(Path(test.run_name).name)

    if bad_tests - KNOWN_BAD_UNIT_TESTING:
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
