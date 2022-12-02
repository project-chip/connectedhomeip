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


# Each 'check' add an entry into the logs db. This entry contains the success of failure state as well as a log
# message describing the check itself.
# A check state can be any of the three valid state:
#  * success: Indicates that the check was successfull
#  * failure: Indicates that the check was unsuccessfull
#  * warning: Indicates that the check is probably successful but that something needs to be considered.
class YamlLog:
    def __init__(self, state, category, message):
        if not state == 'success' and not state == 'warning' and not state == 'error':
            raise ValueError

        self.state = state
        self.category = category
        self.message = message

    def is_success(self):
        return self.state == 'success'

    def is_warning(self):
        return self.state == 'warning'

    def is_error(self):
        return self.state == 'error'


class YamlLogger:
    def __init__(self):
        self.entries = []
        self.successes = 0
        self.warnings = 0
        self.errors = 0

    def success(self, category, message):
        self.__insert('success', category, message)
        self.successes += 1
        pass

    def warning(self, category, message):
        self.__insert('warning', category, message)
        self.warnings += 1
        pass

    def error(self, category, message):
        self.__insert('error', category, message)
        self.errors += 1
        pass

    def is_success(self):
        return self.errors == 0

    def is_failure(self):
        return self.errors != 0

    def __insert(self, state, category, message):
        log = YamlLog(state, category, message)
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


