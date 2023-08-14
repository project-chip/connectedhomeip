#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
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

import os
import sys
import unittest
from dataclasses import dataclass, field
from typing import List

import yaml

try:
    from matter_idl.matter_idl_parser import CreateParser
except ImportError:

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.matter_idl_parser import CreateParser

from matter_idl.generators import GeneratorStorage
from matter_idl.generators.cpp.application import CppApplicationGenerator
from matter_idl.generators.cpp.tlvmeta import TLVMetaDataGenerator
from matter_idl.generators.java import JavaClassGenerator, JavaJNIGenerator
from matter_idl.matter_idl_types import Idl

TESTS_DIR = os.path.join(os.path.dirname(__file__), "tests")
REGENERATE_GOLDEN_IMAGES = False


@dataclass
class ExpectedOutput:
    file_name: str
    golden_path: str


@dataclass
class GeneratorTestCase:
    input_idl: str
    outputs: List[ExpectedOutput] = field(default_factory=list)

    def add_outputs(self, yaml_outputs_dict):
        for file_name, golden_path in yaml_outputs_dict.items():
            self.outputs.append(ExpectedOutput(
                file_name=file_name, golden_path=golden_path))


class TestCaseStorage(GeneratorStorage):
    def __init__(self, test_case: GeneratorTestCase, checker: unittest.TestCase):
        super().__init__()
        self.test_case = test_case
        self.checker = checker
        self.checked_files = set()

    def get_existing_data_path(self, relative_path: str):
        for expected in self.test_case.outputs:
            if expected.file_name == relative_path:
                return os.path.join(TESTS_DIR, expected.golden_path)

        self.checker.fail("Expected output %s not found" % relative_path)
        return None

    def get_existing_data(self, relative_path: str):
        self.checked_files.add(relative_path)

        path = self.get_existing_data_path(relative_path)
        if path:
            with open(path, 'rt') as golden:
                return golden.read()

        # This will attempt a new write, causing a unit test failure
        self.checker.fail("Expected output %s not found" % relative_path)
        return None

    def write_new_data(self, relative_path: str, content: str):
        if REGENERATE_GOLDEN_IMAGES:
            print("RE-GENERATING %r" % relative_path)
            # Expect writing only on regeneration
            with open(self.get_existing_data_path(relative_path), 'wt') as golden:
                golden.write(content)
                return

        # This is a unit test failure: we do NOT expect
        # to write any new data

        # This will display actual diffs in the output files
        self.checker.assertEqual(
            self.get_existing_data(relative_path), content, "Content of %s" % relative_path)

        # Even if no diff, to be build system friendly, we do NOT expect any
        # actual data writes.
        raise AssertionError("Unexpected write to %s" % relative_path)


@dataclass
class GeneratorTest:
    generator_name: str
    test_cases: List[GeneratorTestCase] = field(default_factory=list)

    def add_test_cases(self, yaml_test_case_dict):
        for idl_path, outputs in yaml_test_case_dict.items():
            test_case = GeneratorTestCase(input_idl=idl_path)
            test_case.add_outputs(outputs)
            self.test_cases.append(test_case)

    def _create_generator(self, storage: GeneratorStorage, idl: Idl):
        if self.generator_name.lower() == 'java-jni':
            return JavaJNIGenerator(storage, idl)
        if self.generator_name.lower() == 'java-class':
            return JavaClassGenerator(storage, idl)
        if self.generator_name.lower() == 'cpp-app':
            return CppApplicationGenerator(storage, idl)
        if self.generator_name.lower() == 'cpp-tlvmeta':
            return TLVMetaDataGenerator(storage, idl, table_name="clusters_meta")
        if self.generator_name.lower() == 'custom-example-proto':
            sys.path.append(os.path.abspath(
                os.path.join(os.path.dirname(__file__), '../examples')))
            from matter_idl_plugin import CustomGenerator
            return CustomGenerator(storage, idl, package='com.matter.example.proto')
        else:
            raise Exception("Unknown generator for testing: %s",
                            self.generator_name.lower())

    def run_test_cases(self, checker: unittest.TestCase):
        for test in self.test_cases:
            with checker.subTest(idl=test.input_idl):
                storage = TestCaseStorage(test, checker)
                with open(os.path.join(TESTS_DIR, test.input_idl), 'rt') as stream:
                    idl = CreateParser().parse(stream.read())

                generator = self._create_generator(storage, idl)
                generator.render(dry_run=False)

                checker.assertEqual(storage.checked_files, set(
                    map(lambda x: x.file_name, test.outputs)))


def build_tests(yaml_data) -> List[GeneratorTest]:
    """
    Transforms the YAML dictonary (Dict[str, Dict[str, Dict[str,str]]]) into
    a generator test structure.
    """
    result = []

    for key, test_cases in yaml_data.items():
        generator = GeneratorTest(generator_name=key)
        generator.add_test_cases(test_cases)
        result.append(generator)

    return result


class TestGenerators(unittest.TestCase):
    def test_generators(self):
        self.maxDiff = None
        with open(os.path.join(TESTS_DIR, "available_tests.yaml"), 'rt') as stream:
            yaml_data = yaml.safe_load(stream)

        for test in build_tests(yaml_data):
            with self.subTest(generator=test.generator_name):
                test.run_test_cases(self)


if __name__ == '__main__':
    if 'IDL_GOLDEN_REGENERATE' in os.environ:
        # run with `IDL_GOLDEN_REGENERATE=1` to cause a regeneration of test
        # data. Then one can use `git diff` to see if the deltas make sense
        REGENERATE_GOLDEN_IMAGES = True
    unittest.main()
