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

from abc import ABC, abstractmethod
import chip.yaml.format_converter as Converter
from .variable_storage import VariableStorage


class ConstraintValidationError(Exception):
    def __init__(self, message):
        super().__init__(message)


class BaseConstraint(ABC):
    '''Constrain Interface'''

    @abstractmethod
    def is_met(self, response) -> bool:
        pass


class _LoadableConstraint(BaseConstraint):
    '''Constraints where value might be stored in VariableStorage needing runtime load.'''

    def __init__(self, value, field_type, variable_storage: VariableStorage, config_values: dict):
        self._variable_storage = variable_storage
        # When not none _indirect_value_key is binding a name to the constraint value, and the
        # actual value can only be looked-up dynamically, which is why this is a key name.
        self._indirect_value_key = None
        self._value = None

        if value is None:
            # Default values set above is all we need here.
            return

        if isinstance(value, str) and self._variable_storage.is_key_saved(value):
            self._indirect_value_key = value
        else:
            self._value = Converter.parse_and_convert_yaml_value(
                value, field_type, config_values)

    def get_value(self):
        '''Gets the current value of the constraint.

        This method accounts for getting the runtime saved value from DUT previous responses.
        '''
        if self._indirect_value_key:
            return self._variable_storage.load(self._indirect_value_key)
        return self._value


class _ConstraintHasValue(BaseConstraint):
    def __init__(self, has_value):
        self._has_value = has_value

    def is_met(self, response) -> bool:
        raise ConstraintValidationError('HasValue constraint currently not implemented')


class _ConstraintType(BaseConstraint):
    def __init__(self, type):
        self._type = type

    def is_met(self, response) -> bool:
        raise ConstraintValidationError('Type constraint currently not implemented')


class _ConstraintStartsWith(BaseConstraint):
    def __init__(self, starts_with):
        self._starts_with = starts_with

    def is_met(self, response) -> bool:
        return response.startswith(self._starts_with)


class _ConstraintEndsWith(BaseConstraint):
    def __init__(self, ends_with):
        self._ends_with = ends_with

    def is_met(self, response) -> bool:
        return response.endswith(self._ends_with)


class _ConstraintIsUpperCase(BaseConstraint):
    def __init__(self, is_upper_case):
        self._is_upper_case = is_upper_case

    def is_met(self, response) -> bool:
        return response.isupper() == self._is_upper_case


class _ConstraintIsLowerCase(BaseConstraint):
    def __init__(self, is_lower_case):
        self._is_lower_case = is_lower_case

    def is_met(self, response) -> bool:
        return response.islower() == self._is_lower_case


class _ConstraintMinValue(_LoadableConstraint):
    def __init__(self, min_value, field_type, variable_storage: VariableStorage,
                 config_values: dict):
        super().__init__(min_value, field_type, variable_storage, config_values)

    def is_met(self, response) -> bool:
        min_value = self.get_value()
        return response >= min_value


class _ConstraintMaxValue(_LoadableConstraint):
    def __init__(self, max_value, field_type, variable_storage: VariableStorage,
                 config_values: dict):
        super().__init__(max_value, field_type, variable_storage, config_values)

    def is_met(self, response) -> bool:
        max_value = self.get_value()
        return response <= max_value


class _ConstraintContains(BaseConstraint):
    def __init__(self, contains):
        self._contains = contains

    def is_met(self, response) -> bool:
        return set(self._contains).issubset(response)


class _ConstraintExcludes(BaseConstraint):
    def __init__(self, excludes):
        self._excludes = excludes

    def is_met(self, response) -> bool:
        return set(self._excludes).isdisjoint(response)


class _ConstraintHasMaskSet(BaseConstraint):
    def __init__(self, has_masks_set):
        self._has_masks_set = has_masks_set

    def is_met(self, response) -> bool:
        return all([(response & mask) == mask for mask in self._has_masks_set])


class _ConstraintHasMaskClear(BaseConstraint):
    def __init__(self, has_masks_clear):
        self._has_masks_clear = has_masks_clear

    def is_met(self, response) -> bool:
        return all([(response & mask) == 0 for mask in self._has_masks_clear])


class _ConstraintNotValue(_LoadableConstraint):
    def __init__(self, not_value, field_type, variable_storage: VariableStorage,
                 config_values: dict):
        super().__init__(not_value, field_type, variable_storage, config_values)

    def is_met(self, response) -> bool:
        not_value = self.get_value()
        return response != not_value


def get_constraints(constraints, field_type, variable_storage: VariableStorage,
                    config_values: dict) -> list[BaseConstraint]:
    _constraints = []
    if 'hasValue' in constraints:
        _constraints.append(_ConstraintHasValue(constraints.get('hasValue')))

    if 'type' in constraints:
        _constraints.append(_ConstraintType(constraints.get('type')))

    if 'startsWith' in constraints:
        _constraints.append(_ConstraintStartsWith(constraints.get('startsWith')))

    if 'endsWith' in constraints:
        _constraints.append(_ConstraintEndsWith(constraints.get('endsWith')))

    if 'isUpperCase' in constraints:
        _constraints.append(_ConstraintIsUpperCase(constraints.get('isUpperCase')))

    if 'isLowerCase' in constraints:
        _constraints.append(_ConstraintIsLowerCase(constraints.get('isLowerCase')))

    if 'minValue' in constraints:
        _constraints.append(_ConstraintMinValue(
            constraints.get('minValue'), field_type, variable_storage, config_values))

    if 'maxValue' in constraints:
        _constraints.append(_ConstraintMaxValue(
            constraints.get('maxValue'), field_type, variable_storage, config_values))

    if 'contains' in constraints:
        _constraints.append(_ConstraintContains(constraints.get('contains')))

    if 'excludes' in constraints:
        _constraints.append(_ConstraintExcludes(constraints.get('excludes')))

    if 'hasMasksSet' in constraints:
        _constraints.append(_ConstraintHasMaskSet(constraints.get('hasMasksSet')))

    if 'hasMasksClear' in constraints:
        _constraints.append(_ConstraintHasMaskClear(constraints.get('hasMasksClear')))

    if 'notValue' in constraints:
        _constraints.append(_ConstraintNotValue(
            constraints.get('notValue'), field_type, variable_storage, config_values))

    return _constraints