class YamlTest:
    isEnabled: True
    isCommand: False
    isAttribute: False
    isEvent: False

    def __init__(self, test, config, definitions):
        # disabled tests are not parsed in order to allow the test to be added to the test
        # suite even if the feature is not implemented yet.
        self.isEnabled = not ('disabled' in test and test['disabled'])
        if not self.isEnabled:
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

        self.isAttribute = self.command in _ATTRIBUTE_COMMANDS
        self.isEvent = self.command in _EVENT_COMMANDS

        self.arguments = _valueOrNone(test, 'arguments')
        self.response = _valueOrNone(test, 'response')

        _check_valid_keys(self.arguments, _TEST_ARGUMENTS_SECTION)
        _check_valid_keys(self.response, _TEST_RESPONSE_SECTION)

        if self.isAttribute:
            attribute_definition = definitions.get_attribute_definition(self.attribute)
            self.arguments = self.__update_with_definition(self.arguments, attribute_definition, config)
            self.response = self.__update_with_definition(self.response, attribute_definition, config)
        else:
            command_definition = definitions.get_command_args_definition(self.command)
            response_definition = definitions.get_response_args_definition(self.command)
            self.arguments = self.__update_with_definition(self.arguments, command_definition, config)
            self.response = self.__update_with_definition(self.response, response_definition, config)

    def check_response(self, response, config):
        logger = YamlLogger()

        if (not self.__maybe_check_optional(response, logger)):
            return logger

        self.__maybe_check_error(response, logger)
        self.__maybe_check_cluster_error(response, logger)
        self.__maybe_check_values(response, config, logger)
        self.__maybe_check_constraints(response, config, logger)
        self.__maybe_save_as(response, config, logger)

        return logger

    def __maybe_check_optional(self, response, logger):
        if not self.optional or not 'error' in response:
            return True

        error = response['error']
        if error == 'UNSUPPORTED_ATTRIBUTE' or error == 'UNSUPPORTED_COMMAND':
            # logger.warning('Optional', f'The response contains the error: "{error}".')
            return False

        return True

    def __maybe_check_error(self, response, logger):
        if self.response and 'error' in self.response:
            expectedError = self.response['error']
            if 'error' in response:
                receivedError = response['error']
                if expectedError == receivedError:
                    logger.success('Error', f'The test expects the "{expectedError}" error which occured successfully.')
                else:
                    logger.error('Error', f'The test expects the "{expectedError}" error but the "{receivedError}" error occured.')
            else:
                logger.error('Error', f'The test expects the "{expectedError}" error but no error occured.')
        elif not self.response or not 'error' in self.response:
            if 'error' in response:
                receivedError = response['error']
                logger.error('Error', f'The test expects no error but the "{receivedError}" error occured.')
            # Handle generic success/errors
            elif response == 'failure':
                receivedError = response
                logger.error('Error', f'The test expects no error but the "{receivedError}" error occured.')
            else:
                logger.success('Error', f'The test expects no error an no error occured.')

    def __maybe_check_cluster_error(self, response, logger):
        if self.response and 'clusterError' in self.response:
            expectedError = self.response['clusterError']
            if 'clusterError' in response:
                receivedError = response['clusterError']
                if expectedError == receivedError:
                    logger.success('Error', f'The test expects the "{expectedError}" error which occured successfully.')
                else:
                    logger.error('Error', f'The test expects the "{expectedError}" error but the "{receivedError}" error occured.')
            else:
                logger.error('Error', f'The test expects the "{expectedError}" error but no error occured.')

    def __maybe_check_values(self, response, config, logger):
        if not self.response or not 'values' in self.response:
            return

        if not 'value' in response:
            logger.error('Response', f'The test expects some values but none was received.')
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
                        logger.success('Response', f'The test expectation "{expected_name} == {expected_value}" is true')
                    else:
                        logger.error('Response', f'The test expects a value named "{expected_name}" but it does not exists in the response."')
                    return

                received_value = received_entry[expected_name]
                self.__check_value(expected_name, expected_value, received_value, logger)
            else:
                if not 'value' in expected_entries[0]:
                    continue

                expected_value = expected_entry['value']
                received_value = received_entry
                self.__check_value('value', expected_value, received_value, logger)

    def __check_value(self, name, expected_value, received_value, logger):
        # TODO Supports Array/List. See an exemple of failure in TestArmFailSafe.yaml
        if expected_value == received_value:
            logger.success('Response', f'The test expectation "{name} == {expected_value}" is true')
        else:
            logger.error('Response', f'The test expectation "{name} == {expected_value}" is false')

    def __maybe_check_constraints(self, response, config, logger):
        if not self.response or not 'constraints' in self.response:
            return
        constraints = self.response['constraints']

        allowed_constraint_types = {
            'minValue': {
                'types': [int, float],
                'is_allowed_null': True,
            },
            'maxValue': {
                'types': [int, float],
                'is_allowed_null': True,
            },
            'notValue': {
                'types': '*',
                'is_allowed_null': True,
            },
            'minLength': {
                'types': [str, bytes, list],
                'is_allowed_null': False,
            },
            'maxLength': {
                'types': [str, bytes, list],
                'is_allowed_null': False,
            },
            'isLowerCase': {
                'types': [str],
                'is_allowed_null': False,
            },
            'isUpperCase': {
                'types': [str],
                'is_allowed_null': False,
            },
            'isHexString': {
                'types': [str],
                'is_allowed_null': False,
            },
            'startsWith': {
                'types': [str],
                'is_allowed_null': False,
            },
            'endsWith': {
                'types': [str],
                'is_allowed_null': False,
            },
            'hasMasksSet': {
                'types': [int],
                'is_allowed_null': False,
            },
            'hasMasksClear': {
                'types': [int],
                'is_allowed_null': False,
            },
            'contains': {
                'types': [list],
                'is_allowed_null': False,
            },
            'excludes': {
                'types': [list],
                'is_allowed_null': False,
            },
            'type': {
                'types': '*',
                'is_allowed_null': True,
            }
        }

        def check_is_valid_type(constraint, value):
            allowed = allowed_constraint_types[constraint]

            if value is None and allowed['is_allowed_null']:
                return True

            if allowed['types'] == '*' or type(value) in allowed['types']:
                return True

            logger.error('Constraints', f'The "{constraint}" constraint is used but the value is not of types {allowed["types"]}.')
            return False

        received_value = response['value']

        for constraint, constraint_value in constraints.items():
            if type(constraint_value) is str and constraint_value in config:
                constraint_value = config[constraint_value]

            if not check_is_valid_type(constraint, received_value):
                continue

            success = False
            warning = False
            message = None

            if constraint == 'minValue':
                if received_value is None:
                    continue

                success = received_value >= constraint_value
                message = f'{received_value} >= {constraint_value}'
            elif constraint == 'maxValue':
                if received_value is None:
                    continue

                success = received_value <= constraint_value
                message = f'{received_value} <= {constraint_value}'
            elif constraint == 'notValue':
                success = received_value != constraint_value
                message = f'{received_value} != {constraint_value}'
            elif constraint == 'minLength':
                success = len(received_value) >= constraint_value
                message = f'len({received_value}) >= {constraint_value}'
            elif constraint == 'maxLength':
                success = len(received_value) <= constraint_value
                message = f'len({received_value}) <= {constraint_value}'
            elif constraint == 'startsWith':
                success = received_value.startswith(constraint_value)
                message = f'{received_value} starts with {constraint_value}'
            elif constraint == 'endsWith':
                success = received_value.endswith(constraint_value)
                message = f'{received_value} ends with {constraint_value}'
            elif constraint == 'isLowerCase':
                success = (received_value == received_value.lower()) == constraint_value
                message = f'{received_value} isLowerCase = {constraint_value}'
            elif constraint == 'isUpperCase':
                success = (received_value == received_value.upper()) == constraint_value
                message = f'{received_value} isUpperCase = {constraint_value}'
            elif constraint == 'isHexString':
                success = all(c in string.hexdigits for c in received_value) == constraint_value
                message = f'{received_value} isHexStringCase = {constraint_value}'
            elif constraint == 'contains':
                success = set(constraint_value).issubset(set(received_value))
                message = f'contains {constraint_value}'
            elif constraint == 'excludes':
                success = not bool(set(constraint_value) & set(received_value))
                message = f'excludes {constraint_value}'
            elif constraint == 'hasMasksSet':
                success = True
                for mask in constraint_value:
                    if not received_value & mask:
                        success = False
                message = f'hasMasksSet = {constraint_value}'
            elif constraint == 'hasMasksClear':
                success = True
                for mask in constraint_value:
                    if received_value & mask:
                        success = False
                message = f'hasMasksClear = {constraint_value}'
            elif constraint == 'type':
                if constraint_value == 'boolean' and type(received_value) is bool:
                    success = True
                if constraint_value == 'list' and type(received_value) is list:
                    success = True
                elif constraint_value == 'char_string' and type(received_value) is str:
                    success = True
                elif constraint_value == 'octet_string' and type(received_value) is bytes:
                    success = True
                elif constraint_value == 'vendor_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFF
                elif constraint_value == 'device_type_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'cluster_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'attribute_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'field_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'command_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'event_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'action_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFF
                elif constraint_value == 'transaction_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'node_id' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFFFF
                elif constraint_value == 'bitmap8' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFF
                elif constraint_value == 'bitmap16' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFF
                elif constraint_value == 'bitmap32' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'bitmap64' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFFFF
                elif constraint_value == 'enum8' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFF
                elif constraint_value == 'enum16' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFF
                elif constraint_value == 'Percent' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFF
                elif constraint_value == 'Percent100ths' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFF
                elif constraint_value == 'epoch_us' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFFFF
                elif constraint_value == 'epoch_s' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'utc' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'date' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'tod' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'int8u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFF
                elif constraint_value == 'int16u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFF
                elif constraint_value == 'int24u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFF
                elif constraint_value == 'int32u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFF
                elif constraint_value == 'int40u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFF
                elif constraint_value == 'int48u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFF
                elif constraint_value == 'int56u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFF
                elif constraint_value == 'int64u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFFFF
                elif constraint_value == 'nullable_int8u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFE
                elif constraint_value == 'nullable_int16u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFE
                elif constraint_value == 'nullable_int24u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFE
                elif constraint_value == 'nullable_int32u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFE
                elif constraint_value == 'nullable_int40u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFE
                elif constraint_value == 'nullable_int48u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFE
                elif constraint_value == 'nullable_int56u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFE
                elif constraint_value == 'nullable_int64u' and type(received_value) is int:
                    success = received_value >= 0 and received_value <= 0xFFFFFFFFFFFFFFFE
                elif constraint_value == 'int8s' and type(received_value) is int:
                    success = received_value >= -128 and received_value <= 127
                elif constraint_value == 'int16s' and type(received_value) is int:
                    success = received_value >= -32768 and received_value <= 32767
                elif constraint_value == 'int24s' and type(received_value) is int:
                    success = received_value >= -8388608 and received_value <= 8388607
                elif constraint_value == 'int32s' and type(received_value) is int:
                    success = received_value >= -2147483648 and received_value <= 2147483647
                elif constraint_value == 'int40s' and type(received_value) is int:
                    success = received_value >= -549755813888 and received_value <= 549755813887
                elif constraint_value == 'int48s' and type(received_value) is int:
                    success = received_value >= -140737488355328 and received_value <= 140737488355327
                elif constraint_value == 'int56s' and type(received_value) is int:
                    success = received_value >= -36028797018963968 and received_value <= 36028797018963967
                elif constraint_value == 'int64s' and type(received_value) is int:
                    success = received_value >= -9223372036854775808 and received_value <= 9223372036854775807
                elif constraint_value == 'nullable_int8s' and type(received_value) is int:
                    success = received_value >= -127 and received_value <= 127
                elif constraint_value == 'nullable_int16s' and type(received_value) is int:
                    success = received_value >= -32767 and received_value <= 32767
                elif constraint_value == 'nullable_int24s' and type(received_value) is int:
                    success = received_value >= -8388607 and received_value <= 8388607
                elif constraint_value == 'nullable_int32s' and type(received_value) is int:
                    success = received_value >= -2147483647 and received_value <= 2147483647
                elif constraint_value == 'nullable_int40s' and type(received_value) is int:
                    success = received_value >= -549755813887 and received_value <= 549755813887
                elif constraint_value == 'nullable_int48s' and type(received_value) is int:
                    success = received_value >= -140737488355327 and received_value <= 140737488355327
                elif constraint_value == 'nullable_int56s' and type(received_value) is int:
                    success = received_value >= -36028797018963967 and received_value <= 36028797018963967
                elif constraint_value == 'nullable_int64s' and type(received_value) is int:
                    success = received_value >= -9223372036854775807 and received_value <= 9223372036854775807
                else:
                    warning = True
                message = f'{constraint} == {constraint_value}'
            else:
                message = f'Unknown constraint type: {constraint}'

            if success:
                logger.success('Constraints', f'The test constraint "{message}" succeeds.')
            elif warning:
                logger.warning('Constraints', f'The test constraints "{message}" is ignored.')
            else:
                logger.error('Constraints', f'The test constraint "{message}" failed.')

    def __maybe_save_as(self, response, config, logger):
        if not self.response or not 'values' in self.response:
            return

        if not 'value' in response:
            logger.error('SaveAs', f'SaveAs: The test expects some values but none was received.')
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
                    logger.error('SaveAs', f'The test expects a value named "{expected_name}" but it does not exists in the response."')
                    continue

                received_value = received_entry[expected_name]
                config[saveAs] = received_value
                logger.success('SaveAs', f'The test save the value "{received_value}" as {saveAs}.')
            else:
                received_value = received_entry
                config[saveAs] = received_value
                logger.success('SaveAs', f'The test save the value "{received_value}" as {saveAs}.')

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
    config: None
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

            tests = list(filter(lambda test: test.isEnabled, [YamlTest(
                test, self.config, definitions) for test in _valueOrNone(data, 'tests')]))
            YamlFixes.try_update_yaml_node_id_test_runner_state(tests, self.config)

            self.tests = YamlTests(tests, self.__updatePlaceholder)

    def __updatePlaceholder(self, data):
        data.arguments = self.__encode_values(data.arguments)
        data.response = self.__encode_values(data.response)
        return data

    def __encode_values(self, container):
        if not container:
            return None

        if 'value' in container:
            container['values'] = [{'name': 'value', 'value': container['value']}]
            del container['value']

        if 'values' in container:
            for idx, item in enumerate(container['values']):
                if 'value' in item:
                    container['values'][idx]['value'] = self.__encode_value_with_config(item['value'], self.config)

        if 'saveAs' in container:
            if not 'values' in container:
                container['values'] = [{}]
            container['values'][0]['saveAs'] = container['saveAs']
            del container['saveAs']

        return container

    def __encode_value_with_config(self, value, config):
        if type(value) is list:
            return [self.__encode_value_with_config(entry, config) for entry in value]
        elif type(value) is dict:
            mapped_value = {}
            for key in value:
                mapped_value[key] = self.__encode_value_with_config(value[key], config)
            return mapped_value
        elif type(value) is str:
            # For most tests, a single config variable is used and it can be replaced as in.
            # But some other tests were relying on the fact that the expression was put 'as if' in
            # the generated code and was resolved before beeing sent over the wire. For such expressions
            # (e.g 'myVar + 1') we need to compute it before sending it over the wire.
            tokens = value.split()
            if len(tokens) == 0:
                return value

            has_items_from_config = False
            for idx, token in enumerate(tokens):
                if token in config:
                    config_item = self.config[token]
                    if type(config_item) is dict and 'defaultValue' in config_item:
                        config_item = config_item['defaultValue']
                    tokens[idx] = config_item
                    has_items_from_config = True

            if len(tokens) == 1:
                return tokens[0]

            tokens = [str(token) for token in tokens]
            value = ' '.join(tokens)
            return value if not has_items_from_config else eval(value)
        else:
            return value

    def updateConfig(key, value):
        self.config[key] = value
