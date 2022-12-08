#!/usr/bin/env python
# Copyright (c) 2022 Project CHIP Authors
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

import logging
import os
import subprocess
import sys
import unittest
import yaml

from typing import List, Optional
from dataclasses import dataclass, field

TESTS_DIR = os.path.join(os.path.dirname(__file__), "tests")
CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../.."))
REGENERATE_GOLDEN_IMAGES = False


@dataclass
class ExpectedOutput:
    file_name: str
    golden_path: str


@dataclass
class GeneratorTestCase:
    template: str
    outputs: List[ExpectedOutput] = field(default_factory=list)

    def add_outputs(self, yaml_outputs_dict):
        for file_name, golden_path in yaml_outputs_dict.items():
            self.outputs.append(ExpectedOutput(
                file_name=file_name, golden_path=golden_path))


@dataclass
class GeneratorTest:
    zap: str
    test_cases: List[GeneratorTestCase] = field(default_factory=list)

    def add_test_cases(self, yaml_test_case_dict):
        for json, outputs in yaml_test_case_dict.items():
            test_case = GeneratorTestCase(template=json)
            test_case.add_outputs(outputs)
            self.test_cases.append(test_case)


    def run_test_cases(self, checker: unittest.TestCase):
        for test in self.test_cases:
            with checker.subTest(template=test.template):
                subprocess.check_call(
                    [
                    f"{CHIP_ROOT}/scripts/tools/zap/generate.py",
                    "--parallel",
                    "--output-dir",
                    ".",
                    "--templates",
                    os.path.join(TESTS_DIR, test.template),
                    os.path.join(TESTS_DIR, self.zap),
                    ]
                )

                checker.assertEqual(0, 1)


def build_tests(yaml_data) -> List[GeneratorTest]:
    """
    Transforms the YAML dictonary (Dict[str, Dict[str, Dict[str,str]]]) into
    a generator test structure.
    """
    result = []

    for input_zap, test_cases in yaml_data.items():
        generator = GeneratorTest(zap=input_zap)
        generator.add_test_cases(test_cases)
        result.append(generator)

    return result


class TestGenerators(unittest.TestCase):
    def test_generators(self):
        with open(os.path.join(TESTS_DIR, "available_tests.yaml"), 'rt') as stream:
             yaml_data = yaml.safe_load(stream)

        for test in build_tests(yaml_data):
             with self.subTest(zap=test.zap):
                 test.run_test_cases(self)


@dataclass
class ProgramArguments:
    stamp_file: Optional[str] = None
    regenerate_golden: bool = False


def process_arguments():
    """Parses sys.argv and extracts arguments that are specifict to the script."""
    args = sys.argv[:]

    program_args = ProgramArguments()

    if '--regenerate' in args:
        idx = args.index('--regenerate')
        program_args.regenerate_golden = True
        del args[idx]

    if '--stamp' in args:
        idx = args.index('--stamp')
        program_args.stamp_file = args[idx + 1]
        del args[idx+1]
        del args[idx]

    return program_args, args


if __name__ == '__main__':
    process_args, unittest_args = process_arguments()

    if process_args.regenerate_golden:
        REGENERATE_GOLDEN_IMAGES = True

    if process_args.stamp_file:
        if os.path.exists(process_args.stamp_file):
            os.remove(process_args.stamp_file)

    unittest.main(argv=unittest_args)

    if process_args.stamp_file:
        open(process_args.stamp_file, "wb").close()
