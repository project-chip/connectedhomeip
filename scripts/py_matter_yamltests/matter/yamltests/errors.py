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

import yaml

_ERROR_START_TAG = '__error_start__'
_ERROR_END_TAG = '__error_end__'


class TestStepError(Exception):
    """Raise when a step is malformed."""

    def __init__(self, message):
        self.step_index = 0
        self.context = None
        self.message = message

    def __str__(self):
        return self.message

    def update_context(self, context, step_index):
        self.context = yaml.dump(
            context,
            default_flow_style=False,
            sort_keys=False
        )
        self.step_index = step_index

    def tag_key_with_error(self, content, target_key):
        self.__tag_key(content, target_key, _ERROR_START_TAG, _ERROR_END_TAG)

    def untag_keys_with_error(self, content):
        self.__untag_keys(content, _ERROR_START_TAG, _ERROR_END_TAG)

    def __tag_key(self, content, target_key, tag_start, tag_end):
        """This method replaces the key for the dictionary with the tag provided while preserving the order of the dictionary."""
        reversed_dictionary = {}

        # Build a reversed dictionary, tagging the target key.
        for _ in range(len(content)):
            key, value = content.popitem()
            if key == target_key:
                reversed_dictionary[tag_start + key + tag_end] = value
            else:
                reversed_dictionary[key] = value

        # Revert back the dictionary to the original order.
        for _ in range(len(reversed_dictionary)):
            key, value = reversed_dictionary.popitem()
            content[key] = value

    def __untag_keys(self, content, tag_start, tag_end):
        """This method replaces the tagged key for the dictionary with the original key while preserving the order of the dictionary."""
        reversed_dictionary = {}

        # Build a reversed dictionary, untagging the tagged key.
        for _ in range(len(content)):
            key, value = content.popitem()
            if key.startswith(tag_start) and key.endswith(tag_end):
                reversed_dictionary[key.replace(
                    tag_start, '').replace(tag_end, '')] = value
            else:
                reversed_dictionary[key] = value

        # Revert back the dictionary to the original order.
        for _ in range(len(reversed_dictionary)):
            key, value = reversed_dictionary.popitem()
            content[key] = value


class TestStepKeyError(TestStepError):
    """Raise when a key is unknown."""

    def __init__(self, content, key):
        message = f'Unknown key "{key}"'
        super().__init__(message)

        self.tag_key_with_error(content, key)


class TestStepValueNameError(TestStepError):
    """Raise when a value name is unknown."""

    def __init__(self, content, key, candidate_keys):
        message = f'Unknown key: "{key}". Candidates are: "{candidate_keys}"'
        for candidate_key in candidate_keys:
            if candidate_key.lower() == key.lower():
                message = f'Unknown key: "{key}". Did you mean "{candidate_key}" ?'
                break
        super().__init__(message)

        self.tag_key_with_error(content, 'name')


class TestStepInvalidTypeError(TestStepError):
    """Raise when the value for a given key is not of the expected type."""

    def __init__(self, content, key, expected_type):
        if isinstance(expected_type, tuple):
            expected_name = ''
            for _type in expected_type:
                expected_name += _type.__name__ + ','
            expected_name = expected_name[:-1]
        else:
            expected_name = expected_type.__name__
        received_name = type(content[key]).__name__
        message = f'Unexpected type. Expecting "{expected_name}", got "{received_name}"'
        super().__init__(message)

        self.tag_key_with_error(content, key)


class TestStepGroupResponseError(TestStepError):
    """Raise when a test step targeting a group of nodes expects a response."""

    def __init__(self, content):
        message = 'Group command should not expect a response'
        super().__init__(message)

        self.tag_key_with_error(content, 'groupId')
        self.tag_key_with_error(content, 'response')


class TestStepGroupEndPointError(TestStepError):
    """Raise when a test step targeting a group of nodes targets an endpoint."""

    def __init__(self, content):
        message = 'Group command should not target an endpoint'
        super().__init__(message)

        self.tag_key_with_error(content, 'groupId')
        self.tag_key_with_error(content, 'endpoint')


