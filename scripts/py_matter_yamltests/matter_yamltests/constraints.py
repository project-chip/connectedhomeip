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

import math
import re
import string
from abc import ABC, abstractmethod
from typing import List

from .errors import TestStepError


class ConstraintParseError(Exception):
    def __init__(self, message):
        super().__init__(message)


class ConstraintCheckError(TestStepError):
    def __init__(self, context, key, reason):
        super().__init__(reason)
        self.untag_keys_with_error(context)
        self.tag_key_with_error(context, key)


class ConstraintTypeError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'type', reason)


class ConstraintContainsError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'contains', reason)


class ConstraintExcludesError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'excludes', reason)


class ConstraintHasMaskClearError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'hasMasksClear', reason)


class ConstraintHasMaskSetError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'hasMasksSet', reason)


class ConstraintHasValueError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'hasValue', reason)


class ConstraintMinLengthError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'minLength', reason)


class ConstraintMaxLengthError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'maxLength', reason)


class ConstraintIsHexStringError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'isHexString', reason)


class ConstraintStartsWithError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'startsWith', reason)


class ConstraintEndsWithError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'endsWith', reason)


class ConstraintIsUpperCaseError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'isUpperCase', reason)


class ConstraintIsLowerCaseError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'isLowerCase', reason)


class ConstraintMinValueError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'minValue', reason)


class ConstraintMaxValueError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'maxValue', reason)


class ConstraintNotValueError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'notValue', reason)


class ConstraintAnyOfError(ConstraintCheckError):
    def __init__(self, context, reason):
        super().__init__(context, 'anyOf', reason)


class BaseConstraint(ABC):
    '''Constraint Interface'''

    def __init__(self, context, types: list, is_null_allowed: bool = False):
        '''An empty type list provided that indicates any type is accepted'''
        self._types = types
        self._is_null_allowed = is_null_allowed
        self._context = context

    def validate(self, value, value_type_name):
        if value is None and self._is_null_allowed:
            return

        response_type = type(value)
        if self._types:
            found_type_match = any(
                [issubclass(response_type, expected) for expected in self._types])
            if not found_type_match:
                if len(self._types) == 1:
                    expected_str = f'type "{self._types[0].__name__}"'
                else:
                    expected_str = f'one of those types: {[x.__name__ for x in self._types]}'
                reason = f'This constraint can only be used with a value of {expected_str} but the value is of type "{response_type.__name__}".'
                self._raise_error(reason)

        if self.check_response(value, value_type_name):
            return

        reason = self.get_reason(value, value_type_name)
        self._raise_error(reason)

    @abstractmethod
    def check_response(self, value, value_type_name) -> bool:
        pass

    @abstractmethod
    def get_reason(self, value, value_type_name) -> str:
        """Get the a human readable explanation about the failure."""
        pass

    def _raise_error(self, reason):
        if isinstance(self, _ConstraintType):
            raise ConstraintTypeError(self._context, reason)
        elif isinstance(self, _ConstraintContains):
            raise ConstraintContainsError(self._context, reason)
        elif isinstance(self, _ConstraintExcludes):
            raise ConstraintExcludesError(self._context, reason)
        elif isinstance(self, _ConstraintHasMaskClear):
            raise ConstraintHasMaskClearError(self._context, reason)
        elif isinstance(self, _ConstraintHasMaskSet):
            raise ConstraintHasMaskSetError(self._context, reason)
        elif isinstance(self, _ConstraintMinLength):
            raise ConstraintMinLengthError(self._context, reason)
        elif isinstance(self, _ConstraintMaxLength):
            raise ConstraintMaxLengthError(self._context, reason)
        elif isinstance(self, _ConstraintIsHexString):
            raise ConstraintIsHexStringError(self._context, reason)
        elif isinstance(self, _ConstraintStartsWith):
            raise ConstraintStartsWithError(self._context, reason)
        elif isinstance(self, _ConstraintEndsWith):
            raise ConstraintEndsWithError(self._context, reason)
        elif isinstance(self, _ConstraintIsUpperCase):
            raise ConstraintIsUpperCaseError(self._context, reason)
        elif isinstance(self, _ConstraintIsLowerCase):
            raise ConstraintIsLowerCaseError(self._context, reason)
        elif isinstance(self, _ConstraintMinValue):
            raise ConstraintMinValueError(self._context, reason)
        elif isinstance(self, _ConstraintMaxValue):
            raise ConstraintMaxValueError(self._context, reason)
        elif isinstance(self, _ConstraintNotValue):
            raise ConstraintNotValueError(self._context, reason)
        elif isinstance(self, _ConstraintAnyOf):
            raise ConstraintAnyOfError(self._context, reason)
        else:
            # This should not happens.
            raise ConstraintParseError('Unknown constraint instance.')


