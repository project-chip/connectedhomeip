#
#    Copyright (c) 2022 Project CHIP Authors
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

import yaml
import string
from enum import Enum

from .constraints import get_constraints
from . import YamlFixes

_TESTS_SECTION = [
    'name',
    'config',
    'tests',
    'PICS',
]

_TEST_SECTION = [
    'label',
    'cluster',
    'command',
    'disabled',
    'endpoint',
    'identity',
    'fabricFiltered',
    'verification',
    'nodeId',
    'attribute',
    'optional',
    'PICS',
    'arguments',
    'response',
    'minInterval',
    'maxInterval',
    'timedInteractionTimeoutMs',
    'busyWaitMs',
]

_TEST_ARGUMENTS_SECTION = [
    'values',
    'value',
]

_TEST_RESPONSE_SECTION = [
    'value',
    'values',
    'error',
    'constraints',
    'type',
    'hasMasksSet',
    'contains',
    'saveAs'
]

_ATTRIBUTE_COMMANDS = [
    'readAttribute',
    'writeAttribute',
    'subscribeAttribute',
]

_EVENT_COMMANDS = [
    'readEvent',
    'subscribeEvent',
]


class PostProcessCheckStatus(Enum):
    SUCCESS = 'success',
    WARNING = 'warning',
    ERROR = 'error'


# TODO these values are just what was there before, these should be updated
class PostProcessCheckType(Enum):
    IM_STATUS = 'Error',
    CLUSTER_STATUS = 'ClusterError',
    RESPONSE_VALIDATION = 'Response',
    CONSTRAINT_VALIDATION = 'Constraints',
    SAVE_AS_VARIABLE = 'SaveAs'


# Each 'check' add an entry into the logs db. This entry contains the success of failure state as well as a log
# message describing the check itself.
# A check state can be any of the three valid state:
#  * success: Indicates that the check was successfull
#  * failure: Indicates that the check was unsuccessfull
#  * warning: Indicates that the check is probably successful but that something needs to be considered.
class PostProcessCheck:
    def __init__(self, state: PostProcessCheckStatus, category: PostProcessCheckType, message: str):
        self.state = state
        self.category = category
        self.message = message

    def is_success(self) -> bool:
        return self.state == PostProcessCheckStatus.SUCCESS

    def is_warning(self) -> bool:
        return self.state == PostProcessCheckStatus.WARNING

    def is_error(self) -> bool:
        return self.state == PostProcessCheckStatus.ERROR


class PostProcessResponseResult:
    ''' asdf

    There are multiple steps that occur when post processing a response. This is a summary of the
    results. Note that the number and types of steps performed is dependant on test step itself.
    '''

    def __init__(self):
        self.entries = []
        self.successes = 0
        self.warnings = 0
        self.errors = 0

    def success(self, category: PostProcessCheckType, message: str):
        ''' Adds a success entry that occured when post processing response to results.'''
        self.__insert(PostProcessCheckStatus.SUCCESS, category, message)
        self.successes += 1

    def warning(self, category: PostProcessCheckType, message: str):
        ''' Adds a warning entry that occured when post processing response to results.'''
        self.__insert(PostProcessCheckStatus.WARNING, category, message)
        self.warnings += 1

    def error(self, category: PostProcessCheckType, message: str):
        ''' Adds an error entry that occured when post processing response to results.'''
        self.__insert(PostProcessCheckStatus.ERROR, category, message)
        self.errors += 1

    def is_success(self):
        # It is possible that post processing a response doesn't have any success entires added
        # that is why we explicitly only search for if an error occurred.
        return self.errors == 0

    def is_failure(self):
        return self.errors != 0

    def __insert(self, state: PostProcessCheckStatus, category: PostProcessCheckType, message: str):
        log = PostProcessCheck(state, category, message)
        self.entries.append(log)


