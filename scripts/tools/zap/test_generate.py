#!/usr/bin/env python3
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

import glob
import os
import shutil
import subprocess
import sys
import unittest
from dataclasses import dataclass, field
from typing import List, Optional

import yaml

TESTS_DIR = os.path.join(os.path.dirname(__file__), "tests")
CHIP_ROOT = os.path.abspath(os.path.join(
    os.path.dirname(__file__), "../../.."))


@dataclass
class ProgramArguments:
    stamp_file: Optional[str] = None
    regenerate_golden: bool = False
    output_directory: str = ''


PROGRAM_ARGUMENTS = None


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
    context: ProgramArguments
    test_cases: List[GeneratorTestCase] = field(default_factory=list)

    def add_test_cases(self, yaml_test_case_dict):
        for json, outputs in yaml_test_case_dict.items():
            test_case = GeneratorTestCase(template=json)
            test_case.add_outputs(outputs)
            self.test_cases.append(test_case)

    def run_test_cases(self, checker: unittest.TestCase):
        for test in self.test_cases:
            with checker.subTest(template=test.template):
                output_directory = os.path.join(
                    self.context.output_directory,
                    os.path.splitext(os.path.basename(self.zap))[0],
                    os.path.splitext(os.path.basename(test.template))[0]
                )

                # ensure a clean start as ALL outputs will be compared
                if os.path.exists(output_directory):
                    shutil.rmtree(output_directory)
                os.makedirs(output_directory, exist_ok=True)

                subprocess.check_call([
                    f"{CHIP_ROOT}/scripts/tools/zap/generate.py",
                    "--parallel",
                    "--output-dir",
                    output_directory,
                    "--templates",
                    os.path.join(TESTS_DIR, test.template),
                    os.path.join(TESTS_DIR, self.zap),
                ], cwd=output_directory)

                # Files generated, ready to check:
                #   - every output file MUST exist in the golden image list
                #   - every golden image file MUST exist in the output
                #   - content must match

                # test.outputs contain:
                #    - file_name
                #    - golden_path
                expected_files = set([o.file_name for o in test.outputs])
                actual_files = set([
                    name[len(output_directory)+1:] for name in glob.glob(f"{output_directory}/**/*", recursive=True)
                ])

                checker.assertEqual(
                    expected_files, actual_files, msg="Expected and actual generated file list MUST be identical.")

                # All files exist, ready to do the compare
                for entry in test.outputs:
                    expected = os.path.join(TESTS_DIR, entry.golden_path)
                    actual = os.path.join(output_directory, entry.file_name)

                    try:
                        subprocess.check_call(["diff", actual, expected])
                    except subprocess.CalledProcessError:
                        if self.context.regenerate_golden:
                            print(
                                f"Copying updated golden image from {actual} to {expected}")
                            subprocess.check_call(["cp", actual, expected])
                        else:
                            print("*"*80)
                            print("* Golden image regeneration seems to have failed.")
                            print("* Documentation regarding code-generation logic available at docs/code_generation.md")
                            print("*\n* Specifically to update golden images, you may want to run:")
                            print("*\n* ./scripts/tools/zap/test_generate.py --output out/gen --regenerate")
                            print("*\n" + "*"*80)
                            raise


def build_tests(yaml_data, context: ProgramArguments) -> List[GeneratorTest]:
    """
    Transforms the YAML dictonary (Dict[str, Dict[str, Dict[str,str]]]) into
    a generator test structure.
    """
    result = []

    for input_zap, test_cases in yaml_data.items():
        generator = GeneratorTest(zap=input_zap, context=context)
        generator.add_test_cases(test_cases)
        result.append(generator)

    return result


class TestGenerators(unittest.TestCase):
    def test_generators(self):
        with open(os.path.join(TESTS_DIR, "available_tests.yaml"), 'rt') as stream:
            yaml_data = yaml.safe_load(stream)

        global PROGRAM_ARGUMENTS
        for test in build_tests(yaml_data, context=PROGRAM_ARGUMENTS):
            with self.subTest(zap=test.zap):
                test.run_test_cases(self)


def process_arguments():
    """Parses sys.argv and extracts arguments that are specific to the script."""
    args = sys.argv[:]

    program_args = ProgramArguments()

    if '--regenerate' in args:
        idx = args.index('--regenerate')
        program_args.regenerate_golden = True
        del args[idx]
    elif 'ZAP_GENERATE_GOLDEN_REGENERATE' in os.environ:
        # Allow `ZAP_GENERATE_GOLDEN_REGENERATE=1 ninja check` to also
        # update golden images
        program_args.regenerate_golden = True

    if '--stamp' in args:
        idx = args.index('--stamp')
        program_args.stamp_file = args[idx + 1]
        del args[idx+1]
        del args[idx]

    if '--output' in args:
        idx = args.index('--output')
        program_args.output_directory = args[idx + 1]
        del args[idx+1]
        del args[idx]
    else:
        raise Exception("`--output` argument is required")

    return program_args, args


if __name__ == '__main__':
    process_args, unittest_args = process_arguments()

    if process_args.stamp_file:
        if os.path.exists(process_args.stamp_file):
            os.remove(process_args.stamp_file)

    PROGRAM_ARGUMENTS = process_args

    test_results = unittest.main(argv=unittest_args, exit=False)
    if test_results.result.failures:
        sys.exit(1)

    if process_args.stamp_file:
        open(process_args.stamp_file, "wb").close()
