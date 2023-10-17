# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from typing import Optional

from matter_idl.generators.types import GetDataTypeSizeInBits, IsSignedDataType
from matter_idl.matter_idl_types import AccessPrivilege, Attribute, AttributeQuality, DataType, Field, FieldQuality


def ParseInt(value: str, data_type: Optional[DataType] = None) -> int:
    """
    Convert a string that is a known integer into an actual number.

    Supports decimal or hex values prefixed with '0x'
    """
    if value.startswith('0x'):
        value = int(value[2:], 16)
        if data_type and IsSignedDataType(data_type):
            bits = GetDataTypeSizeInBits(data_type)
            if value & (1 << (bits - 1)):
                value -= 1 << bits
        return value
    else:
        return int(value)