def _check_valid_keys(section, valid_keys_dict):
    if section:
        for key in section:
            if not key in valid_keys_dict:
                print(f'Unknown key: {key}')
                raise KeyError


def _valueOrNone(data, key):
    return data[key] if key in data else None


def _valueOrConfig(data, key, config):
    return data[key] if key in data else config[key]


class TestStep:
    is_enabled: True
    is_command: False
    is_attribute: False
    is_event: False

    def __init__(self, test: dict, config: dict, definitions):
        # Disabled tests are not parsed in order to allow the test to be added to the test
        # suite even if the feature is not implemented yet.
        self.is_enabled = not ('disabled' in test and test['disabled'])
        if not self.is_enabled:
            return

        _check_valid_keys(test, _TEST_SECTION)

        self.label = _valueOrNone(test, 'label')
        self.optional = _valueOrNone(test, 'optional')
        self.nodeId = _valueOrConfig(test, 'nodeId', config)
        self.cluster = _valueOrConfig(test, 'cluster', config)
        self.command = _valueOrConfig(test, 'command', config)
        self.attribute = _valueOrNone(test, 'attribute')
        self.endpoint = _valueOrConfig(test, 'endpoint', config)

        self.identity = _valueOrNone(test, 'identity')
        self.fabricFiltered = _valueOrNone(test, 'fabricFiltered')
        self.minInterval = _valueOrNone(test, 'minInterval')
        self.maxInterval = _valueOrNone(test, 'maxInterval')
        self.timedInteractionTimeoutMs = _valueOrNone(test, 'timedInteractionTimeoutMs')
        self.busyWaitMs = _valueOrNone(test, 'busyWaitMs')

        self.is_attribute = self.command in _ATTRIBUTE_COMMANDS
        self.is_event = self.command in _EVENT_COMMANDS

        self.arguments = _valueOrNone(test, 'arguments')
        self.response = _valueOrNone(test, 'response')

        _check_valid_keys(self.arguments, _TEST_ARGUMENTS_SECTION)
        _check_valid_keys(self.response, _TEST_RESPONSE_SECTION)

        if self.is_attribute:
            attribute = definitions.get_attribute_by_name(self.cluster, self.attribute)
            if attribute:
                attribute_definition = self.__as_mapping(definitions, self.cluster, attribute.definition.data_type.name)
                self.arguments = self.__update_with_definition(self.arguments, attribute_definition, config)
                self.response = self.__update_with_definition(self.response, attribute_definition, config)
        else:
            command = definitions.get_command_by_name(self.cluster, self.command)
            if command:
                command_definition = self.__as_mapping(definitions, self.cluster, command.input_param)
                response_definition = self.__as_mapping(definitions, self.cluster, command.output_param)

                self.arguments = self.__update_with_definition(self.arguments, command_definition, config)
                self.response = self.__update_with_definition(self.response, response_definition, config)

    def post_process_response(self, response, config):
        result = PostProcessResponseResult()

        if (not self.__maybe_check_optional(response, result)):
            return result

        self.__maybe_check_error(response, result)
        self.__maybe_check_cluster_error(response, result)
        self.__maybe_check_values(response, config, result)
        self.__maybe_check_constraints(response, config, result)
        self.__maybe_save_as(response, config, result)

        return result

    def __maybe_check_optional(self, response, result):
        if not self.optional or not 'error' in response:
            return True

        error = response['error']
        if error == 'UNSUPPORTED_ATTRIBUTE' or error == 'UNSUPPORTED_COMMAND':
            # result.warning('Optional', f'The response contains the error: "{error}".')
            return False

        return True

    def __maybe_check_error(self, response, result):
        if self.response and 'error' in self.response:
            expectedError = self.response['error']
            if 'error' in response:
                receivedError = response['error']
                if expectedError == receivedError:
                    result.success(PostProcessCheckType.IM_STATUS,
                                   f'The test expects the "{expectedError}" error which occured successfully.')
                else:
                    result.error(PostProcessCheckType.IM_STATUS,
                                 f'The test expects the "{expectedError}" error but the "{receivedError}" error occured.')
            else:
                result.error(PostProcessCheckType.IM_STATUS, f'The test expects the "{expectedError}" error but no error occured.')
        elif not self.response or not 'error' in self.response:
            if 'error' in response:
                receivedError = response['error']
                result.error(PostProcessCheckType.IM_STATUS, f'The test expects no error but the "{receivedError}" error occured.')
            # Handle generic success/errors
            elif response == 'failure':
                receivedError = response
                result.error(PostProcessCheckType.IM_STATUS, f'The test expects no error but the "{receivedError}" error occured.')
            else:
                result.success(PostProcessCheckType.IM_STATUS, f'The test expects no error an no error occured.')

    def __maybe_check_cluster_error(self, response, result):
        if self.response and 'clusterError' in self.response:
            expectedError = self.response['clusterError']
            if 'clusterError' in response:
                receivedError = response['clusterError']
                if expectedError == receivedError:
                    result.success(PostProcessCheckType.CLUSTER_STATUS,
                                   f'The test expects the "{expectedError}" error which occured successfully.')
                else:
                    result.error(PostProcessCheckType.CLUSTER_STATUS,
                                 f'The test expects the "{expectedError}" error but the "{receivedError}" error occured.')
            else:
                result.error(PostProcessCheckType.CLUSTER_STATUS,
                             f'The test expects the "{expectedError}" error but no error occured.')

    def __maybe_check_values(self, response, config, result):
        if not self.response or not 'values' in self.response:
            return

        if not 'value' in response:
            result.error(PostProcessCheckType.RESPONSE_VALIDATION, f'The test expects some values but none was received.')
            return

        expected_entries = self.response['values']
        received_entry = response['value']

        for expected_entry in expected_entries:
            if type(expected_entry) is dict and type(received_entry) is dict:
                if not 'value' in expected_entry:
                    continue

                expected_name = expected_entry['name']
                expected_value = expected_entry['value']
                if not expected_name in received_entry:
                    if expected_value is None:
                        result.success(PostProcessCheckType.RESPONSE_VALIDATION,
                                       f'The test expectation "{expected_name} == {expected_value}" is true')
                    else:
                        result.error(
                            PostProcessCheckType.RESPONSE_VALIDATION, f'The test expects a value named "{expected_name}" but it does not exists in the response."')
                    return

                received_value = received_entry[expected_name]
                self.__check_value(expected_name, expected_value, received_value, result)
            else:
                if not 'value' in expected_entries[0]:
                    continue

                expected_value = expected_entry['value']
                received_value = received_entry
                self.__check_value('value', expected_value, received_value, result)

    def __check_value(self, name, expected_value, received_value, result):
        # TODO Supports Array/List. See an exemple of failure in TestArmFailSafe.yaml
        if expected_value == received_value:
            result.success(PostProcessCheckType.RESPONSE_VALIDATION, f'The test expectation "{name} == {expected_value}" is true')
        else:
            result.error(PostProcessCheckType.RESPONSE_VALIDATION, f'The test expectation "{name} == {expected_value}" is false')

    def __maybe_check_constraints(self, response, config, result):
        if not self.response or not 'constraints' in self.response:
            return

        # TODO eventually move getting contraints into __update_placeholder
        # TODO We need to provide config to get_constraints and perform substitutions.
        #    if type(constraint_value) is str and constraint_value in config:
        #        constraint_value = config[constraint_value]
        constraints = get_constraints(self.response['constraints'])

        received_value = response['value']
        if all([constraint.is_met(received_value) for constraint in constraints]):
            result.success(PostProcessCheckType.CONSTRAINT_VALIDATION, f'Constraints check passed')
        else:
            # TODO would be helpful to be more verbose here
            result.error(PostProcessCheckType.CONSTRAINT_VALIDATION, f'Constraints check failed')

    def __maybe_save_as(self, response, config, result):
        if not self.response or 'values' not in self.response:
            return

        if not 'value' in response:
            result.error(PostProcessCheckType.SAVE_AS_VARIABLE, f'The test expects some values but none was received.')
            return

        expected_entries = self.response['values']
        received_entry = response['value']

        for expected_entry in expected_entries:
            if not 'saveAs' in expected_entry:
                continue
            saveAs = expected_entry['saveAs']

            if type(expected_entry) is dict and type(received_entry) is dict:
                expected_name = expected_entry['name']
                if not expected_name in received_entry:
                    result.error(
                        PostProcessCheckType.SAVE_AS_VARIABLE, f'The test expects a value named "{expected_name}" but it does not exists in the response."')
                    continue

                received_value = received_entry[expected_name]
                config[saveAs] = received_value
                result.success(PostProcessCheckType.SAVE_AS_VARIABLE, f'The test save the value "{received_value}" as {saveAs}.')
            else:
                received_value = received_entry
                config[saveAs] = received_value
                result.success(PostProcessCheckType.SAVE_AS_VARIABLE, f'The test save the value "{received_value}" as {saveAs}.')

    def __as_mapping(self, definitions, cluster_name, target_name):
        element = definitions.get_type_by_name(cluster_name, target_name)

        if hasattr(element, 'base_type'):
            target_name = element.base_type.lower()
        elif hasattr(element, 'fields'):
            target_name = {f.name: self.__as_mapping(definitions, cluster_name, f.data_type.name) for f in element.fields}
        elif target_name:
            target_name = target_name.lower()

        return target_name

    def __update_with_definition(self, container, mapping_type, config):
        if not container or not mapping_type:
            return container

        for key, items in container.items():
            if type(items) is list and key == 'values':
                rv = []
                for item in items:
                    newItem = {}
                    for item_key in item:
                        if item_key == 'value':
                            newItem[item_key] = self.__update_value_with_definition(
                                item['value'], mapping_type[item['name']], config)
                        else:
                            if item_key == 'saveAs' and not item[item_key] in config:
                                config[item[item_key]] = None
                            newItem[item_key] = item[item_key]
                    rv.append(newItem)
                container[key] = rv
            elif key == 'value' or key == 'values':
                if 'saveAs' in container and not container['saveAs'] in config:
                    config[container['saveAs']] = None
                rv = self.__update_value_with_definition(items, mapping_type, config)
                container[key] = rv
            elif key == 'constraints':
                for constraint in container[key]:
                    container[key][constraint] = self.__update_value_with_definition(
                        container[key][constraint], mapping_type, config)
        return container

    def __update_value_with_definition(self, value, mapping_type, config):
        if not mapping_type:
            return value

        if type(value) is dict:
            rv = {}
            for key in value:
                # FabricIndex is a special case where the framework requires it to be passed even if it is not part of the
                # requested arguments per spec and not part of the XML definition.
                if key == 'FabricIndex' or key == 'fabricIndex':
                    rv[key] = value[key]  # int64u
                else:
                    mapping = mapping_type[key]
                    rv[key] = self.__update_value_with_definition(value[key], mapping, config)
            value = rv
        elif type(value) is list:
            value = [self.__update_value_with_definition(entry, mapping_type, config) for entry in value]
        elif value and not value in config:
            if mapping_type == 'int64u' or mapping_type == 'int64s' or mapping_type == 'bitmap64' or mapping_type == 'epoch_us':
                value = YamlFixes.try_apply_yaml_cpp_longlong_limitation_fix(value)
                value = YamlFixes.try_apply_yaml_unrepresentable_integer_for_javascript_fixes(value)
            elif mapping_type == 'single' or mapping_type == 'double':
                value = YamlFixes.try_apply_yaml_float_written_as_strings(value)
            elif mapping_type == 'octet_string' or mapping_type == 'long_octet_string':
                if value.startswith('hex:'):
                    value = bytes.fromhex(value[4:])
                else:
                    value = value.encode()
            elif mapping_type == 'boolean':
                value = bool(value)

        return value