class _ConstraintHasValue(BaseConstraint):
    def __init__(self, context, has_value):
        super().__init__(context, types=[])
        self._has_value = has_value

    def validate(self, value, value_type_name):
        # We are overriding the BaseConstraint of validate since has value is a special case where
        # we might not be expecting a value at all, but the basic null check in BaseConstraint
        # is not what we want.
        if self.check_response(value, value_type_name):
            return

        reason = self.get_reason(value, value_type_name)
        raise ConstraintHasValueError(self._context, reason)

    def check_response(self, value, value_type_name) -> bool:
        has_value = value is not None
        return self._has_value == has_value

    def get_reason(self, value, value_type_name) -> str:
        if self._has_value:
            return "The constraint expects a value but there isn't one."
        return f"The response contains the value ({value}), but wasn't expecting any value."


class _ConstraintType(BaseConstraint):
    def __init__(self, context, type):
        super().__init__(context, types=[], is_null_allowed=True)
        self._type = type

    def check_response(self, value, value_type_name) -> bool:
        success = False
        if self._type == 'boolean' and type(value) is bool:
            success = True
        elif self._type == 'struct' and type(value) is dict:
            success = True
        elif self._type == 'list' and type(value) is list:
            success = True
        elif self._type == 'char_string' and type(value) is str:
            success = True
        elif self._type == 'long_char_string' and type(value) is str:
            success = True
        elif self._type == 'octet_string' and type(value) is bytes:
            success = True
        elif self._type == 'long_octet_string' and type(value) is bytes:
            success = True
        elif self._type == 'group_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'vendor_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFF
        elif self._type == 'devtype_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFF
        elif self._type == 'nullable_cluster_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFE
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
        elif self._type == 'nullable_node_id' and type(value) is int:
            success = value >= 0 and value <= 0xFFFFFFFFFFFFFFFE
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
        elif self._type == 'enum8' and isinstance(value, int):
            success = value >= 0 and value <= 0xFF
        elif self._type == 'enum16' and isinstance(value, int):
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
        elif self._type == 'single' and type(value) is float:
            success = self._is_single(value)
        elif self._type == 'double' and type(value) is float:
            success = self._is_double(value)
        else:
            success = self._type == value_type_name
        return success

    def get_reason(self, value, value_type_name) -> str:
        types = []

        if type(value) is bool:
            types.append('boolean')
        elif type(value) is dict:
            types.append('struct')
        elif type(value) is list:
            types.append('list')
        elif type(value) is str:
            types.append('char_string')
            types.append('long_char_string')
        elif type(value) is bytes:
            types.append('octet_string')
            types.append('long_octet_string')
        elif type(value) is int:
            if value >= 0 and value <= 0xFE:
                types.append('nullable_int8u')

            if value >= 0 and value <= 0xFF:
                types.append('action_id')
                types.append('bitmap8')
                types.append('enum8')
                types.append('Percent')
                types.append('int8u')

            if value >= 0 and value <= 0xFFFE:
                types.append('nullable_int16u')

            if value >= 0 and value <= 0xFFFF:
                types.append('vendor_id')
                types.append('group_id')
                types.append('bitmap16')
                types.append('enum16')
                types.append('Percent100ths')
                types.append('int16u')

            if value >= 0 and value <= 0xFFFFFE:
                types.append('nullable_int24u')

            if value >= 0 and value <= 0xFFFFFF:
                types.append('int24u')

            if value >= 0 and value <= 0xFFFFFFFE:
                types.append('nullable_int32u')
                types.append('nullable_cluster_id')

            if value >= 0 and value <= 0xFFFFFFFF:
                types.append('device_type_id')
                types.append('cluster_id')
                types.append('attribute_id')
                types.append('field_id')
                types.append('command_id')
                types.append('event_id')
                types.append('transaction_id')
                types.append('bitmap32')
                types.append('epoch_s')
                types.append('utc')
                types.append('date')
                types.append('tod')
                types.append('int32u')

            if value >= 0 and value <= 0xFFFFFFFFFE:
                types.append('nullable_int40u')

            if value >= 0 and value <= 0xFFFFFFFFFF:
                types.append('int40u')

            if value >= 0 and value <= 0xFFFFFFFFFFFE:
                types.append('nullable_int48u')

            if value >= 0 and value <= 0xFFFFFFFFFFFF:
                types.append('int48u')

            if value >= 0 and value <= 0xFFFFFFFFFFFFFE:
                types.append('nullable_int56u')

            if value >= 0 and value <= 0xFFFFFFFFFFFFFF:
                types.append('int56u')

            if value >= 0 and value <= 0xFFFFFFFFFFFFFFFE:
                types.append('nullable_int64u')
                types.append('nullable_node_id')

            if value >= 0 and value <= 0xFFFFFFFFFFFFFFFF:
                types.append('node_id')
                types.append('bitmap64')
                types.append('epoch_us')
                types.append('int64u')

            if value >= -128 and value <= 127:
                types.append('int8s')

            if value >= -32768 and value <= 32767:
                types.append('int16s')

            if value >= -8388608 and value <= 8388607:
                types.append('int24s')

            if value >= -2147483648 and value <= 2147483647:
                types.append('int32s')

            if value >= -549755813888 and value <= 549755813887:
                types.append('int40s')

            if value >= -140737488355328 and value <= 140737488355327:
                types.append('int48s')

            if value >= -36028797018963968 and value <= 36028797018963967:
                types.append('int56s')

            if value >= -9223372036854775808 and value <= 9223372036854775807:
                types.append('int64s')

            if value >= -127 and value <= 127:
                types.append('nullable_int8s')

            if value >= -32767 and value <= 32767:
                types.append('nullable_int16s')

            if value >= -8388607 and value <= 8388607:
                types.append('nullable_int24s')

            if value >= -2147483647 and value <= 2147483647:
                types.append('nullable_int32s')

            if value >= -549755813887 and value <= 549755813887:
                types.append('nullable_int40s')

            if value >= -140737488355327 and value <= 140737488355327:
                types.append('nullable_int48s')

            if value >= -36028797018963967 and value <= 36028797018963967:
                types.append('nullable_int56s')

            if value >= -9223372036854775807 and value <= 9223372036854775807:
                types.append('nullable_int64s')

            if self._is_single(value):
                types.append('single')

            if self._is_double(value):
                types.append('double')

        types.sort(key=lambda input_type: [int(c) if c.isdigit(
        ) else c for c in re.split('([0-9]+)', input_type)])

        if value_type_name not in types:
            types.append(value_type_name)

        if len(types) == 1:
            reason = f'The response type ({types[0]}) does not match the constraint.'
        else:
            reason = f'The response value ({value}) is of one of those types: {types}.'
        return reason

    def _is_single(self, value):
        return (value >= -1.7976931348623157E+308 and value <= -2.2250738585072014E-308) or value == 0.0 or (
            value >= 2.2250738585072014E-308 and value <= 1.7976931348623157E+308) or math.isnan(value) or math.isinf(value)

    def _is_double(self, value):
        return (value >= -1.7976931348623157E+308 and value <= -2.2250738585072014E-308) or value == 0.0 or (
            value >= 2.2250738585072014E-308 and value <= 1.7976931348623157E+308) or math.isnan(value) or math.isinf(value)


