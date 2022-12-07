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
    'clusterError',
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
    'waitForReport',
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

        self._convert_single_value_to_values(self.arguments)
        self._convert_single_value_to_values(self.response)

        argument_mapping = None
        response_mapping = None

        if self.is_attribute:
            attribute = definitions.get_attribute_by_name(self.cluster, self.attribute)
            if attribute:
                attribute_mapping = self.__as_mapping(definitions, self.cluster, attribute.definition.data_type.name)
                argument_mapping = attribute_mapping
                response_mapping = attribute_mapping
        else:
            command = definitions.get_command_by_name(self.cluster, self.command)
            if command:
                argument_mapping = self.__as_mapping(definitions, self.cluster, command.input_param)
                response_mapping = self.__as_mapping(definitions, self.cluster, command.output_param)

        self.__update_with_definition(self.arguments, argument_mapping, config)
        self.__update_with_definition(self.response, response_mapping, config)

    def _convert_single_value_to_values(self, container):
        if container is None or 'values' in container:
            return

        # Attribute tests pass a single value argument that does not carry a name but
        # instead uses a generic 'value' keyword. Convert to keyword to be the single
        # members of the 'values' array which is what is used for other tests.
        value = {}

        known_keys_to_copy = ['value', 'constraints', 'saveAs']
        known_keys_to_allow = ['error', 'clusterError']

        for key, item in list(container.items()):
            if key in known_keys_to_copy:
                value[key] = item
                del container[key]
            elif key in known_keys_to_allow:
                # Nothing to do for those keys.
                pass
            else:
                raise KeyError(f'Unknown key: {key}')

        container['values'] = [value]

    def post_process_response(self, response, config):
        result = PostProcessResponseResult()

        if not self.__maybe_check_optional(response, result):
            return result

        self.__maybe_check_error(response, result)
        if self.response:
            self.__maybe_check_cluster_error(response, result)
            self.__maybe_check_values(response, config, result)
            self.__maybe_check_constraints(response, config, result)
            self.__maybe_save_as(response, config, result)

        return result

    def __maybe_check_optional(self, response, result):
        if not self.optional or not 'error' in response:
            return True

        received_error = response.get('error')
        if received_error == 'UNSUPPORTED_ATTRIBUTE' or received_error == 'UNSUPPORTED_COMMAND':
            # result.warning('Optional', f'The response contains the error: "{error}".')
            return False

        return True

    def __maybe_check_error(self, response, result):
        check_type = PostProcessCheckType.IM_STATUS
        error_success = 'The test expects the "{error}" error which occured successfully.'
        error_success_no_error = 'The test expects no error and no error occurred.'
        error_wrong_error = 'The test expects the "{error}" error but the "{value}" error occured.'
        error_unexpected_error = 'The test expects no error but the "{value}" error occured.'
        error_unexpected_success = 'The test expects the "{error}" error but no error occured.'

        expected_error = self.response.get('error') if self.response else None

        # Handle generic success/error
        if type(response) is str and response == 'failure':
            received_error = response
        elif type(response) is str and response == 'success':
            received_error = None
        else:
            received_error = response.get('error')

        if expected_error and received_error and expected_error == received_error:
            result.success(check_type, error_success.format(error=expected_error))
        elif expected_error and received_error:
            result.error(check_type, error_wrong_error.format(error=expected_error, value=received_error))
        elif expected_error and not received_error:
            result.error(check_type, error_unexpected_success.format(error=expected_error))
        elif not expected_error and received_error:
            result.error(check_type, error_unexpected_error.format(error=received_error))
        elif not expected_error and not received_error:
            result.success(check_type, error_success_no_error)
        else:
            # This should not happens
            raise AssertionError('This should not happens.')

    def __maybe_check_cluster_error(self, response, result):
        check_type = PostProcessCheckType.IM_STATUS
        error_success = 'The test expects the "{error}" error which occured successfully.'
        error_unexpected_success = 'The test expects the "{error}" error but no error occured.'
        error_wrong_error = 'The test expects the "{error}" error but the "{value}" error occured.'

        expected_error = self.response.get('clusterError')
        received_error = response.get('clusterError')

        if expected_error:
            if received_error and expected_error == received_error:
                result.success(check_type, error_success.format(error=expected_error))
            elif received_error:
                result.error(check_type, error_wrong_error.format(error=expected_error, value=received_error))
            else:
                result.error(check_type, error_unexpected_success.format(error=expected_error))
        else:
            # Nothing is logged here to not be redundant with the generic error checking code.
            pass

    def __maybe_check_values(self, response, config, result):
        check_type = PostProcessCheckType.RESPONSE_VALIDATION
        error_success = 'The test expectation "{name} == {value}" is true'
        error_failure = 'The test expectation "{name} == {value}" is false'
        error_name_does_not_exist = 'The test expects a value named "{name}" but it does not exists in the response."'

        for value in self.response['values']:
            if not 'value' in value:
                continue

            expected_name = 'value'
            received_value = response.get('value')
            if not self.is_attribute:
                expected_name = value.get('name')
                if not expected_name in received_value:
                    result.error(check_type, error_name_does_not_exist.format(name=expected_name))
                    continue

                received_value = received_value.get(expected_name) if received_value else None

            # TODO Supports Array/List. See an exemple of failure in TestArmFailSafe.yaml
            expected_value = value.get('value')
            if expected_value == received_value:
                result.success(check_type, error_success.format(name=expected_name, value=received_value))
            else:
                result.error(check_type, error_failure.format(name=expected_name, value=expected_value))

    def __maybe_check_constraints(self, response, config, result):
        check_type = PostProcessCheckType.CONSTRAINT_VALIDATION
        error_success = 'Constraints check passed'
        error_failure = 'Constraints check failed'
        error_name_does_not_exist = 'The test expects a value named "{name}" but it does not exists in the response."'

        for value in self.response['values']:
            if not 'constraints' in value:
                continue

            expected_name = 'value'
            received_value = response.get('value')
            if not self.is_attribute:
                expected_name = value.get('name')
                if not expected_name in received_value:
                    result.error(check_type, error_name_does_not_exist.format(name=expected_name))
                    continue

                received_value = received_value.get(expected_name) if received_value else None

            constraints = get_constraints(value['constraints'])
            if all([constraint.is_met(received_value) for constraint in constraints]):
                result.success(check_type, error_success)
            else:
                # TODO would be helpful to be more verbose here
                result.error(check_type, error_failure)

    def __maybe_save_as(self, response, config, result):
        check_type = PostProcessCheckType.SAVE_AS_VARIABLE
        error_success = 'The test save the value "{value}" as {name}.'
        error_name_does_not_exist = 'The test expects a value named "{name}" but it does not exists in the response."'

        for value in self.response['values']:
            if not 'saveAs' in value:
                continue

            expected_name = 'value'
            received_value = response.get('value')
            if not self.is_attribute:
                expected_name = value.get('name')
                if not expected_name in received_value:
                    result.error(check_type, error_name_does_not_exist.format(name=expected_name))
                    continue

                received_value = received_value.get(expected_name) if received_value else None

            save_as = value.get('saveAs')
            config[save_as] = received_value
            result.success(check_type, error_success.format(value=received_value, name=save_as))

    def __as_mapping(self, definitions, cluster_name, target_name):
        element = definitions.get_type_by_name(cluster_name, target_name)

        if hasattr(element, 'base_type'):
            target_name = element.base_type.lower()
        elif hasattr(element, 'fields'):
            target_name = {f.name: self.__as_mapping(definitions, cluster_name, f.data_type.name) for f in element.fields}
        elif target_name:
            target_name = target_name.lower()

        return target_name

    def __update_with_definition(self, container: dict, mapping_type, config: dict):
        if not container or not mapping_type:
            return

        for value in list(container['values']):
            for key, item_value in list(value.items()):
                mapping = mapping_type if self.is_attribute else mapping_type[value['name']]

                if key == 'value':
                    value[key] = self.__update_value_with_definition(item_value, mapping, config)
                elif key == 'saveAs' and type(item_value) is str and not item_value in config:
                    config[item_value] = None
                elif key == 'constraints':
                    for constraint, constraint_value in item_value.items():
                        value[key][constraint] = self.__update_value_with_definition(constraint_value, mapping_type, config)
                else:
                    # This key, value pair does not rely on cluster specifications.
                    pass

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
            return rv
        if type(value) is list:
            return [self.__update_value_with_definition(entry, mapping_type, config) for entry in value]
        # TODO currently I am unsure if the check of `value not in config` is sufficant. For
        # example let's say value = 'foo + 1' and map type is 'int64u', we would arguably do
        # the wrong thing below.
        if value is not None and value not in config:
            if mapping_type == 'int64u' or mapping_type == 'int64s' or mapping_type == 'bitmap64' or mapping_type == 'epoch_us':
                value = YamlFixes.try_apply_yaml_cpp_longlong_limitation_fix(value)
                value = YamlFixes.try_apply_yaml_unrepresentable_integer_for_javascript_fixes(value)
            elif mapping_type == 'single' or mapping_type == 'double':
                value = YamlFixes.try_apply_yaml_float_written_as_strings(value)
            elif mapping_type == 'octet_string' or mapping_type == 'long_octet_string':
                value = YamlFixes.convert_yaml_octet_string_to_bytes(value)
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
    # TODO config should be internal (_config)
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
        self.__encode_values(data.arguments)
        self.__encode_values(data.response)
        return data

    def __encode_values(self, container):
        if not container:
            return

        values = container['values']

        for idx, item in enumerate(values):
            if 'value' in item:
                values[idx]['value'] = self.__config_variable_substitution(item['value'])

            if 'constraints' in item:
                for constraint, constraint_value in item['constraints'].items():
                    values[idx]['constraints'][constraint] = self.__config_variable_substitution(constraint_value)

        container['values'] = values

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
