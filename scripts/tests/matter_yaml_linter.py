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
import subprocess
import sys
from pathlib import Path

from chiptest import AllChipToolYamlTests

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))


def _is_cert_test(path):
    return "certification" in os.path.dirname(path)


def check_unit_testing():
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

    if bad_tests:
        return 1
    return 0


def check_manual_steps():
    # Doing this on a test-by-test basis so the log message is more obvious
    bad_test = False
    # We are operating in a VM, and although there is a checkout, it is working in a scratch directory
    # where the ownership is different than the runner.
    # Adding an exception for this directory so that git can function properly.
    subprocess.run("git config --global --add safe.directory '*'", shell=True)
    for test in AllChipToolYamlTests(use_short_run_name=False):

        cmd = f'git diff HEAD^..HEAD --unified=0 -- {test.run_name}'
        output = subprocess.check_output(cmd, shell=True).decode().splitlines()
        user_prompt_added = [line for line in output if re.search(r'^\+.*UserPrompt.*', line)]
        user_prompt_removed = [line for line in output if re.search(r'^\-.*UserPrompt.*', line)]
        if len(user_prompt_added) > len(user_prompt_removed):
            print(f'Found YAML test with additional manual steps: {test.name}')
            bad_test = True
    if bad_test:
        return 1
    return 0


def main():
    ret = check_unit_testing()
    ret += check_manual_steps()
    return ret


if __name__ == '__main__':
    sys.exit(main())