class _ConstraintMinLength(BaseConstraint):
    def __init__(self, context, min_length):
        super().__init__(context, types=[
            str, bytes, list], is_null_allowed=True)
        self._min_length = min_length

    def check_response(self, value, value_type_name) -> bool:
        return len(value) >= self._min_length

    def get_reason(self, value, value_type_name) -> str:
        return f'The response length ({len(value)}) should be greater or equal to the constraint but {len(value)} < {self._min_length}.'


class _ConstraintMaxLength(BaseConstraint):
    def __init__(self, context, max_length):
        super().__init__(context, types=[
            str, bytes, list], is_null_allowed=True)
        self._max_length = max_length

    def check_response(self, value, value_type_name) -> bool:
        return len(value) <= self._max_length

    def get_reason(self, value, value_type_name) -> str:
        return f'The response length ({len(value)}) should be lower or equal to the constraint but {len(value)} > {self._max_length}.'


class _ConstraintIsHexString(BaseConstraint):
    def __init__(self, context, is_hex_string: bool):
        super().__init__(context, types=[str])
        self._is_hex_string = is_hex_string

    def check_response(self, value, value_type_name) -> bool:
        return all(c in string.hexdigits for c in value) == self._is_hex_string

    def get_reason(self, value, value_type_name) -> str:
        if self._is_hex_string:
            chars = []

            for char in value:
                if char not in string.hexdigits:
                    chars.append(char)

            if len(chars) == 1:
                reason = f'The response "{value}" contains an invalid hexadecimal character: "{chars[0]}".'
            else:
                reason = f'The response "{value}" contains invalid hexadecimal characters: {chars}.'
        else:
            reason = f'The response "{value}" is an hexadecimal string.'
        return reason


