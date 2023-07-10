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
from dataclasses import dataclass

from chip.clusters.enum import MatterIntEnum
from chip.clusters.Types import Nullable, NullValue
from chip.tlv import float32, uint
from chip.yaml.errors import ValidationError
from matter_idl import matter_idl_types


@dataclass
class _TargetTypeInfo:
    field: typing.Union[list[matter_idl_types.Field], matter_idl_types.Field]
    is_fabric_scoped: bool


def _case_insensitive_getattr(object, attr_name, default):
    for attr in dir(object):
        if attr.lower() == attr_name.lower():
            return getattr(object, attr)
    return default


def _get_target_type_info(test_spec_definition, cluster_name, target_name) -> _TargetTypeInfo:
    element = test_spec_definition.get_type_by_name(cluster_name, target_name)
    if hasattr(element, 'fields'):
        is_fabric_scoped = test_spec_definition.is_fabric_scoped(element)
        return _TargetTypeInfo(element.fields, is_fabric_scoped)
    return _TargetTypeInfo(None, False)


def from_data_model_to_test_definition(test_spec_definition, cluster_name, response_definition,
                                       response_value, is_fabric_scoped=False):
    '''Converts value from data model to definitions provided in test_spec_definition.

    Args:
        'test_spec_definition': The spec cluster definition used by the test parser.
        'cluster_name': Used when we need to look up information in 'test_spec_definition'.
        'response_definition': Type we are converting 'response_value' to. This will be one of
            two types: list[idl.matter_idl_types.Field] or idl.matter_idl_types.Field
        'response_value': Response value that we want to convert to
    '''
    if response_value is None:
        return response_value

    # We first check to see if response_definition is list[idl.matter_idl_types.Field]. When we
    # have list[idl.matter_idl_types.Field] that means we have a structure with multiple fields
    # that need to be worked through recursively to properly convert the value to the right type.
    if isinstance(response_definition, list):
        rv = {}
        # is_fabric_scoped will only be relevant for struct types, hence why it is only checked
        # here.
        if is_fabric_scoped:
            rv['FabricIndex'] = _case_insensitive_getattr(response_value, 'fabricIndex', None)
        for item in response_definition:
            value = _case_insensitive_getattr(response_value, item.name, None)
            if item.is_optional and value is None:
                continue
            rv[item.name] = from_data_model_to_test_definition(test_spec_definition, cluster_name,
                                                               item, value)
        return rv

    # We convert uint to python int because constraints first check that it is an expected type.
    response_value_type = type(response_value)
    if response_value_type == uint:
        return int(response_value)

    if response_definition is None:
        return response_value

    if response_value is NullValue:
        return None

    # For single float values types there seems to be a floating precision issue. By using '%g'
    # it naturally give 6 most significat digits for us which is the amount of prcision we are
    # looking for to give parity results to what chip-tool was getting (For TestCluster.yaml it
    # give value back of `0.100000`.
    if response_value_type == float32 and response_definition.data_type.name.lower() == 'single':
        return float('%g' % response_value)

    target_type_info = _get_target_type_info(test_spec_definition, cluster_name,
                                             response_definition.data_type.name)

    response_sub_definition = target_type_info.field
    is_sub_definition_fabric_scoped = target_type_info.is_fabric_scoped

    # Check below is to see if the field itself is an array, for example array of ints.
    if response_definition.is_list:
        return [
            from_data_model_to_test_definition(test_spec_definition, cluster_name,
                                               response_sub_definition, item,
                                               is_sub_definition_fabric_scoped) for item in response_value
        ]

    return from_data_model_to_test_definition(test_spec_definition, cluster_name,
                                              response_sub_definition, response_value,
                                              is_sub_definition_fabric_scoped)


def convert_list_of_name_value_pair_to_dict(arg_values):
    '''Converts list of dict with items with keys 'name' and 'value' into single dict.

    The test step contains a list of arguments that have multiple properties other than
    'name' and 'value'. For the purposes of executing a test all these other attributes are not
    important. We only want a simple dictionary of a new key/value where with the key being the
    value of the 'name' field, and the value being 'value' field.
    '''
    ret_value = {}

    for item in arg_values:
        ret_value[item['name']] = item['value']

    return ret_value


def convert_to_data_model_type(field_value, field_type):
    '''Converts value to provided data model pythonic object type.

    The values provided by parser does not line up to the python data model for the various
    command/attribute/event object types. This function converts 'field_value' to the provided
    'field_type'.

    Args:
        'field_value': Value as extracted by YAML parser.
        'field_type': Pythonic command/attribute/event object type that we are converting value to.
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
            # type_none is simple hack for Flake8 E721
            type_none = type(None)
            if t != Nullable and t != type_none:
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
            except StopIteration:
                raise ValidationError(
                    f'Did not find field "{item}" in {str(field_type)}') from None

            return_field_value[field_descriptor.Label] = convert_to_data_model_type(
                field_value[item], field_descriptor.Type)
        return return_field_value
    elif (type(field_value) is float):
        return float32(field_value)
    # list represents a data model list
    elif (type(field_value) is list):
        list_element_type = typing.get_args(field_type)[0]

        # The field type passed in is the type of the list element and not list[T].
        for idx, item in enumerate(field_value):
            field_value[idx] = convert_to_data_model_type(item, list_element_type)
        return field_value
    # YAML conversion treats all numbers as ints. Convert to a uint type if the schema
    # type indicates so.
    elif (field_type == uint):
        # Longer number are stored as strings. Need to make this conversion first.
        value = int(field_value)
        return field_type(value)
    # YAML treats enums as ints. Convert to the typed enum class.
    elif (issubclass(field_type, MatterIntEnum)):
        return field_type.extend_enum_if_value_doesnt_exist(field_value)
    # By default, just return the field_value casted to field_type.
    else:
        return field_type(field_value)
