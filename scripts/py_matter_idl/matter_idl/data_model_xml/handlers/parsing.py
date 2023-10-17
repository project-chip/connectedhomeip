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


def NormalizeName(name: str) -> str:
    """Convert a free form name from the spec into a programming language
       name that is appropriate for matter IDL.
    """

    # Trim human name separators
    for separator in " /-":
        name = name.replace(separator, '_')
    while '__' in name:
       name = name.replace('__', '_')

    # TODO: should we handle acronyms in some way?

    # At this point, we remove all _ and make sure _ is followed by an uppercase
    while name.endswith('_'):
        name = name[:-1]

    while '_' in name:
        idx = name.find('_')
        name = name[:idx] + name[idx+1].upper() + name[idx+2:]


    return name