class _ConstraintStartsWith(BaseConstraint):
    def __init__(self, context, starts_with):
        super().__init__(context, types=[str])
        self._starts_with = starts_with

    def check_response(self, value, value_type_name) -> bool:
        return value.startswith(self._starts_with)

    def get_reason(self, value, value_type_name) -> str:
        return f'The response "{value}" starts with "{value[:len(self._starts_with)]}" which does not match the constraint.'


class _ConstraintEndsWith(BaseConstraint):
    def __init__(self, context, ends_with):
        super().__init__(context, types=[str])
        self._ends_with = ends_with

    def check_response(self, value, value_type_name) -> bool:
        return value.endswith(self._ends_with)

    def get_reason(self, value, value_type_name) -> str:
        return f'The response "{value}" ends with "{value[-len(self._ends_with):]}" which does not match the constraint.'


class _ConstraintIsUpperCase(BaseConstraint):
    def __init__(self, context, is_upper_case):
        super().__init__(context, types=[str])
        self._is_upper_case = is_upper_case

    def check_response(self, value, value_type_name) -> bool:
        # Make sure we don't have any lowercase characters.
        hasLower = any(c.islower() for c in value)
        return hasLower != self._is_upper_case

    def get_reason(self, value, value_type_name) -> str:
        if self._is_upper_case:
            chars = []

            for char in value:
                if not char.upper() == char:
                    chars.append(char)

            if len(chars) == 1:
                reason = f'The response "{value}" contains a lowercase character: "{chars[0]}".'
            else:
                reason = f'The response "{value}" contains lowercase characters: {chars}.'
        else:
            reason = f'The response "{value}" is uppercased.'

        return reason


class _ConstraintIsLowerCase(BaseConstraint):
    def __init__(self, context, is_lower_case):
        super().__init__(context, types=[str])
        self._is_lower_case = is_lower_case

    def check_response(self, value, value_type_name) -> bool:
        # Make sure we don't have any uppercase characters.
        hasUpper = any(c.isupper() for c in value)
        return hasUpper != self._is_lower_case

    def get_reason(self, value, value_type_name) -> str:
        if self._is_lower_case:
            chars = []

            for char in value:
                if not char.lower() == char:
                    chars.append(char)

            if len(chars) == 1:
                reason = f'The response "{value}" contains a uppercase character: "{chars[0]}".'
            else:
                reason = f'The response "{value}" contains uppercase characters: {chars}.'
        else:
            reason = f'The response "{value}" is lowercased.'

        return reason