class TestStepVerificationStandaloneError(TestStepError):
    """Raise when a test step with a verification key is enabled and not interactive."""

    def __init__(self, content):
        message = 'Step using "verification" key should either set "disabled: true" or "PICS: PICS_USER_PROMPT"'
        super().__init__(message)

        self.tag_key_with_error(content, 'verification')


class TestStepNodeIdAndGroupIdError(TestStepError):
    """Raise when a test step contains both "nodeId" and "groupId" keys."""

    def __init__(self, content):
        message = '"nodeId" and "groupId" are mutually exclusive'
        super().__init__(message)

        self.tag_key_with_error(content, 'nodeId')
        self.tag_key_with_error(content, 'groupId')


class TestStepValueAndValuesError(TestStepError):
    """Raise when a test step response contains both "value" and "values" keys."""

    def __init__(self, content):
        message = '"value" and "values" are mutually exclusive'
        super().__init__(message)

        self.tag_key_with_error(content, 'value')
        self.tag_key_with_error(content, 'values')


class TestStepWaitResponseError(TestStepError):
    """Raise when a test step is waiting for a particular event (e.g an attribute read) using the
       wait keyword but also specify a response.
    """

    def __init__(self, content):
        message = 'The "wait" key can not be used in conjuction with the "response" key'
        super().__init__(message)

        self.tag_key_with_error(content, 'wait')
        self.tag_key_with_error(content, 'response')


class TestStepResponseVariableError(TestStepError):
    """Raise when a test step response use a variable but this variable does not exist in the config section.
    """

    def __init__(self, content):
        message = 'The variable does not exist in the config section.'
        super().__init__(message)

        self.tag_key_with_error(content, 'response')


class TestStepArgumentsValueError(TestStepError):
    """Raise when a test step arguments use the 'value' keyword but the command is not trying to write to an attribute"""

    def __init__(self, content):
        message = 'The "value" key can not be used in conjuction with a command that is not "writeAttribute"'
        super().__init__(message)

        self.tag_key_with_error(content, 'command')
        arguments = content.get('arguments')
        self.tag_key_with_error(arguments, 'value')


class TestStepSaveAsNameError(TestStepError):
    """Raise when a test step response save an attribute response with the same name than the attribute itself"""

    def __init__(self, content):
        message = 'The "saveAs" key can not be the same than the "attribute" key'
        super().__init__(message)

        self.tag_key_with_error(content, 'attribute')
        response = content.get('response')
        self.tag_key_with_error(response, 'saveAs')


class TestStepEnumError(TestStepError):
    """
    Raise when an enum value or an enum name is not found in the definitions.

    Parameters:
        - enum_name_or_value (str|int): The name (str) or value (int) of the enumeration in the step.
          If a string is provided, it is considered the name of the enumeration; if an integer is provided, it is considered the value of the enumeration.
        - enum_candidates (dict): A dictionary mapping enumeration names (as strings) to their corresponding values
          (as integers). This dictionary represents all known values of the enumeration.
    """

    def __init__(self, enum_name_or_value, enum_candidates: dict):
        if type(enum_name_or_value) is str:
            message = f'Unknown enum name: "{enum_name_or_value}". The possible values are: "{enum_candidates}"'

            for enum_name in enum_candidates:
                if enum_name.lower() == enum_name_or_value.lower():
                    message = f'Unknown enum name: "{enum_name_or_value}". Did you mean "{enum_name}" ?'
                    break

        else:
            message = f'Unknown enum value: "{enum_name_or_value}". The possible values are: "{enum_candidates}"'

        super().__init__(message)


class TestStepEnumSpecifierNotUnknownError(TestStepError):
    """Raise when an enum value declared as unknown is in fact a known enum value from the definitions."""

    def __init__(self, specified_value, enum_name):
        message = f'The value "{specified_value}" is not unknown. It is the value of "{enum_name}"'
        super().__init__(message)


class TestStepEnumSpecifierWrongError(TestStepError):
    """Raise when an enum value is specified for a given enum name but it does not match the enum value from the definitions."""

    def __init__(self, specified_value, enum_name, enum_value):
        message = f'The value "{specified_value}" is not the value of "{enum_name}({enum_value})"'
        super().__init__(message)
