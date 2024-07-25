#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
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

# TODO Once yamltest is a proper self contained module we can move this file
# to a more appropriate spot. For now, having this file to do some quick checks
# is arguably better then no checks at all.

import io
import unittest
from unittest.mock import mock_open, patch

from matter_yamltests.definitions import ParseSource, SpecDefinitions
from matter_yamltests.errors import TestStepEnumError, TestStepEnumSpecifierNotUnknownError, TestStepEnumSpecifierWrongError
from matter_yamltests.parser import TestParser, TestParserConfig

simple_test_description = '''<?xml version="1.0"?>
  <configurator>
    <enum name="TestEnum" type="enum8">
        <cluster code="0x1234"/>
        <item name="A" value="0x0"/>
        <item name="B" value="0x1"/>
        <item name="C" value="0x2"/>
    </enum>

    <struct name="TestStruct">
        <cluster code="0x1234"/>
        <item name="a" type="boolean"/>
    </struct>

    <cluster>
      <name>Test</name>
      <code>0x1234</code>

      <attribute side="server" code="0x0024" type="TestEnum" writable="true" optional="false">test_enum</attribute>

      <command source="client" code="1" name="test"></command>
      <command source="client" code="2" name="IntTest">
        <arg name="arg" type="int8u"/>
      </command>
    </cluster>
  </configurator>
'''

simple_test_yaml = '''
name: Test Cluster Tests

config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1

tests:
    - label: "Send Test Command"
      command: "test"
'''

enum_values_yaml = '''
name: Test Enum Values

config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1

tests:
    - label: "Read attribute test_enum Value"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: 0

    - label: "Read attribute test_enum Value"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: TestEnum.A

    - label: "Read attribute test_enum Value"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: TestEnum.A(0)

    - label: "Read attribute test_enum Value"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: TestEnum.UnknownEnumValue

    - label: "Read attribute test_enum Value"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: TestEnum.UnknownEnumValue(255)

    - label: "Write attribute test_enum Value"
      command: "writeAttribute"
      attribute: "test_enum"
      arguments:
          value: 0

    - label: "Write attribute test_enum Value"
      command: "writeAttribute"
      attribute: "test_enum"
      arguments:
          value: TestEnum.A

    - label: "Write attribute test_enum Value"
      command: "writeAttribute"
      attribute: "test_enum"
      arguments:
          value: TestEnum.A(0)

    - label: "Write attribute test_enum Value"
      command: "writeAttribute"
      attribute: "test_enum"
      arguments:
          value: TestEnum.UnknownEnumValue

    - label: "Write attribute test_enum Value"
      command: "writeAttribute"
      attribute: "test_enum"
      arguments:
          value: TestEnum.UnknownEnumValue(255)
'''

enum_value_read_response_wrong_code_yaml = '''
tests:
    - label: "Read attribute test_enum Value"
      cluster: "Test"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: 123
'''

enum_value_read_response_wrong_name_yaml = '''
tests:
    - label: "Read attribute test_enum Value"
      cluster: "Test"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: ThisIsWrong
'''

enum_value_read_response_wrong_code_specified_yaml = '''
tests:
    - label: "Read attribute test_enum Value"
      cluster: "Test"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: TestEnum.A(123)
'''

enum_value_read_response_not_unknown_code_specified_yaml = '''
tests:
    - label: "Read attribute test_enum Value"
      cluster: "Test"
      command: "readAttribute"
      attribute: "test_enum"
      response:
          value: TestEnum.UnknownEnumValue(0)
'''

_BASIC_ARITHMETIC_ARG_RESULTS = [6, 6, 2, 2, 8, 8, 2, 2, 1]
basic_arithmetic_yaml = '''
name: Test Cluster Tests

config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1
    myVariable: 4

tests:
    - label: "Add 2"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable + 2

    - label: "Add 2 with no space"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable+2

    - label: "Minus 2"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable - 2

    - label: "Minus 2 with no space"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable-2

    - label: "Multiply by 2"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable * 2

    - label: "Multiply by 2 with no space"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable*2

    - label: "Divide by 2"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable / 2

    - label: "Divide by 2 with no space"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: myVariable/2

    - label: "Arithmetic with parentheses"
      command: "IntTest"
      arguments:
          values:
              - name: "arg"
                value: (myVariable +3)/7
'''


