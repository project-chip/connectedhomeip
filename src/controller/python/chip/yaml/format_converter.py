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

import typing
from chip.clusters.Types import Nullable, NullValue
from chip.tlv import uint, float32
import enum
from chip.yaml.errors import ValidationError
import binascii


def substitute_in_config_variables(field_value, config_values: dict):
    ''' Substitutes values that are config variables.

    YAML values can contain a string of a configuration variable name. In these instances we
    substitute the configuration variable name with the actual value.

    For examples see unittest src/controller/python/test/unit_tests/test_yaml_format_converter.py

    # TODO This should also substitue any saveAs values as well as perform any required
    # evaluations.

    Args:
        'field_value': Value as extracted from YAML.
        'config_values': Dictionary of global configuration variables.
    Returns:
        Value with all global configuration variables substituted with the real value.
    '''
    if isinstance(field_value, dict):
        return {key: substitute_in_config_variables(
            field_value[key], config_values) for key in field_value}
    if isinstance(field_value, list):
        return [substitute_in_config_variables(item, config_values) for item in field_value]
    if isinstance(field_value, str) and field_value in config_values:
        config_value = config_values[field_value]
        if isinstance(config_value, dict) and 'defaultValue' in config_value:
            # TODO currently we don't validate that if config_value['type'] is provided
            # that the type does in fact match our expectation.
            return config_value['defaultValue']
        return config_values[field_value]

    return field_value


def convert_yaml_octet_string_to_bytes(s: str) -> bytes:
    '''Convert YAML octet string body to bytes.

    Included handling any c-style hex escapes (e.g. \x5a) and 'hex:' prefix.
    '''
    # Step 1: handle explicit "hex:" prefix
    if s.startswith('hex:'):
        return binascii.unhexlify(s[4:])

    # Step 2: convert non-hex-prefixed to bytes
    # TODO(#23669): This does not properly support utf8 octet strings. We mimic
    # javascript codegen behavior. Behavior of javascript is:
    #   * Octet string character >= u+0200 errors out.
    #   * Any character greater than 0xFF has the upper bytes chopped off.
    as_bytes = [ord(c) for c in s]

    if any([value > 0x200 for value in as_bytes]):
        raise ValueError('Unsupported char in octet string %r' % as_bytes)
    accumulated_hex = ''.join([f"{(v & 0xFF):02x}" for v in as_bytes])
    return binascii.unhexlify(accumulated_hex)


def convert_name_value_pair_to_dict(arg_values):
    ''' Fix yaml command arguments.

    For some reason, instead of treating the entire data payload of a
    command as a singular struct, the top-level args are specified as 'name'
    and 'value' pairs, while the payload of each argument is itself
    correctly encapsulated. This fixes up this oddity to create a new
    key/value pair with the key being the value of the 'name' field, and
    the value being 'value' field.
    '''
    ret_value = {}

    for item in arg_values:
        ret_value[item['name']] = item['value']

    return ret_value


def convert_yaml_type(field_value, field_type, inline_cast_dict_to_struct):
    ''' Converts yaml value to provided pythonic type.

    The YAML representation when converted to a dictionary does not line up to
    the python type data model for the various command/attribute/event object
    types. This function converts 'field_value' to the appropriate provided
    'field_type'.

    Args:
        'field_value': Value as extracted from yaml
        'field_type': Pythonic command/attribute/event object type that we
            are converting value to.
        'inline_cast_dict_to_struct': If true, for any dictionary 'field_value'
            types provided we will do a convertion to the corresponding data
            model class in `field_type` by doing field_type.FromDict(...).
    '''
    origin = typing.get_origin(field_type)

    if field_value is None:
        field_value = NullValue

    if (origin == typing.Union or origin == typing.Optional or origin == Nullable):
        underlying_field_type = None

        if field_value is NullValue:
            for t in typing.get_args(field_type):
                if t == Nullable:
                    return field_value

        for t in typing.get_args(field_type):
            # Comparison below explicitly not using 'isinstance' as that doesn't do what we want.
            if t != Nullable and t != type(None):
                underlying_field_type = t
                break

        if (underlying_field_type is None):
            raise ValueError(f"Can't find the underling type for {field_type}")

        field_type = underlying_field_type

    # Dictionary represents a data model struct.
    if (type(field_value) is dict):
        return_field_value = {}
        field_descriptors = field_type.descriptor
        for item in field_value:
            try:
                # We search for a matching item in the list of field descriptors
                # for this struct and ensure we can find a field with a matching
                # label.
                field_descriptor = next(
                    x for x in field_descriptors.Fields if x.Label.lower() ==
                    item.lower())
            except StopIteration as exc:
                raise ValidationError(
                    f'Did not find field "{item}" in {str(field_type)}') from None

            return_field_value[field_descriptor.Label] = convert_yaml_type(
                field_value[item], field_descriptor.Type, inline_cast_dict_to_struct)
        if inline_cast_dict_to_struct:
            return field_type.FromDict(return_field_value)
        return return_field_value
    elif(type(field_value) is float):
        return float32(field_value)
    # list represents a data model list
    elif(type(field_value) is list):
        list_element_type = typing.get_args(field_type)[0]

        # The field type passed in is the type of the list element and not list[T].
        for idx, item in enumerate(field_value):
            field_value[idx] = convert_yaml_type(item, list_element_type,
                                                 inline_cast_dict_to_struct)
        return field_value
    # YAML conversion treats all numbers as ints. Convert to a uint type if the schema
    # type indicates so.
    elif (field_type == uint):
        # Longer number are stored as strings. Need to make this conversion first.
        value = int(field_value)
        return field_type(value)
    # YAML treats enums as ints. Convert to the typed enum class.
    elif (issubclass(field_type, enum.Enum)):
        return field_type(field_value)
    # YAML treats bytes as strings. Convert to a byte string.
    elif (field_type == bytes and type(field_value) != bytes):
        return convert_yaml_octet_string_to_bytes(field_value)
    # By default, just return the field_value casted to field_type.
    else:
        return field_type(field_value)


def parse_and_convert_yaml_value(field_value, field_type, config_values: dict,
                                 inline_cast_dict_to_struct: bool = False):
    ''' Parse and converts YAML type

    Parsing the YAML value means performing required substitutions and evaluations. Parsing is
    then followed by converting from the YAML type done using yaml.safe_load() to the type used in
    the various command/attribute/event object data model types.

    Args:
        'field_value': Value as extracted from yaml to be parsed
        'field_type': Pythonic command/attribute/event object type that we
            are converting value to.
        'config_values': Dictionary of global configuration variables.
        'inline_cast_dict_to_struct': If true, for any dictionary 'field_value'
            types provided we will do an inline convertion to the corresponding
            struct in `field_type` by doing field_type.FromDict(...).
    '''
    field_value_with_config_variables = substitute_in_config_variables(field_value, config_values)
    return convert_yaml_type(field_value_with_config_variables, field_type,
                             inline_cast_dict_to_struct)
