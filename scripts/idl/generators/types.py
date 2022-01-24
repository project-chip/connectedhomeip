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

import logging
import enum

from idl.matter_idl_types import DataType
from idl import matter_idl_types  # to explicitly say 'Enum'

from typing import Union, List
from dataclasses import dataclass


def ToPowerOfTwo(bits: int) -> int:
    # probably bit manipulation can be faster, but this should be ok as well
    result = 1
    while result < bits:
        result = result * 2
    return result


@dataclass
class BasicInteger:
    """
    Represents something that is stored as a basic integer.
    """
    idl_name: str
    byte_count: int  # NOTE: may NOT be a power of 2 for odd sized integers
    is_signed: bool

    @property
    def bits(self):
        return self.byte_count * 8

    @property
    def power_of_two_bits(self):
        return ToPowerOfTwo(self.bits)


@dataclass
class BasicString:
    idl_name: str
    is_binary: bool
    max_length: Union[int, None] = None


class FundamentalType(enum.Enum):
    BOOL = enum.auto()
    FLOAT = enum.auto()
    DOUBLE = enum.auto()

    @property
    def idl_name(self):
        if self == FundamentalType.BOOL:
            return "bool"
        elif self == FundamentalType.FLOAT:
            return "single"
        elif self == FundamentalType.DOUBLE:
            return "double"
        else:
            raise Error("Type not handled: %r" % self)

    @property
    def byte_count(self):
        if self == FundamentalType.BOOL:
            return 1
        elif self == FundamentalType.FLOAT:
            return 4
        elif self == FundamentalType.DOUBLE:
            return 8
        else:
            raise Error("Type not handled: %r" % self)

    @property
    def bits(self):
        return self.byte_count * 8


@dataclass
class IdlEnumType:
    idl_name: str
    base_type: BasicInteger

    @property
    def byte_count(self):
        return base_type.byte_count()

    @property
    def bits(self):
        return base_type.bits()


@dataclass
class IdlType:
    idl_name: str