class YamlTests:
    __tests: None
    __hook: None
    __index: 0

    count: 0

    def __init__(self, tests, hook):
        self.__tests = tests
        self.__hook = hook
        self.__index = 0

        self.count = len(tests)

    def __iter__(self):
        return self

    def __next__(self):
        if self.__index < self.count:
            data = self.__tests[self.__index]
            data = self.__hook(data)
            self.__index += 1
            return data

        raise StopIteration


class YamlParser:
    name: None
    PICS: None
    # TODO config should be internal (_config). Also can it actually be None, or
    config: dict = {}
    tests: None

    def __init__(self, test_file, pics_file, definitions):
        # TODO Needs supports for PICS file
        with open(test_file) as f:
            loader = yaml.FullLoader
            loader = YamlFixes.try_add_yaml_support_for_scientific_notation_without_dot(loader)

            data = yaml.load(f, Loader=loader)
            _check_valid_keys(data, _TESTS_SECTION)

            self.name = _valueOrNone(data, 'name')
            self.PICS = _valueOrNone(data, 'PICS')

            self.config = _valueOrNone(data, 'config')

            tests = list(filter(lambda test: test.is_enabled, [TestStep(
                test, self.config, definitions) for test in _valueOrNone(data, 'tests')]))
            YamlFixes.try_update_yaml_node_id_test_runner_state(tests, self.config)

            self.tests = YamlTests(tests, self.__update_placeholder)

    def __update_placeholder(self, data):
        data.arguments = self.__encode_values(data.arguments)
        data.response = self.__encode_values(data.response)
        return data

    def __encode_values(self, container):
        if not container:
            return None

        # TODO this should likely be moved to end of TestStep.__init__
        if 'value' in container:
            container['values'] = [{'name': 'value', 'value': container['value']}]
            del container['value']

        if 'values' in container:
            for idx, item in enumerate(container['values']):
                if 'value' in item:
                    container['values'][idx]['value'] = self.__config_variable_substitution(item['value'])

        # TODO this should likely be moved to end of TestStep.__init__. But depends on rationale
        if 'saveAs' in container:
            if not 'values' in container:
                # TODO Currently very unclear why this corner case is needed. Would be nice to add
                # information as to why this would happen.
                container['values'] = [{}]
            container['values'][0]['saveAs'] = container['saveAs']
            del container['saveAs']

        return container

    def __config_variable_substitution(self, value):
        if type(value) is list:
            return [self.__config_variable_substitution(entry) for entry in value]
        elif type(value) is dict:
            mapped_value = {}
            for key in value:
                mapped_value[key] = self.__config_variable_substitution(value[key])
            return mapped_value
        elif type(value) is str:
            # For most tests, a single config variable is used and it can be replaced as in.
            # But some other tests were relying on the fact that the expression was put 'as if' in
            # the generated code and was resolved before being sent over the wire. For such expressions
            # (e.g 'myVar + 1') we need to compute it before sending it over the wire.
            tokens = value.split()
            if len(tokens) == 0:
                return value

            substitution_occured = False
            for idx, token in enumerate(tokens):
                if token in self.config:
                    variable_info = self.config[token]
                    if type(variable_info) is dict and 'defaultValue' in variable_info:
                        variable_info = variable_info['defaultValue']
                    tokens[idx] = variable_info
                    substitution_occured = True

            if len(tokens) == 1:
                return tokens[0]

            tokens = [str(token) for token in tokens]
            value = ' '.join(tokens)
            # TODO we should move away from eval. That will mean that we will need to do extra parsing,
            # but it would be safer then just blindly running eval.
            return value if not substitution_occured else eval(value)
        else:
            return value

    def update_config(self, key, value):
        self.config[key] = value