def mock_open_with_parameter_content(content):
    file_object = mock_open(read_data=content).return_value
    file_object.__iter__.return_value = content.splitlines(True)
    return file_object


@patch('builtins.open', new=mock_open_with_parameter_content)
class TestYamlParser(unittest.TestCase):
    def setUp(self):
        self._definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(simple_test_description), name='simple_test_description')])

    def test_able_to_iterate_over_all_parsed_tests(self):
        # self._yaml_parser.tests implements `__next__`, which does value substitution. We are
        # simply ensure there is no exceptions raise.
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(simple_test_yaml, parser_config)
        count = 0
        for idx, test_step in enumerate(yaml_parser.tests):
            count += 1
            pass
        self.assertEqual(count, 1)

    def test_config(self):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(simple_test_yaml, parser_config)
        for idx, test_step in enumerate(yaml_parser.tests):
            self.assertEqual(test_step.node_id, 0x12344321)
            self.assertEqual(test_step.cluster, 'Test')
            self.assertEqual(test_step.endpoint, 1)

    def test_basic_arithmetic(self):
        parser_config = TestParserConfig(None, self._definitions)

        yaml_parser = TestParser(basic_arithmetic_yaml, parser_config)
        for idx, test_step in enumerate(yaml_parser.tests):
            values = test_step.arguments['values']
            self.assertEqual(len(values), 1)
            value = values[0]
            self.assertEqual(value['name'], 'arg')
            self.assertEqual(value['value'], _BASIC_ARITHMETIC_ARG_RESULTS[idx])

    def test_config_override(self):
        config_override = {'nodeId': 12345,
                           'cluster': 'TestOverride', 'endpoint': 4}
        parser_config = TestParserConfig(
            None, self._definitions, config_override)
        yaml_parser = TestParser(simple_test_yaml, parser_config)
        for idx, test_step in enumerate(yaml_parser.tests):
            self.assertEqual(test_step.node_id, 12345)
            self.assertEqual(test_step.cluster, 'TestOverride')
            self.assertEqual(test_step.endpoint, 4)

    def test_config_override_unknown_field(self):
        config_override = {'unknown_field': 1}
        parser_config = TestParserConfig(
            None, self._definitions, config_override)

        yaml_parser = TestParser(simple_test_yaml, parser_config)
        self.assertIsInstance(yaml_parser, TestParser)

    def test_config_valid_enum_values(self):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(enum_values_yaml, parser_config)
        self.assertIsInstance(yaml_parser, TestParser)

        for idx, test_step in enumerate(yaml_parser.tests):
            pass

    def test_config_read_response_wrong_code(self):
        parser_config = TestParserConfig(None, self._definitions)
        self.assertRaises(TestStepEnumError, TestParser,
                          enum_value_read_response_wrong_code_yaml, parser_config)

    def test_config_read_response_wrong_name(self):
        parser_config = TestParserConfig(None, self._definitions)
        self.assertRaises(TestStepEnumError, TestParser,
                          enum_value_read_response_wrong_name_yaml, parser_config)

    def test_config_read_response_wrong_code_specified(self):
        parser_config = TestParserConfig(None, self._definitions)
        self.assertRaises(TestStepEnumSpecifierWrongError, TestParser,
                          enum_value_read_response_wrong_code_specified_yaml, parser_config)

    def test_config_read_response_not_unknown_code_specified(self):
        parser_config = TestParserConfig(None, self._definitions)
        self.assertRaises(TestStepEnumSpecifierNotUnknownError, TestParser,
                          enum_value_read_response_not_unknown_code_specified_yaml, parser_config)


def main():
    unittest.main()


if __name__ == '__main__':
    main()
