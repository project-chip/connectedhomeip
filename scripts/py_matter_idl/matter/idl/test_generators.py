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
from importlib import import_module
from pathlib import Path
from typing import List

import yaml

try:
    from matter.idl.matter_idl_parser import CreateParser
except ImportError:
    sys.path.append(str(Path(__file__).resolve().parent / ".." / ".."))
    from matter.idl.matter_idl_parser import CreateParser

from matter.idl.generators.cpp.application import CppApplicationGenerator
from matter.idl.generators.cpp.tlvmeta import TLVMetaDataGenerator
from matter.idl.generators.java import JavaClassGenerator, JavaJNIGenerator
from matter.idl.generators.storage import GeneratorStorage
from matter.idl.matter_idl_types import (
    Cluster,
    Command,
    DataType,
    Field,
    FieldQuality,
    Idl,
    Struct,
)

TESTS_DIR = os.path.join(os.path.dirname(__file__), "tests")
EXAMPLES_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '../../examples'))
ALL_CLUSTERS_APP_MATTER = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '../../../..',
                 'examples/all-clusters-app/all-clusters-common/all-clusters-app.matter'))
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
            if REGENERATE_GOLDEN_IMAGES and not os.path.exists(path):
                return ""

            with open(path) as golden:
                return golden.read()

        # This will attempt a new write, causing a unit test failure
        self.checker.fail("Expected output %s not found" % relative_path)
        return None

    def write_new_data(self, relative_path: str, content: str):
        if REGENERATE_GOLDEN_IMAGES:
            print("RE-GENERATING %r" % relative_path)
            # Expect writing only on regeneration
            path = os.path.abspath(self.get_existing_data_path(relative_path))
            dir_path = os.path.dirname(path)
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)
            with open(path, 'wt') as golden:
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
                os.path.join(os.path.dirname(__file__), '../../examples')))
            from matter_idl_plugin import CustomGenerator
            return CustomGenerator(storage, idl, package='com.matter.example.proto')
        raise Exception("Unknown generator for testing: %s",
                        self.generator_name.lower())

    def run_test_cases(self, checker: unittest.TestCase):
        for test in self.test_cases:
            with checker.subTest(idl=test.input_idl):
                storage = TestCaseStorage(test, checker)
                with open(os.path.join(TESTS_DIR, test.input_idl)) as stream:
                    idl = CreateParser().parse(stream.read(), file_name=test.input_idl)

                generator = self._create_generator(storage, idl)
                generator.render(dry_run=False)

                checker.assertEqual(storage.checked_files, {x.file_name for x in test.outputs})


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
        with open(os.path.join(TESTS_DIR, "available_tests.yaml")) as stream:
            yaml_data = yaml.safe_load(stream)

        for test in build_tests(yaml_data):
            with self.subTest(generator=test.generator_name):
                test.run_test_cases(self)


