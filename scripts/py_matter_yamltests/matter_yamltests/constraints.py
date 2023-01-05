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

import string
from abc import ABC, abstractmethod


class ConstraintParseError(Exception):
    def __init__(self, message):
        super().__init__(message)


class ConstraintValidationError(Exception):
    def __init__(self, message):
        super().__init__(message)


class BaseConstraint(ABC):
    '''Constrain Interface'''

    def __init__(self, types: list, is_null_allowed: bool = False):
        '''An empty type list provided that indicates any type is accepted'''
        self._types = types
        self._is_null_allowed = is_null_allowed

    def is_met(self, value):
        if value is None:
            return self._is_null_allowed

        response_type = type(value)
        if self._types and response_type not in self._types:
            return False

        return self.check_response(value)

    @abstractmethod
    def check_response(self, value) -> bool:
        pass


class _ConstraintHasValue(BaseConstraint):
    def __init__(self, has_value):
        super().__init__(types=[])
        self._has_value = has_value

    def check_response(self, value) -> bool:
        raise ConstraintValidationError('HasValue constraint currently not implemented')


class _ConstraintType(BaseConstraint):
    def __init__(self, type):
        super().__init__(types=[], is_null_allowed=True)
        self._type = type

    def check_response(self, value) -> bool:
        success = False
        if self._type == 'boolean' and type(value) is bool:
            success = True
        elif self._type == 'list' and type(value) is list:
            success = True
        elif self._type == 'char_string' and type(value) is str:
            success = True
        elif self._type == 'octet_string' and type(value) is bytes:
            success = True
        elif self._type == 'vendor_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'device_type_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'cluster_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'attribute_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'field_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'command_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'event_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'action_id' and type(value) is int:
            success = value >= 0 and value <= 0xFF
        elif self._type == 'transaction_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'node_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFFFF
        elif self._type == 'bitmap8' and type(value) is int:
            success = value >= 0 and value <= 0xFF
        elif self._type == 'bitmap16' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'bitmap32' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'bitmap64' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFFFF
        elif self._type == 'enum8' and type(value) is int:
            success = value >= 0 and value <= 0xFF
        elif self._type == 'enum16' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'Percent' and type(value) is int:
            success = value >= 0 and value <= 0xFF
        elif self._type == 'Percent100ths' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'epoch_us' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFFFF
        elif self._type == 'epoch_s' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'utc' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'date' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'tod' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'int8u' and type(value) is int:
            success = value >= 0 and value <= 0xFF
        elif self._type == 'int16u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'int24u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFF
        elif self._type == 'int32u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'int40u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFF
        elif self._type == 'int48u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFF
        elif self._type == 'int56u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFF
        elif self._type == 'int64u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFFFF
        elif self._type == 'nullable_int8u' and type(value) is int:
            success = value >= 0 and value <= 0xFE
        elif self._type == 'nullable_int16u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFE
        elif self._type == 'nullable_int24u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFE
        elif self._type == 'nullable_int32u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFE
        elif self._type == 'nullable_int40u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFE
        elif self._type == 'nullable_int48u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFE
        elif self._type == 'nullable_int56u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFE
        elif self._type == 'nullable_int64u' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFFFE
        elif self._type == 'int8s' and type(value) is int:
            success = value >= -128 and value <= 127
        elif self._type == 'int16s' and type(value) is int:
            success = value >= -32768 and value <= 32767
        elif self._type == 'int24s' and type(value) is int:
            success = value >= -8388608 and value <= 8388607
        elif self._type == 'int32s' and type(value) is int:
            success = value >= -2147483648 and value <= 2147483647
        elif self._type == 'int40s' and type(value) is int:
            success = value >= -549755813888 and value <= 549755813887
        elif self._type == 'int48s' and type(value) is int:
            success = value >= -140737488355328 and value <= 140737488355327
        elif self._type == 'int56s' and type(value) is int:
            success = value >= -36028797018963968 and value <= 36028797018963967
        elif self._type == 'int64s' and type(value) is int:
            success = value >= -9223372036854775808 and value <= 9223372036854775807
        elif self._type == 'nullable_int8s' and type(value) is int:
            success = value >= -127 and value <= 127
        elif self._type == 'nullable_int16s' and type(value) is int:
            success = value >= -32767 and value <= 32767
        elif self._type == 'nullable_int24s' and type(value) is int:
            success = value >= -8388607 and value <= 8388607
        elif self._type == 'nullable_int32s' and type(value) is int:
            success = value >= -2147483647 and value <= 2147483647
        elif self._type == 'nullable_int40s' and type(value) is int:
            success = value >= -549755813887 and value <= 549755813887
        elif self._type == 'nullable_int48s' and type(value) is int:
            success = value >= -140737488355327 and value <= 140737488355327
        elif self._type == 'nullable_int56s' and type(value) is int:
            success = value >= -36028797018963967 and value <= 36028797018963967
        elif self._type == 'nullable_int64s' and type(value) is int:
            success = value >= -9223372036854775807 and value <= 9223372036854775807
        return success


class _ConstraintMinLength(BaseConstraint):
    def __init__(self, min_length):
        super().__init__(types=[str, bytes, list])
        self._min_length = min_length

    def check_response(self, value) -> bool:
        return len(value) >= self._min_length


