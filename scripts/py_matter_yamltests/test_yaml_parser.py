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

from matter.yamltests.definitions import ParseSource, SpecDefinitions
from matter.yamltests.errors import TestStepEnumError, TestStepEnumSpecifierNotUnknownError, TestStepEnumSpecifierWrongError
from matter.yamltests.parser import TestParser, TestParserConfig, build_revision_var_name

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
      <attribute side="server" code="0xFFFD" type="int16u" writable="false" optional="false">ClusterRevision</attribute>

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

no_revision_check_yaml = '''
name: Test without any min/maxRevision applied
config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1
tests:
    - label: "Step 1 - No revision check"
      command: "test"
    - label: "Step 2 - No revision check"
      command: "test"
    - label: "Step 3 - No revision check"
      command: "test"
'''

min_max_revision_injection_yaml = '''
name: Test Min/Max Revision Injection
config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1
tests:
    - label: "Step 1 - No revision check"
      command: "test"
    - label: "Step 2 - With revision check"
      command: "test"
      minRevision: 5
    - label: "Step 3 - Also with revision check"
      command: "test"
      maxRevision: 10
'''

min_max_revision_check_yaml = '''
name: Test Min/Max Revision Check
config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1
tests:
    - label: "Step 1 - The revision check"
      command: "test"
      minRevision: 5
      maxRevision: 10
'''

# A YAML to test ClusterRevision check injection on two different endpoints
min_max_revision_multi_injection_yaml = '''
name: Test Min/Max Revision Multi Injection
config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1
tests:
    - label: "Step 1 - EP 1"
      command: "test"
      minRevision: 5
    - label: "Step 2 - EP 2"
      endpoint: 2
      command: "test"
      maxRevision: 10
    - label: "Step 3 - EP 1 again"
      command: "test"
      maxRevision: 10
    - label: "Step 4 - EP 2 again"
      endpoint: 2
      command: "test"
      minRevision: 5
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
            self.assertEqual(
                value['value'], _BASIC_ARITHMETIC_ARG_RESULTS[idx])

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

    def test_revision_no_injection_when_not_needed(self):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(no_revision_check_yaml, parser_config)

        steps = list(yaml_parser.tests)
        # Original YAML has 3 tests. It should remain untouched.
        self.assertEqual(len(steps), 3)
        self.assertEqual(steps[0].label, "Step 1 - No revision check")
        self.assertEqual(steps[1].label, "Step 2 - No revision check")
        self.assertEqual(steps[2].label, "Step 3 - No revision check")

    def test_revision_step_injection(self):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(
            min_max_revision_injection_yaml, parser_config)

        # The parser's YAML had 3 steps. The injection logic should add one
        # `readAttribute` step before step 2 (which is index 1).
        # The parser filters disabled steps, but here we have none.
        # The injection happens in YamlTests.__init__
        steps = list(yaml_parser.tests)

        # Original YAML has 3 tests. We should have 1 read operation injection.
        self.assertEqual(len(steps), 4)

        # Step index 0 should be the original "Step 1 - No revision check"
        self.assertEqual(steps[0].label, "Step 1 - No revision check")

        # Step index 1 should be the *injected* step
        self.assertEqual(
            steps[1].label, "Read ClusterRevision for conditions (EP: 1, CL: Test)")
        self.assertEqual(steps[1].command, "readAttribute")
        self.assertEqual(steps[1].attribute, "ClusterRevision")
        self.assertEqual(
            steps[1].responses[0]['values'][0]['saveAs'], build_revision_var_name(endpoint=1, cluster="Test"))

        # Step index 2 should be the original "Step 2 - With revision check"
        self.assertEqual(steps[2].label, "Step 2 - With revision check")
        self.assertEqual(steps[2].min_revision, 5)

        # Step index 3 should be the original "Step 3 - Also with revision check"
        self.assertEqual(steps[3].label, "Step 3 - Also with revision check")
        self.assertEqual(steps[3].max_revision, 10)

    def test_revision_step_multi_injection(self):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(
            min_max_revision_multi_injection_yaml, parser_config)

        steps = list(yaml_parser.tests)

        # Original YAML has 4 tests. We should have 2 injections (one for EP1, one for EP2).
        self.assertEqual(len(steps), 6)

        # Step index 0: Injected for (EP: 1, CL: Test)
        self.assertEqual(
            steps[0].label, "Read ClusterRevision for conditions (EP: 1, CL: Test)")
        self.assertEqual(steps[0].endpoint, 1)
        self.assertEqual(
            steps[0].responses[0]['values'][0]['saveAs'], build_revision_var_name(endpoint=1, cluster="Test"))

        # Step index 1: Original "Step 1 - EP 1"
        self.assertEqual(steps[1].label, "Step 1 - EP 1")
        self.assertEqual(steps[1].endpoint, 1)

        # Step index 2: Injected for (EP: 2, CL: Test)
        self.assertEqual(
            steps[2].label, "Read ClusterRevision for conditions (EP: 2, CL: Test)")
        self.assertEqual(steps[2].endpoint, 2)
        self.assertEqual(
            steps[2].responses[0]['values'][0]['saveAs'], build_revision_var_name(endpoint=2, cluster="Test"))

        # Step index 3: Original "Step 2 - EP 2"
        self.assertEqual(steps[3].label, "Step 2 - EP 2")
        self.assertEqual(steps[3].endpoint, 2)

        # Step index 4: Original "Step 3 - EP 1 again" (no new injection)
        self.assertEqual(steps[4].label, "Step 3 - EP 1 again")
        self.assertEqual(steps[4].endpoint, 1)

        # Step index 5: Original "Step 4 - EP 2 again" (no new injection)
        self.assertEqual(steps[5].label, "Step 4 - EP 2 again")
        self.assertEqual(steps[5].endpoint, 2)

    def _get_revision_check_between_5_and_10_test_step(self, revision_value):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(min_max_revision_check_yaml, parser_config)

        # Manually set the runtime variable to simulate the read step.
        # The injected step (list(yaml_parser.tests)[0]) set this variable
        # during a read that the runner would do for real.
        # We are checking the step that *uses* it (list(yaml_parser.tests)[1]).
        cluster_rev_var_name = build_revision_var_name(
            endpoint=1, cluster="Test")
        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=revision_value)

        # Get the step that has the min/max check.
        # This is the 2nd step (index 1) after the injected step (index 0).
        test_step = list(yaml_parser.tests)[1]

        # The step from the YAML has min: 5, max: 10
        self.assertEqual(test_step.min_revision, 5)
        self.assertEqual(test_step.max_revision, 10)

        return test_step

    def test_revision_check_in_range(self):
        # 7 is between 5 and 10
        test_step = self._get_revision_check_between_5_and_10_test_step(
            revision_value=7)
        self.assertTrue(test_step.is_revision_condition_passed)

    def test_revision_check_in_range_min_boundary(self):
        # 5 is between 5 and 10 (inclusive)
        test_step = self._get_revision_check_between_5_and_10_test_step(
            revision_value=5)
        self.assertTrue(test_step.is_revision_condition_passed)

    def test_revision_check_in_range_max_boundary(self):
        # 10 is between 5 and 10 (inclusive)
        test_step = self._get_revision_check_between_5_and_10_test_step(
            revision_value=10)
        self.assertTrue(test_step.is_revision_condition_passed)

    def test_revision_check_out_of_range_min(self):
        # 4 is less than 5
        test_step = self._get_revision_check_between_5_and_10_test_step(
            revision_value=4)
        self.assertFalse(test_step.is_revision_condition_passed)

    def test_revision_check_out_of_range_max(self):
        # 11 is greater than 10
        test_step = self._get_revision_check_between_5_and_10_test_step(
            revision_value=11)
        self.assertFalse(test_step.is_revision_condition_passed)

    def test_revision_check_variable_not_set(self):
        # Variable is not set (simulated by passing None): this is illegal and raises
        test_step = self._get_revision_check_between_5_and_10_test_step(
            revision_value=None)
        with self.assertRaises(KeyError):
            test_step.is_revision_condition_passed

    def test_revision_check_only_min(self):
        # This test needs a slightly different YAML
        yaml_content = '''
