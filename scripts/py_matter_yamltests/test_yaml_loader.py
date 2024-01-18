#
#    Copyright (c) 2023 Project CHIP Authors
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

import unittest
from unittest.mock import mock_open, patch

from matter_yamltests.errors import (TestStepGroupResponseError, TestStepInvalidTypeError, TestStepKeyError,
                                     TestStepNodeIdAndGroupIdError, TestStepValueAndValuesError,
                                     TestStepVerificationStandaloneError)
from matter_yamltests.yaml_loader import YamlLoader


def mock_open_with_parameter_content(content):
    file_object = mock_open(read_data=content).return_value
    file_object.__iter__.return_value = content.splitlines(True)
    return file_object


@patch('builtins.open', new=mock_open_with_parameter_content)
class TestYamlLoader(unittest.TestCase):
    def _get_wrong_values(self, valid_types, spaces=2):
        values = []

        if type(None) not in valid_types:
            values.append('')

        if str not in valid_types:
            values.append('A Test')

        if bool not in valid_types:
            values.append(True)

        if int not in valid_types:
            values.append(2)

        if float not in valid_types:
            values.append(2.1)

        if dict not in valid_types:
            values.append('\n' + (spaces * ' ') +
                          'value: True\n' + (spaces * ' ') + 'values: False')

        if list not in valid_types:
            values.append('\n' + (spaces * ' ') +
                          '- value: Test1\n' + (spaces * ' ') + '- value: Test2')

        return values

    def test_missing_file(self):
        load = YamlLoader().load

        content = None

        filename, name, pics, config, tests = load(content)
        self.assertEqual(filename, '')
        self.assertEqual(name, '')
        self.assertEqual(pics, None)
        self.assertEqual(config, {})
        self.assertEqual(tests, [])

    def test_empty_file(self):
        load = YamlLoader().load

        content = ''

        filename, name, pics, config, tests = load(content)
        self.assertEqual(name, '')
        self.assertEqual(name, '')
        self.assertEqual(pics, None)
        self.assertEqual(config, {})
        self.assertEqual(tests, [])

    def test_key_unknown(self):
        load = YamlLoader().load

        content = '''
        unknown: Test Name
        '''

        self.assertRaises(TestStepKeyError, load, content)

    def test_key_name(self):
        load = YamlLoader().load

        content = '''
        name: Test Name
        '''

        _, name, _, _, _ = load(content)
        self.assertEqual(name, 'Test Name')

    def test_key_name_wrong_values(self):
        load = YamlLoader().load

        key = 'name'
        values = self._get_wrong_values([str])
        [self.assertRaises(TestStepInvalidTypeError, load,
                           f'{key}: {x}') for x in values]

    def test_key_pics_string(self):
        load = YamlLoader().load

        content = '''
        PICS: OO.S
        '''

        _, _, pics, _, _ = load(content)
        self.assertEqual(pics, 'OO.S')

    def test_key_pics_list(self):
        load = YamlLoader().load

        content = '''
        PICS:
            - OO.S
            - OO.C
        '''

        _, _, pics, _, _ = load(content)
        self.assertEqual(pics, ['OO.S', 'OO.C'])

    def test_key_pics_wrong_values(self):
        load = YamlLoader().load

        key = 'PICS'
        values = self._get_wrong_values([str, list])
        [self.assertRaises(TestStepInvalidTypeError, load,
                           f'{key}: {x}') for x in values]

    def test_key_config(self):
        load = YamlLoader().load

        content = '''
        config:
            name: value
            name2: value2
        '''

        _, _, _, config, _ = load(content)
        self.assertEqual(config, {'name': 'value', 'name2': 'value2'})

    def test_key_config_wrong_values(self):
        load = YamlLoader().load

        key = 'config'
        values = self._get_wrong_values([dict])
        [self.assertRaises(TestStepInvalidTypeError, load,
                           f'{key}: {x}') for x in values]

    def test_key_tests(self):
        load = YamlLoader().load

        content = '''
        tests:
            - label: Test1
            - label: Test2
        '''

        _, _, _, _, tests = load(content)
        self.assertEqual(tests, [{'label': 'Test1'}, {'label': 'Test2'}])

    def test_key_tests_wrong_values(self):
        load = YamlLoader().load

        key = 'tests'
        values = self._get_wrong_values([list])
        [self.assertRaises(TestStepInvalidTypeError, load,
                           f'{key}: {x}') for x in values]

    def test_key_tests_step_unknown_key(self):
        load = YamlLoader().load

        content = '''
        tests:
            - unknown: Test2
        '''

        self.assertRaises(TestStepKeyError, load, content)

    def test_key_tests_step_bool_keys(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - {key}: {value}')
        keys = [
            'disabled',
            'fabricFiltered',
        ]

        wrong_values = self._get_wrong_values([bool], spaces=6)
        for key in keys:
            _, _, _, _, tests = load(content.format(key=key, value=True))
            self.assertEqual(tests, [{key: True}])

            for value in wrong_values:
                x = content.format(key=key, value=value)
                self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_str_keys(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - {key}: {value}')
        keys = [
            'label',
            'identity',
            'cluster',
            'attribute',
            'command',
            'event',
            'PICS',
            'wait',
        ]

        # NOTE: 'verification' is excluded from this list despites beeing a key of type
        #       str. This is because 'verification' key has a rule that requires it to
        #       tied with either a 'disabled: True' or a 'command: UserPrompt'.
        #       As such it has dedicated tests.

        wrong_values = self._get_wrong_values([str], spaces=6)
        for key in keys:
            _, _, _, _, tests = load(content.format(key=key, value='a string'))
            self.assertEqual(tests, [{key: 'a string'}])

            for value in wrong_values:
                x = content.format(key=key, value=value)
                self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_int_keys(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - {key}: {value}')
        keys = [
            'minInterval',
            'maxInterval',
            'timedInteractionTimeoutMs',
            'busyWaitMs',
        ]

        wrong_values = self._get_wrong_values([int], spaces=6)
        for key in keys:
            _, _, _, _, tests = load(content.format(key=key, value=1))
            self.assertEqual(tests, [{key: 1}])

            for value in wrong_values:
                x = content.format(key=key, value=value)
                self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_dict_keys(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - command: writeAttribute\n'
                   '    {key}: {value}')
        keys = [
            'arguments',
        ]

        valid_value = ('\n'
                       '      value: True\n')
        wrong_values = self._get_wrong_values([dict], spaces=6)
        for key in keys:
            _, _, _, _, tests = load(
                content.format(key=key, value=valid_value))
            self.assertEqual(
                tests, [{'command': 'writeAttribute', key: {'value': True}}])

            for value in wrong_values:
                x = content.format(key=key, value=value)
                self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_response_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response: {value}')

        value = ('\n'
                 '      value: True\n')
        _, _, _, _, tests = load(content.format(value=value))
        self.assertEqual(tests, [{'response': {'value': True}}])

        value = ('\n'
                 '      - value: True\n')
        _, _, _, _, tests = load(content.format(value=value))
        self.assertEqual(tests, [{'response': [{'value': True}]}])

        wrong_values = self._get_wrong_values([dict, list, str], spaces=6)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_endpoint_number_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - endpoint: {value}')

        _, _, _, _, tests = load(content.format(value=1))
        self.assertEqual(tests, [{'endpoint': 1}])

        _, _, _, _, tests = load(content.format(value='TestKey'))
        self.assertEqual(tests, [{'endpoint': 'TestKey'}])

        wrong_values = self._get_wrong_values([str, int], spaces=6)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_group_id_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - groupId: {value}')

        _, _, _, _, tests = load(content.format(value=1))
        self.assertEqual(tests, [{'groupId': 1}])

        _, _, _, _, tests = load(content.format(value='TestKey'))
        self.assertEqual(tests, [{'groupId': 'TestKey'}])

        wrong_values = self._get_wrong_values([str, int], spaces=6)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_node_id_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - nodeId: {value}')

        _, _, _, _, tests = load(content.format(value=1))
        self.assertEqual(tests, [{'nodeId': 1}])

        _, _, _, _, tests = load(content.format(value='TestKey'))
        self.assertEqual(tests, [{'nodeId': 'TestKey'}])

        wrong_values = self._get_wrong_values([str, int], spaces=6)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_event_number_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - eventNumber: {value}')

        _, _, _, _, tests = load(content.format(value=1))
        self.assertEqual(tests, [{'eventNumber': 1}])

        _, _, _, _, tests = load(content.format(value='TestKey'))
        self.assertEqual(tests, [{'eventNumber': 'TestKey'}])

        wrong_values = self._get_wrong_values([str, int], spaces=6)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_verification_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - verification: {value}\n'
                   '    disabled: true')

        _, _, _, _, tests = load(content.format(value='Test Sentence'))
        self.assertEqual(
            tests, [{'verification': 'Test Sentence', 'disabled': True}])

        wrong_values = self._get_wrong_values([str, int], spaces=6)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

        # TODO
        # 'verification',

    def test_key_tests_step_rule_node_id_and_group_id_are_mutually_exclusive(self):
        load = YamlLoader().load

        content = '''
          tests:
              - label: A Test Name
                nodeId: 0
                groupId: 1
        '''

        self.assertRaises(TestStepNodeIdAndGroupIdError, load, content)

    def test_key_tests_step_rule_group_step_should_not_expect_a_response(self):
        load = YamlLoader().load

        content = '''
          tests:
              - label: A Test Name
                groupId: 1
                response:
                    value: An expected value
        '''

        self.assertRaises(TestStepGroupResponseError, load, content)

    def test_key_tests_step_rule_step_with_verification_should_be_disabled_or_interactive(self):
        load = YamlLoader().load

        content = '''
          tests:
              - label: A Test Name
                verification: A verification sentence
        '''

        self.assertRaises(TestStepVerificationStandaloneError, load, content)

        content = '''
          tests:
              - label: A Test Name
                verification: A verification sentence
                disabled: false
        '''

        self.assertRaises(TestStepVerificationStandaloneError, load, content)

        content = '''
          tests:
              - label: A Test Name
                verification: A verification sentence
                disabled: true
        '''

        _, _, _, _, tests = load(content)
        self.assertEqual(tests, [
                         {'label': 'A Test Name', 'verification': 'A verification sentence', 'disabled': True}])

        content = '''
          tests:
              - label: A Test Name
                verification: A verification sentence
                command: Something
        '''

        self.assertRaises(TestStepVerificationStandaloneError, load, content)

        content = '''
          tests:
              - label: A Test Name
                verification: A verification sentence
                command: UserPrompt
        '''

        _, _, _, _, tests = load(content)
        self.assertEqual(tests, [
                         {'label': 'A Test Name', 'verification': 'A verification sentence', 'command': 'UserPrompt'}])

    def test_key_tests_step_response_key_value_key(self):
        # NOTE: The value key can be of any type.
        pass

    def test_key_tests_step_response_key_values_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response:\n'
                   '      values: {value}')

        _, _, _, _, tests = load(content.format(value=[]))
        self.assertEqual(tests, [{'response': {'values': []}}])

        wrong_values = self._get_wrong_values([list], spaces=8)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_response_key_error_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response:\n'
                   '      error: {value}')

        _, _, _, _, tests = load(content.format(value='AnError'))
        self.assertEqual(tests, [{'response': {'error': 'AnError'}}])

        wrong_values = self._get_wrong_values([str], spaces=8)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_response_key_cluster_error_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response:\n'
                   '      clusterError: {value}')

        _, _, _, _, tests = load(content.format(value=1))
        self.assertEqual(tests, [{'response': {'clusterError': 1}}])

        wrong_values = self._get_wrong_values([int], spaces=8)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_response_key_constraints_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response:\n'
                   '      constraints: {value}')

        _, _, _, _, tests = load(content.format(value={}))
        self.assertEqual(tests, [{'response': {'constraints': {}}}])

        wrong_values = self._get_wrong_values([dict], spaces=8)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_key_tests_step_response_key_save_as_key(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response:\n'
                   '      saveAs: {value}')

        _, _, _, _, tests = load(content.format(value='AKey'))
        self.assertEqual(tests, [{'response': {'saveAs': 'AKey'}}])

        wrong_values = self._get_wrong_values([str], spaces=8)
        for value in wrong_values:
            x = content.format(value=value)
            self.assertRaises(TestStepInvalidTypeError, load, x)

    def test_rule_response_value_and_values_are_mutually_exclusive(self):
        load = YamlLoader().load

        content = ('tests:\n'
                   '  - response:\n'
                   '      value: 1\n'
                   '      values: []')

        self.assertRaises(TestStepValueAndValuesError, load, content)

    # TODO Check constraints


if __name__ == '__main__':
    unittest.main()