class _ConstraintMaxLength(BaseConstraint):
    def __init__(self, max_length):
        super().__init__(types=[str, bytes, list])
        self._max_length = max_length

    def check_response(self, value) -> bool:
        return len(value) <= self._max_length


class _ConstraintIsHexString(BaseConstraint):
    def __init__(self, is_hex_string: bool):
        super().__init__(types=[str])
        self._is_hex_string = is_hex_string

    def check_response(self, value) -> bool:
        return all(c in string.hexdigits for c in value) == self._is_hex_string


class _ConstraintStartsWith(BaseConstraint):
    def __init__(self, starts_with):
        super().__init__(types=[str])
        self._starts_with = starts_with

    def check_response(self, value) -> bool:
        return value.startswith(self._starts_with)


class _ConstraintEndsWith(BaseConstraint):
    def __init__(self, ends_with):
        super().__init__(types=[str])
        self._ends_with = ends_with

    def check_response(self, value) -> bool:
        return value.endswith(self._ends_with)


class _ConstraintIsUpperCase(BaseConstraint):
    def __init__(self, is_upper_case):
        super().__init__(types=[str])
        self._is_upper_case = is_upper_case

    def check_response(self, value) -> bool:
        return value.isupper() == self._is_upper_case


class _ConstraintIsLowerCase(BaseConstraint):
    def __init__(self, is_lower_case):
        super().__init__(types=[str])
        self._is_lower_case = is_lower_case

    def check_response(self, value) -> bool:
        return value.islower() == self._is_lower_case


class _ConstraintMinValue(BaseConstraint):
    def __init__(self, min_value):
        super().__init__(types=[int, float], is_null_allowed=True)
        self._min_value = min_value

    def check_response(self, value) -> bool:
        return value >= self._min_value


class _ConstraintMaxValue(BaseConstraint):
    def __init__(self, max_value):
        super().__init__(types=[int, float], is_null_allowed=True)
        self._max_value = max_value

    def check_response(self, value) -> bool:
        return value <= self._max_value


class _ConstraintContains(BaseConstraint):
    def __init__(self, contains):
        super().__init__(types=[list])
        self._contains = contains

    def check_response(self, value) -> bool:
        return set(self._contains).issubset(value)


class _ConstraintExcludes(BaseConstraint):
    def __init__(self, excludes):
        super().__init__(types=[list])
        self._excludes = excludes

    def check_response(self, value) -> bool:
        return set(self._excludes).isdisjoint(value)


class _ConstraintHasMaskSet(BaseConstraint):
    def __init__(self, has_masks_set):
        super().__init__(types=[int])
        self._has_masks_set = has_masks_set

    def check_response(self, value) -> bool:
        return all([(value & mask) == mask for mask in self._has_masks_set])


class _ConstraintHasMaskClear(BaseConstraint):
    def __init__(self, has_masks_clear):
        super().__init__(types=[int])
        self._has_masks_clear = has_masks_clear

    def check_response(self, value) -> bool:
        return all([(value & mask) == 0 for mask in self._has_masks_clear])


class _ConstraintNotValue(BaseConstraint):
    def __init__(self, not_value):
        super().__init__(types=[], is_null_allowed=True)
        self._not_value = not_value

    def check_response(self, value) -> bool:
        return value != self._not_value


def get_constraints(constraints: dict) -> list[BaseConstraint]:
    _constraints = []

    for constraint, constraint_value in constraints.items():
        if 'hasValue' == constraint:
            _constraints.append(_ConstraintHasValue(constraint_value))
        elif 'type' == constraint:
            _constraints.append(_ConstraintType(constraint_value))
        elif 'minLength' == constraint:
            _constraints.append(_ConstraintMinLength(constraint_value))
        elif 'maxLength' == constraint:
            _constraints.append(_ConstraintMaxLength(constraint_value))
        elif 'isHexString' == constraint:
            _constraints.append(_ConstraintIsHexString(constraint_value))
        elif 'startsWith' == constraint:
            _constraints.append(_ConstraintStartsWith(constraint_value))
        elif 'endsWith' == constraint:
            _constraints.append(_ConstraintEndsWith(constraint_value))
        elif 'isUpperCase' == constraint:
            _constraints.append(_ConstraintIsUpperCase(constraint_value))
        elif 'isLowerCase' == constraint:
            _constraints.append(_ConstraintIsLowerCase(constraint_value))
        elif 'minValue' == constraint:
            _constraints.append(_ConstraintMinValue(constraint_value))
        elif 'maxValue' == constraint:
            _constraints.append(_ConstraintMaxValue(constraint_value))
        elif 'contains' == constraint:
            _constraints.append(_ConstraintContains(constraint_value))
        elif 'excludes' == constraint:
            _constraints.append(_ConstraintExcludes(constraint_value))
        elif 'hasMasksSet' == constraint:
            _constraints.append(_ConstraintHasMaskSet(constraint_value))
        elif 'hasMasksClear' == constraint:
            _constraints.append(_ConstraintHasMaskClear(constraint_value))
        elif 'notValue' == constraint:
            _constraints.append(_ConstraintNotValue(constraint_value))
        else:
            raise ConstraintParseError(f'Unknown constraint type:{constraint}')

    return _constraints