name: Test Min/Max Revision Check
config: { cluster: "Test", endpoint: 1 }
tests:
    - label: "Step 1"
      command: "test"
      minRevision: 5
'''
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(yaml_content, parser_config)
        cluster_rev_var_name = build_revision_var_name(
            endpoint=1, cluster="Test")

        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=4)
        test_step_fail = list(yaml_parser.tests)[1]
        self.assertFalse(test_step_fail.is_revision_condition_passed)

        yaml_parser = TestParser(yaml_content, parser_config)
        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=5)
        test_step_pass = list(yaml_parser.tests)[1]
        self.assertTrue(test_step_pass.is_revision_condition_passed)

        yaml_parser = TestParser(yaml_content, parser_config)
        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=100)
        test_step_pass_high = list(yaml_parser.tests)[1]
        self.assertTrue(test_step_pass_high.is_revision_condition_passed)

    def test_revision_check_only_max(self):
        # This test needs a slightly different YAML
        yaml_content = '''
name: Test Min/Max Revision Check
config: { cluster: "Test", endpoint: 1 }
tests:
    - label: "Step 1"
      command: "test"
      maxRevision: 10
'''
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(yaml_content, parser_config)
        cluster_rev_var_name = build_revision_var_name(
            endpoint=1, cluster="Test")

        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=11)
        test_step_fail = list(yaml_parser.tests)[1]
        self.assertFalse(test_step_fail.is_revision_condition_passed)

        yaml_parser = TestParser(yaml_content, parser_config)
        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=10)
        test_step_pass = list(yaml_parser.tests)[1]
        self.assertTrue(test_step_pass.is_revision_condition_passed)

        yaml_parser = TestParser(yaml_content, parser_config)
        yaml_parser.tests.set_runtime_variable(
            name=cluster_rev_var_name, value=2)
        test_step_pass_low = list(yaml_parser.tests)[1]
        self.assertTrue(test_step_pass_low.is_revision_condition_passed)

    def test_revision_check_no_revision_at_all(self):
        parser_config = TestParserConfig(None, self._definitions)
        yaml_parser = TestParser(no_revision_check_yaml, parser_config)

        test_step_fail = list(yaml_parser.tests)[0]
        self.assertTrue(test_step_fail.is_revision_condition_passed)


def main():
    unittest.main()


if __name__ == '__main__':
    main()
