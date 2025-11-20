#
#    Copyright (c) 2023 Project CHIP Authors
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

from threading import Lock

from aenum import IntEnum, extend_enum  # type: ignore

# Flag on whether we should map unknown enum values to kUnknownEnumValue.
_map_missing_enum_to_unknown_enum_value = True
_placeholder_count_lock = Lock()
# Count that is used to append the end of placeholder enum names in
# MatterIntEnum.extend_enum_if_value_doesnt_exist.
_placeholder_count = 0


class MatterIntEnum(IntEnum):
    '''Matter implementation of integer enum.

    This provides flexibility so that we don't have to rely on the strongly typed
    nature of built in enum. By default, globally, all unknown enum mapping are
    turned into kUnknownEnumValue. This also give capability of extending the enum
    at runtime allowing for test code to test behaviour of sending out out of scope
    enum values.
    '''
    @classmethod
    def _missing_(cls, value):
        if _map_missing_enum_to_unknown_enum_value:
            return cls.kUnknownEnumValue
        return None

    @classmethod
    def extend_enum_if_value_doesnt_exist(cls, value):
        try:
            return_value = cls(value)
        except ValueError:
            return_value = None

        if return_value is None or return_value.value != value:
            global _placeholder_count_lock
            global _placeholder_count
            with _placeholder_count_lock:
                return_value = extend_enum(cls, f'kUnknownPlaceholder{_placeholder_count}', value)
                _placeholder_count = _placeholder_count + 1

        return return_value


def set_map_missing_enum_to_unknown_enum_value(value: bool):
    '''Sets flag that handles what to do on unknown enum value type.'''
    global _map_missing_enum_to_unknown_enum_value
    _map_missing_enum_to_unknown_enum_value = value