class _ConstraintMinValue(BaseConstraint):
    def __init__(self, context, min_value):
        super().__init__(context, types=[int, float], is_null_allowed=True)
        self._min_value = min_value

    def check_response(self, value, value_type_name) -> bool:
        return value >= self._min_value

    def get_reason(self, value, value_type_name) -> str:
        return f'The response value ({value}) should be greater or equal to the constraint but {value} < {self._min_value}.'


class _ConstraintMaxValue(BaseConstraint):
    def __init__(self, context, max_value):
        super().__init__(context, types=[int, float], is_null_allowed=True)
        self._max_value = max_value

    def check_response(self, value, value_type_name) -> bool:
        return value <= self._max_value

    def get_reason(self, value, value_type_name) -> str:
        return f'The response value ({value}) should be lower or equal to the constraint but {value} > {self._max_value}.'


def _values_match(expected_value, received_value):
    # TODO: This is a copy of _response_value_validation over in parser.py,
    # but with the recursive calls renamed.
    if isinstance(expected_value, list):
        if len(expected_value) != len(received_value):
            return False

        for index, expected_item in enumerate(expected_value):
            received_item = received_value[index]
            if not _values_match(expected_item, received_item):
                return False
        return True
    elif isinstance(expected_value, dict):
        for key, expected_item in expected_value.items():
            received_item = received_value.get(key)
            if not _values_match(expected_item, received_item):
                return False
        return True
    else:
        return expected_value == received_value


class _ConstraintContains(BaseConstraint):
    def __init__(self, context, contains):
        super().__init__(context, types=[list])
        self._contains = contains

    def _find_missing_values(self, expected_values, received_values):
        # Make a copy of received_values, so that we can remove things from the
        # list as they match up against our expected values.
        received_values = list(received_values)
        missing_values = []
        for expected_value in expected_values:
            for index, received_value in enumerate(received_values):
                if _values_match(expected_value, received_value):
                    # We've used up this received value
                    del received_values[index]
                    break
            else:
                missing_values.append(expected_value)
        return missing_values

    def check_response(self, value, value_type_name) -> bool:
        return len(self._find_missing_values(self._contains, value)) == 0

    def get_reason(self, value, value_type_name) -> str:
        expected_values = self._find_missing_values(self._contains, value)

        return f'The response ({value}) is missing {expected_values}.'


class _ConstraintExcludes(BaseConstraint):
    def __init__(self, context, excludes):
        super().__init__(context, types=[list])
        self._excludes = excludes

    def check_response(self, value, value_type_name) -> bool:
        for expected_value in self._excludes:
            for received_value in value:
                if _values_match(expected_value, received_value):
                    return False
        return True

    def get_reason(self, value, value_type_name) -> str:
        unexpected_values = []

        for unexpected_value in self._excludes:
            for received_value in value:
                if _values_match(unexpected_value, received_value):
                    unexpected_values.append(unexpected_value)

        return f'The response ({value}) contains {unexpected_values}.'


class _ConstraintHasMaskSet(BaseConstraint):
    def __init__(self, context, has_masks_set):
        super().__init__(context, types=[int])
        self._has_masks_set = has_masks_set

    def check_response(self, value, value_type_name) -> bool:
        return all([(value & mask) != 0 for mask in self._has_masks_set])

    def get_reason(self, value, value_type_name) -> str:
        expected_masks = []

        for expected_mask in self._has_masks_set:
            if (value & expected_mask) != expected_mask:
                expected_masks.append(hex(expected_mask))

        return f'The response ({hex(value)}) does not match the masks: {expected_masks}.'


