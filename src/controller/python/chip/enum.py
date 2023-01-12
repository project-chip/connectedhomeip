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

from aenum import IntEnum as AenumsIntEnum
from aenum import extend_enum

_add_missing_enum_value_as_is = False
_map_missing_enum_to_unknown_enum_value = True
_dummy_count = 0


class IntEnum(AenumsIntEnum):
    @classmethod
    def _missing_(cls, value):
        global _add_missing_enum_value_as_is
        if _add_missing_enum_value_as_is:
            global _dummy_count
            return_value = extend_enum(cls, f'kDummy{_dummy_count}', value)
            _dummy_count = _dummy_count + 1
            return return_value
        global _map_missing_enum_to_unknown_enum_value
        if _map_missing_enum_to_unknown_enum_value:
            return cls.kUnknownEnumValue

        return None


def set_add_missing_enum_value_as_is(value: bool):
    global _add_missing_enum_value_as_is
    _add_missing_enum_value_as_is = value


def set_map_missing_enum_to_unknown_enum_value(value: bool):
    global _map_missing_enum_to_unknown_enum_value
    _map_missing_enum_to_unknown_enum_value = value
