#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import unittest
from unittest.mock import mock_open, patch

from matter_yamltests.hooks import TestParserHooks
from matter_yamltests.parser import TestParser
from matter_yamltests.parser_builder import TestParserBuilder, TestParserBuilderConfig

simple_yaml = '''
name: Hello World
'''

valid_yaml = '''
name: TestOnOff

tests:
    - label: "Toggle the light"
      cluster: "OnOff"
      command: "Toggle"

    - label: "Toggle the light"
      cluster: "OnOff"
      command: "Toggle"
'''

invalid_yaml = '''
name: TestOnOff

tests:
    - label: "Toggle the light"
      cluster: "OnOff"
      command: "Toggle"

    - label: "Toggle the light"
      cluster: "OnOff"
      command: "Toggle"

    - label: "Toggle the light"
      cluster: "OnOff"
      command: "Toggle"

    - label: "Toggle the light"
      cluster_wrong_key: "OnOff"
      command: "Toggle"

    - label: "Toggle the light"
      cluster: "OnOff"
      command: "Toggle"
'''


class TestHooks(TestParserHooks):
    def __init__(self):
        self.start_count = 0
        self.stop_count = 0
        self.test_start_count = 0
        self.test_failure_count = 0
        self.test_success_count = 0

    def parsing_start(self, count):
        self.start_count += 1

    def parsing_stop(self, duration):
        self.stop_count += 1

    def test_parsing_start(self, name):
        self.test_start_count += 1

    def test_parsing_success(self, duration):
        self.test_success_count += 1

    def test_parsing_failure(self, exception, duration):
        self.test_failure_count += 1


def mock_open_with_parameter_content(content):
    file_object = mock_open(read_data=content).return_value
    file_object.__iter__.return_value = content.splitlines(True)
    return file_object


@patch('builtins.open', new=mock_open_with_parameter_content)
class TestSuiteParserBuilder(unittest.TestCase):
    def test_parser_builder_config_defaults(self):
        parser_builder_config = TestParserBuilderConfig()
        self.assertIsInstance(parser_builder_config, TestParserBuilderConfig)
        self.assertIsNotNone(parser_builder_config.tests)
        self.assertIsNotNone(parser_builder_config.parser_config)
        self.assertIsNotNone(parser_builder_config.options)
        self.assertIsNotNone(parser_builder_config.hooks)

    def test_parser_builder_config_with_tests(self):
        tests = [simple_yaml, simple_yaml]
        parser_builder_config = TestParserBuilderConfig(tests)
        self.assertIsInstance(parser_builder_config, TestParserBuilderConfig)
        self.assertIsNotNone(parser_builder_config.tests)
        self.assertEqual(len(tests), len(parser_builder_config.tests))
        self.assertIsNotNone(parser_builder_config.parser_config)
        self.assertIsNotNone(parser_builder_config.options)

    def test_parser_builder_default(self):
        parser_builder = TestParserBuilder()
        self.assertIsInstance(parser_builder, TestParserBuilder)
        self.assertRaises(StopIteration, next, parser_builder)

    def test_parser_builder_with_empty_config(self):
        parser_builder_config = TestParserBuilderConfig()
        parser_builder = TestParserBuilder(parser_builder_config)
        self.assertIsInstance(parser_builder, TestParserBuilder)
        self.assertRaises(StopIteration, next, parser_builder)

    def test_parser_builder_with_a_single_test(self):
        tests = [valid_yaml]

        parser_builder_config = TestParserBuilderConfig(tests)
        parser_builder = TestParserBuilder(parser_builder_config)
        self.assertIsInstance(parser_builder, TestParserBuilder)
        self.assertIsInstance(next(parser_builder), TestParser)
        self.assertRaises(StopIteration, next, parser_builder)

    def test_parser_builder_with_a_multiple_tests(self):
        tests = [valid_yaml] * 5

        parser_builder_config = TestParserBuilderConfig(tests)
        parser_builder = TestParserBuilder(parser_builder_config)
        self.assertIsInstance(parser_builder, TestParserBuilder)

        for i in range(0, 5):
            self.assertIsInstance(next(parser_builder), TestParser)

        self.assertRaises(StopIteration, next, parser_builder)

    def test_parser_builder_config_hooks_single_test_with_multiple_steps(self):
        tests = [valid_yaml]
        hooks = TestHooks()
        parser_builder_config = TestParserBuilderConfig(tests, hooks=hooks)

        parser_builder = TestParserBuilder(parser_builder_config)
        self.assertIsInstance(parser_builder, TestParserBuilder)

        for parser in parser_builder:
            pass

        self.assertRaises(StopIteration, next, parser_builder)

        self.assertEqual(hooks.start_count, 1)
        self.assertEqual(hooks.stop_count, 1)
        self.assertEqual(hooks.test_start_count, 1)
        self.assertEqual(hooks.test_success_count, 1)
        self.assertEqual(hooks.test_failure_count, 0)

    def test_parser_builder_config_hooks_multiple_test_with_multiple_steps(self):
        tests = [valid_yaml] * 5
        hooks = TestHooks()
        parser_builder_config = TestParserBuilderConfig(tests, hooks=hooks)

        parser_builder = TestParserBuilder(parser_builder_config)
        self.assertIsInstance(parser_builder, TestParserBuilder)

        for parser in parser_builder:
            pass

        self.assertRaises(StopIteration, next, parser_builder)

        self.assertEqual(hooks.start_count, 1)
        self.assertEqual(hooks.stop_count, 1)
        self.assertEqual(hooks.test_start_count, 5)
        self.assertEqual(hooks.test_success_count, 5)
        self.assertEqual(hooks.test_failure_count, 0)

    def test_parser_builder_config_with_errors(self):
        tests = [invalid_yaml]
        hooks = TestHooks()
        parser_builder_config = TestParserBuilderConfig(tests, hooks=hooks)

        parser_builder = TestParserBuilder(parser_builder_config)
        self.assertIsInstance(parser_builder, TestParserBuilder)

        for parser in parser_builder:
            pass

        self.assertRaises(StopIteration, next, parser_builder)

        self.assertEqual(hooks.start_count, 1)
        self.assertEqual(hooks.stop_count, 1)
        self.assertEqual(hooks.test_start_count, 1)
        self.assertEqual(hooks.test_success_count, 0)
        self.assertEqual(hooks.test_failure_count, 1)


if __name__ == '__main__':
    unittest.main()
