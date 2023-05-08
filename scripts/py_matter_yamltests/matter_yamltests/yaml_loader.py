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

from typing import Union

from .errors import (TestStepError, TestStepGroupResponseError, TestStepInvalidTypeError, TestStepKeyError,
                     TestStepNodeIdAndGroupIdError, TestStepValueAndValuesError, TestStepVerificationStandaloneError,
                     TestStepWaitResponseError)
from .fixes import add_yaml_support_for_scientific_notation_without_dot

try:
    from yaml import CSafeLoader as SafeLoader
except:
    from yaml import SafeLoader

import os

import yaml


class YamlLoader:
    """This class loads a file from the disk and validates that the content is a well formed yaml test."""

    def load(self, yaml_file: str) -> tuple[str, Union[list, str], dict, list]:
        filename = ''
        name = ''
        pics = None
        config = {}
        tests = []

        if yaml_file:
            filename = os.path.splitext(os.path.basename(yaml_file))[0]
            with open(yaml_file) as f:
                loader = SafeLoader
                add_yaml_support_for_scientific_notation_without_dot(loader)
                content = yaml.load(f, Loader=loader)

                self.__check_content(content)

                name = content.get('name', '')
                pics = content.get('PICS')
                config = content.get('config', {})
                tests = content.get('tests', [])

        return (filename, name, pics, config, tests)

    def __check_content(self, content):
        schema = {
            'name': str,
            'PICS': (str, list),
            'config': dict,
            'tests': list,
        }

        try:
            self.__check(content, schema)
        except TestStepError as e:
            if 'tests' in content:
                # This is a top level error. The content of the tests section
                # does not really matter here and dumping it may be counter-productive
                # since it can be very long...
                content['tests'] = 'Skipped...'
            e.update_context(content, 0)
            raise

        tests = content.get('tests', [])
        for step_index, step in enumerate(tests):
            try:
                self.__check_test_step(step)
            except TestStepError as e:
                e.update_context(step, step_index)
                raise

    def __check_test_step(self, content):
        schema = {
            'label': str,
            'identity': str,
            'nodeId': int,
            'groupId': int,
            'endpoint': int,
            'cluster': str,
            'attribute': str,
            'command': str,
            'event': str,
            'eventNumber': (int, str),  # Can be a variable.
            'disabled': bool,
            'fabricFiltered': bool,
            'verification': str,
            'PICS': str,
            'arguments': dict,
            'response': (dict, list),
            'minInterval': int,
            'maxInterval': int,
            'timedInteractionTimeoutMs': int,
            'busyWaitMs': int,
            'wait': str,
        }

        self.__check(content, schema)
        self.__rule_node_id_and_group_id_are_mutually_exclusive(content)
        self.__rule_group_step_should_not_expect_a_response(content)
        self.__rule_step_with_verification_should_be_disabled_or_interactive(
            content)
        self.__rule_wait_should_not_expect_a_response(content)

        if 'arguments' in content:
            arguments = content.get('arguments')
            self.__check_test_step_arguments(arguments)

        if 'response' in content:
            response = content.get('response')
            if isinstance(response, list):
                [self.__check_test_step_response(x) for x in response]
            else:
                self.__check_test_step_response(response)

    def __check_test_step_arguments(self, content):
        schema = {
            'values': list,
            'value': (type(None), bool, str, int, float, dict, list),
        }

        self.__check(content, schema)

        if 'values' in content:
            values = content.get('values')
            for value in values:
                [self.__check_test_step_argument_value(x) for x in values]

    def __check_test_step_argument_value(self, content):
        schema = {
            'value': (type(None), bool, str, int, float, dict, list),
            'name': str,
        }

        self.__check(content, schema)

    def __check_test_step_response(self, content):
        self.__rule_response_value_and_values_are_mutually_exclusive(content)

        if 'values' in content:
            self.__check_type('values', content, list)
            values = content.get('values')
            [self.__check_test_step_response_value(
                x, allow_name_key=True) for x in values]
        else:
            self.__check_test_step_response_value(content)

    def __check_test_step_response_value(self, content, allow_name_key=False):
        schema = {
            'value': (type(None), bool, str, int, float, dict, list),
            'name': str,
            'error': str,
            'clusterError': int,
            'constraints': dict,
            'saveAs': str
        }

        if allow_name_key:
            schema['name'] = str

        self.__check(content, schema)

        if 'constraints' in content:
            constraints = content.get('constraints')
            self.__check_test_step_response_value_constraints(constraints)

    def __check_test_step_response_value_constraints(self, content):
        schema = {
            'hasValue': bool,
            'type': str,
            'minLength': int,
            'maxLength': int,
            'isHexString': bool,
            'startsWith': str,
            'endsWith': str,
            'isUpperCase': bool,
            'isLowerCase': bool,
            'minValue': (int, float, str),  # Can be a variable
            'maxValue': (int, float, str),  # Can be a variable
            'contains': list,
            'excludes': list,
            'hasMasksSet': list,
            'hasMasksClear': list,
            'notValue': (type(None), bool, str, int, float, list, dict)
        }

        self.__check(content, schema)

    def __check(self, content, schema):
        for key in content:
            if key not in schema:
                raise TestStepKeyError(content, key)

            self.__check_type(key, content, schema.get(key))

    def __check_type(self, key, content, expected_type):
        value = content.get(key)
        if isinstance(expected_type, tuple) and type(value) not in expected_type:
            raise TestStepInvalidTypeError(content, key, expected_type)
        elif not isinstance(expected_type, tuple) and type(value) is not expected_type:
            raise TestStepInvalidTypeError(content, key, expected_type)

    def __rule_node_id_and_group_id_are_mutually_exclusive(self, content):
        if 'nodeId' in content and 'groupId' in content:
            raise TestStepNodeIdAndGroupIdError(content)

    def __rule_group_step_should_not_expect_a_response(self, content):
        if 'groupId' in content and 'response' in content:
            response = content.get('response')
            if 'value' in response or 'values' in response:
                raise TestStepGroupResponseError(content)

    def __rule_step_with_verification_should_be_disabled_or_interactive(self, content):
        if 'verification' in content:
            disabled = content.get('disabled')
            command = content.get('command')
            if disabled != True and command != 'UserPrompt':
                raise TestStepVerificationStandaloneError(content)

    def __rule_response_value_and_values_are_mutually_exclusive(self, content):
        if 'value' in content and 'values' in content:
            raise TestStepValueAndValuesError(content)

    def __rule_wait_should_not_expect_a_response(self, content):
        if 'wait' in content and 'response' in content:
            raise TestStepWaitResponseError(content)
