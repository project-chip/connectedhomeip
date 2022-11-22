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


def convert_yaml_octet_string_to_bytes(s: str) -> bytes:
    """Convert YAML octet string body to bytes, handling any c-style hex escapes (e.g. \x5a) and hex: prefix"""
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


def convert_yaml_type(field_value, field_type, use_from_dict=False):
    ''' Converts yaml value to expected type.

    The YAML representation when converted to a Python dictionary does not
    quite line up in terms of type (see each of the specific if branches
    below for the rationale for the necessary fix-ups). This function does
    a fix-up given a field value (as present in the YAML) and its matching
    cluster object type and returns it.
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
                field_value[item], field_descriptor.Type, use_from_dict)
        if use_from_dict:
            return field_type.FromDict(return_field_value)
        return return_field_value
    elif(type(field_value) is float):
        return float32(field_value)
    # list represents a data model list
    elif(type(field_value) is list):
        list_element_type = typing.get_args(field_type)[0]

        # The field type passed in is the type of the list element and not list[T].
        for idx, item in enumerate(field_value):
            field_value[idx] = convert_yaml_type(item, list_element_type, use_from_dict)
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