class _ConstraintHasMaskClear(BaseConstraint):
    def __init__(self, context, has_masks_clear):
        super().__init__(context, types=[int])
        self._has_masks_clear = has_masks_clear

    def check_response(self, value, value_type_name) -> bool:
        return all([(value & mask) == 0 for mask in self._has_masks_clear])

    def get_reason(self, value, value_type_name) -> str:
        unexpected_masks = []

        for unexpected_mask in self._has_masks_clear:
            if (value & unexpected_mask) == unexpected_mask:
                unexpected_masks.append(hex(unexpected_mask))

        return f'The response ({hex(value)}) match the masks: {unexpected_masks}.'


class _ConstraintNotValue(BaseConstraint):
    def __init__(self, context, not_value):
        super().__init__(context, types=[], is_null_allowed=True)
        self._not_value = not_value

    def check_response(self, value, value_type_name) -> bool:
        return value != self._not_value

    def get_reason(self, value, value_type_name) -> str:
        return f'The response value "{value}" should differs from the constraint.'


class _ConstraintAnyOf(BaseConstraint):
    def __init__(self, context, any_of):
        super().__init__(context, types=[], is_null_allowed=True)
        self._any_of = any_of

    def check_response(self, value, value_type_name) -> bool:
        return value in self._any_of

    def get_reason(self, value, value_type_name) -> str:
        return f'The response value "{value}" is not a value from {self._any_of}.'


def get_constraints(constraints: dict) -> List[BaseConstraint]:
    _constraints = []
    context = constraints

    for constraint, constraint_value in constraints.items():
        if 'hasValue' == constraint:
            _constraints.append(_ConstraintHasValue(
                context, constraint_value))
        elif 'type' == constraint:
            _constraints.append(_ConstraintType(context, constraint_value))
        elif 'minLength' == constraint:
            _constraints.append(_ConstraintMinLength(
                context, constraint_value))
        elif 'maxLength' == constraint:
            _constraints.append(_ConstraintMaxLength(
                context, constraint_value))
        elif 'isHexString' == constraint:
            _constraints.append(_ConstraintIsHexString(
                context, constraint_value))
        elif 'startsWith' == constraint:
            _constraints.append(_ConstraintStartsWith(
                context, constraint_value))
        elif 'endsWith' == constraint:
            _constraints.append(_ConstraintEndsWith(
                context, constraint_value))
        elif 'isUpperCase' == constraint:
            _constraints.append(_ConstraintIsUpperCase(
                context, constraint_value))
        elif 'isLowerCase' == constraint:
            _constraints.append(_ConstraintIsLowerCase(
                context, constraint_value))
        elif 'minValue' == constraint:
            _constraints.append(_ConstraintMinValue(
                context, constraint_value))
        elif 'maxValue' == constraint:
            _constraints.append(_ConstraintMaxValue(
                context, constraint_value))
        elif 'contains' == constraint:
            _constraints.append(_ConstraintContains(
                context, constraint_value))
        elif 'excludes' == constraint:
            _constraints.append(_ConstraintExcludes(
                context, constraint_value))
        elif 'hasMasksSet' == constraint:
            _constraints.append(_ConstraintHasMaskSet(
                context, constraint_value))
        elif 'hasMasksClear' == constraint:
            _constraints.append(_ConstraintHasMaskClear(
                context, constraint_value))
        elif 'notValue' == constraint:
            _constraints.append(_ConstraintNotValue(
                context, constraint_value))
        elif 'anyOf' == constraint:
            _constraints.append(_ConstraintAnyOf(
                context, constraint_value))
        else:
            raise ConstraintParseError(f'Unknown constraint type:{constraint}')

    return _constraints


def is_typed_constraint(constraint: str):
    constraints = {
        'hasValue': False,
        'type': False,
        'minLength': False,
        'maxLength': False,
        'isHexString': False,
        'startsWith': True,
        'endsWith': True,
        'isUpperCase': False,
        'isLowerCase': False,
        'minValue': True,
        'maxValue': True,
        'contains': True,
        'excludes': True,
        'hasMasksSet': False,
        'hasMasksClear': False,
        'notValue': True,
        'anyOf': True,
    }

    is_typed = constraints.get(constraint)
    if is_typed is None:
        raise ConstraintParseError(f'Unknown constraint type:{constraint}')
    return is_typed