# Data types, held by ZAP in chip-types.xml
__CHIP_SIZED_TYPES__ = {
    "bitmap16": BasicInteger(idl_name="bitmap16", byte_count=2, is_signed=False),
    "bitmap24": BasicInteger(idl_name="bitmap24", byte_count=3, is_signed=False),
    "bitmap32": BasicInteger(idl_name="bitmap32", byte_count=4, is_signed=False),
    "bitmap64": BasicInteger(idl_name="bitmap64", byte_count=8, is_signed=False),
    "bitmap8": BasicInteger(idl_name="bitmap8", byte_count=1, is_signed=False),
    "enum16": BasicInteger(idl_name="enum16", byte_count=2, is_signed=False),
    "enum8": BasicInteger(idl_name="enum8", byte_count=1, is_signed=False),
    "int16s": BasicInteger(idl_name="int16s", byte_count=2, is_signed=True),
    "int16u": BasicInteger(idl_name="int16u", byte_count=2, is_signed=False),
    "int24s": BasicInteger(idl_name="int24s", byte_count=3, is_signed=True),
    "int24u": BasicInteger(idl_name="int24u", byte_count=3, is_signed=False),
    "int32s": BasicInteger(idl_name="int32s", byte_count=4, is_signed=True),
    "int32u": BasicInteger(idl_name="int32u", byte_count=4, is_signed=False),
    "int40s": BasicInteger(idl_name="int40s", byte_count=5, is_signed=True),
    "int40u": BasicInteger(idl_name="int40u", byte_count=5, is_signed=False),
    "int48s": BasicInteger(idl_name="int48s", byte_count=6, is_signed=True),
    "int48u": BasicInteger(idl_name="int48u", byte_count=6, is_signed=False),
    "int56s": BasicInteger(idl_name="int56s", byte_count=7, is_signed=True),
    "int56u": BasicInteger(idl_name="int56u", byte_count=7, is_signed=False),
    "int64s": BasicInteger(idl_name="int64s", byte_count=8, is_signed=True),
    "int64u": BasicInteger(idl_name="int64u", byte_count=8, is_signed=False),
    "int8s": BasicInteger(idl_name="int8s", byte_count=1, is_signed=True),
    "int8u": BasicInteger(idl_name="int8u", byte_count=1, is_signed=False),
    # Derived types
    "action_id": BasicInteger(idl_name="action_id", byte_count=1, is_signed=True),
    "attrib_id": BasicInteger(idl_name="attrib_id", byte_count=4, is_signed=True),
    "cluster_id": BasicInteger(idl_name="cluster_id", byte_count=4, is_signed=True),
    "command_id": BasicInteger(idl_name="command_id", byte_count=4, is_signed=True),
    "data_ver": BasicInteger(idl_name="data_ver", byte_count=4, is_signed=True),
    "date": BasicInteger(idl_name="date", byte_count=4, is_signed=True),
    "devtype_id": BasicInteger(idl_name="devtype_id", byte_count=4, is_signed=True),
    "endpoint_no": BasicInteger(idl_name="endpoint_no", byte_count=2, is_signed=True),
    "epoch_s": BasicInteger(idl_name="epoch_s", byte_count=4, is_signed=False),
    "epoch_us": BasicInteger(idl_name="epoch_us", byte_count=8, is_signed=False),
    "event_id": BasicInteger(idl_name="event_id", byte_count=4, is_signed=True),
    "event_no": BasicInteger(idl_name="event_no", byte_count=8, is_signed=True),
    "fabric_id": BasicInteger(idl_name="fabric_id", byte_count=8, is_signed=True),
    "fabric_idx": BasicInteger(idl_name="fabric_idx", byte_count=1, is_signed=True),
    "field_id": BasicInteger(idl_name="field_id", byte_count=4, is_signed=True),
    "group_id": BasicInteger(idl_name="group_id", byte_count=2, is_signed=True),
    "node_id": BasicInteger(idl_name="node_id", byte_count=8, is_signed=True),
    "percent": BasicInteger(idl_name="percent", byte_count=1, is_signed=True),
    "percent100ths": BasicInteger(idl_name="percent100ths", byte_count=2, is_signed=True),
    "status": BasicInteger(idl_name="status", byte_count=2, is_signed=True),
    "systime_us": BasicInteger(idl_name="systime_us", byte_count=8, is_signed=True),
    "tod": BasicInteger(idl_name="tod", byte_count=4, is_signed=True),
    "trans_id": BasicInteger(idl_name="trans_id", byte_count=4, is_signed=True),
    "utc": BasicInteger(idl_name="utc", byte_count=4, is_signed=True),
    "vendor_id": BasicInteger(idl_name="vendor_id", byte_count=2, is_signed=True),
}


def ParseDataType(data_type: DataType, known_enum_types: List[matter_idl_types.Enum]) -> Union[BasicInteger, BasicString, FundamentalType, IdlType]:
    """
    Match the given string name to a potentially known type
    """

    lowercase_name = data_type.name.lower()

    if lowercase_name == 'boolean':
        return FundamentalType.BOOL
    if lowercase_name == 'single':
        return FundamentalType.FLOAT
    elif lowercase_name == 'double':
        return FundamentalType.DOUBLE
    elif lowercase_name in ['char_string', 'long_char_string']:
        return BasicString(idl_name=lowercase_name, is_binary=False, max_length=data_type.max_length)
    elif lowercase_name in ['octet_string', 'long_octet_string']:
        return BasicString(idl_name=lowercase_name, is_binary=True, max_length=data_type.max_length)

    int_type = __CHIP_SIZED_TYPES__.get(lowercase_name, None)
    if int_type is not None:
        return int_type

    # All fast checks done, now check against known enumerations
    for e in known_enum_types:
        if e.name == data_type.name:
            # Valid enum found. it MUST be based on a valid data type
            return IdlEnumType(idl_name=data_type.name, base_type=__CHIP_SIZED_TYPES__[e.base_type.lower()])

    return IdlType(idl_name=data_type.name)