class TestMatterIdlPluginHelpers(unittest.TestCase):
    def _import_plugin(self):
        if EXAMPLES_DIR not in sys.path:
            sys.path.append(EXAMPLES_DIR)
        return import_module("matter_idl_plugin")

    def test_to_protobuf_type_aliases_and_named_types(self):
        plugin = self._import_plugin()
        toProtobufType = plugin.toProtobufType

        # Primitive and legacy aliases should map to protobuf primitives.
        self.assertEqual(toProtobufType("boolean"), "bool")
        self.assertEqual(toProtobufType("single"), "float")
        self.assertEqual(toProtobufType("double"), "double")
        self.assertEqual(toProtobufType("char_string"), "string")
        self.assertEqual(toProtobufType("long_octet_string"), "bytes")
        self.assertEqual(toProtobufType("uint32"), "uint32")
        self.assertEqual(toProtobufType("int64"), "int64")

        # Derived/sized integer aliases should normalize into protobuf ints.
        self.assertEqual(toProtobufType("int16u"), "uint32")
        self.assertEqual(toProtobufType("int40u"), "uint64")
        self.assertEqual(toProtobufType("int16s"), "int32")
        self.assertEqual(toProtobufType("int40s"), "int64")
        self.assertEqual(toProtobufType("temperature"), "int32")

        # Cluster-local enum/bitmap names should stay as named types.
        self.assertEqual(toProtobufType("MyEnum"), "MyEnum")
        self.assertEqual(toProtobufType("LevelControlOptions"), "LevelControlOptions")
        self.assertEqual(toProtobufType("CustomStruct"), "CustomStruct")

    def test_to_enum_entry_name_uses_enum_acronym_prefix(self):
        plugin = self._import_plugin()
        toEnumEntryName = plugin.toEnumEntryName

        self.assertEqual(toEnumEntryName("kKnown", "MyEnum"), "ME_KNOWN")
        self.assertEqual(toEnumEntryName("Known", "MyEnum"), "ME_KNOWN")
        self.assertEqual(
            toEnumEntryName("kCoupleColorTempToLevel", "LevelControlOptions"),
            "LCO_COUPLE_COLOR_TEMP_TO_LEVEL")

    def test_encoding_type_and_field_helpers(self):
        plugin = self._import_plugin()
        EncodingDataType = plugin.EncodingDataType

        self.assertEqual(plugin.EncodingDataType.fromType("uint32"), EncodingDataType.UINT)
        self.assertEqual(plugin.EncodingDataType.fromType("uint64"), EncodingDataType.UINT)
        self.assertEqual(plugin.EncodingDataType.fromType("int32"), EncodingDataType.INT)
        self.assertEqual(plugin.EncodingDataType.fromType("int64"), EncodingDataType.INT)
        self.assertEqual(plugin.EncodingDataType.fromType("bool"), EncodingDataType.BOOL)
        self.assertEqual(plugin.EncodingDataType.fromType("string"), EncodingDataType.CHAR_STRING)
        self.assertEqual(plugin.EncodingDataType.fromType("bytes"), EncodingDataType.OCT_STRING)
        self.assertEqual(plugin.EncodingDataType.fromType("float"), EncodingDataType.FLOAT)
        self.assertEqual(plugin.EncodingDataType.fromType("double"), EncodingDataType.DOUBLE)
        self.assertEqual(plugin.EncodingDataType.fromType("MyEnum"), EncodingDataType.STRUCT)

        simple_field = Field(data_type=DataType(name="int16u"), code=1, name="someInteger")
        list_field = Field(data_type=DataType(name="int16u"), code=2, name="someList", is_list=True)
        optional_field = Field(
            data_type=DataType(name="int16s"),
            code=3,
            name="optionalCount",
            qualities=FieldQuality.OPTIONAL,
        )
        named_field = Field(data_type=DataType(name="LevelControlOptions"), code=20, name="options")

        self.assertEqual(plugin.toProtobufFullType(simple_field), "uint32")
        self.assertEqual(plugin.toProtobufFullType(list_field), "repeated uint32")
        self.assertEqual(plugin.toProtobufFullType(optional_field), "optional int32")
        self.assertEqual(plugin.toFieldTag(simple_field), 524289)
        self.assertEqual(plugin.toFieldTag(named_field), 3145748)
        self.assertEqual(
            plugin.toFieldComment(simple_field),
            "/** int16u Type: 1 IsList: 0 FieldId: 1 */",
        )
        self.assertEqual(
            plugin.toFieldComment(named_field),
            "/** LevelControlOptions Type: 6 IsList: 0 FieldId: 20 */",
        )

    def test_command_helpers(self):
        plugin = self._import_plugin()

        request_fields = [Field(data_type=DataType(name="int16u"), code=1, name="request")]
        response_fields = [Field(data_type=DataType(name="int16u"), code=2, name="response")]
        cluster = Cluster(
            name="TestCluster",
            code=1,
            structs=[
                Struct(name="RequestType", fields=request_fields),
                Struct(name="ResponseType", fields=response_fields),
            ],
        )

        command = Command(name="DoThing", code=1, input_param="RequestType", output_param="ResponseType")
        missing = Command(name="Missing", code=2, input_param=None, output_param="UnknownType")

        self.assertEqual(plugin.commandArgs(command, cluster), request_fields)
        self.assertEqual(plugin.commandResponseArgs(command, cluster), response_fields)
        self.assertEqual(plugin.commandArgs(missing, cluster), [])
        self.assertEqual(plugin.commandResponseArgs(missing, cluster), [])

    def test_custom_generator_requires_package_option(self):
        plugin = self._import_plugin()

        with self.assertRaisesRegex(Exception, "Please provide a \"--option package:<name>\" argument"):
            plugin.CustomGenerator(GeneratorStorage(), Idl())

    def test_custom_generator_real_world_all_clusters_sample(self):
        plugin = self._import_plugin()

        class MemoryStorage(GeneratorStorage):
            def __init__(self):
                super().__init__()
                self.outputs = {}

            def get_existing_data(self, relative_path: str):
                return self.outputs.get(relative_path, "")

            def write_new_data(self, relative_path: str, content: str):
                self.outputs[relative_path] = content

        with open(ALL_CLUSTERS_APP_MATTER) as stream:
            idl = CreateParser().parse(stream.read(), file_name=ALL_CLUSTERS_APP_MATTER)

        storage = MemoryStorage()
        plugin.CustomGenerator(storage, idl, package='com.matter.example.proto').render(dry_run=False)

        self.assertEqual(len(storage.outputs), len(idl.clusters))
        self.assertIn("proto/unit_testing_cluster.proto", storage.outputs)
        self.assertIn("proto/access_control_cluster.proto", storage.outputs)

        unit_testing_output = storage.outputs["proto/unit_testing_cluster.proto"]
        access_control_output = storage.outputs["proto/access_control_cluster.proto"]

        # Real-IDL regression checks: enum naming, integer normalization, and tag encoding.
        self.assertIn("SE_VALUE_A = 1;", unit_testing_output)
        self.assertIn("/** int16u Type: 1 IsList: 0 FieldId:", unit_testing_output)
        self.assertIn("/** AccessRestrictionTypeEnum Type: 6 IsList: 0 FieldId: 0 */", access_control_output)
        self.assertIn("AccessRestrictionTypeEnum type = 3145728;", access_control_output)
        self.assertIn("/** cluster_id Type: 1 IsList: 0 FieldId: 1 */", access_control_output)
        self.assertIn("uint32 cluster = 524289;", access_control_output)
        self.assertIn("repeated AccessRestrictionStruct restrictions = 3145730;", access_control_output)


if __name__ == '__main__':
    if 'IDL_GOLDEN_REGENERATE' in os.environ:
        # run with `IDL_GOLDEN_REGENERATE=1` to cause a regeneration of test
        # data. Then one can use `git diff` to see if the deltas make sense
        REGENERATE_GOLDEN_IMAGES = True
    unittest.main()
