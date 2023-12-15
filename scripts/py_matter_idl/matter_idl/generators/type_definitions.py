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

import enum
import logging
from dataclasses import dataclass
from typing import Optional, Union

from matter_idl import matter_idl_types  # to explicitly say 'Enum'
from matter_idl.matter_idl_types import DataType


def ToPowerOfTwo(bits: int) -> int:
    """
    Given a number, find the next power of two that is >= to the given value.

    Can be used to figure out a variable size given non-standard bit sizes in
    matter: eg. a int24 can be stored in an int32, so ToPortOfTwo(24) == 32.

    """
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
    """
    Represents either a string or a binary string (blob).
    """
    idl_name: str
    is_binary: bool
    max_length: Union[int, None] = None


class FundamentalType(enum.Enum):
    """
    Native types, generally available across C++/ObjC/Java/python/other.
    """
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
            raise Exception("Type not handled: %r" % self)

    @property
    def byte_count(self):
        if self == FundamentalType.BOOL:
            return 1
        elif self == FundamentalType.FLOAT:
            return 4
        elif self == FundamentalType.DOUBLE:
            return 8
        else:
            raise Exception("Type not handled: %r" % self)

    @property
    def bits(self):
        return self.byte_count * 8


@dataclass
class IdlEnumType:
    """
    An enumeration type. Enumerations are constants with an underlying
    base type that is an interger.
    """
    idl_name: str
    base_type: BasicInteger

    @property
    def byte_count(self):
        return self.base_type.byte_count

    @property
    def bits(self):
        return self.base_type.bits


@dataclass
class IdlBitmapType:
    """
    Bitmaps mark that each bit (or a subset of said bits) have a meaning.

    Examples include "feature maps" where bits represent feature available or not.
    """
    idl_name: str
    base_type: BasicInteger

    @property
    def byte_count(self):
        return self.base_type.byte_count

    @property
    def bits(self):
        return self.base_type.bits


class IdlItemType(enum.Enum):
    UNKNOWN = enum.auto()
    STRUCT = enum.auto()


@dataclass
class IdlType:
    """
    A type defined within the IDL.

    IDLs would generally only define structures as all other types are
    described in other things like enums/bitmaps/basic types etc.

    However since IDL parsing is not yet codegen just syntactically, we allow
    the option to have a type that is marked 'unknown' (likely invalid/never
    defined).
    """
    idl_name: str
    item_type: IdlItemType

    @property
    def is_struct(self) -> bool:
        return self.item_type == IdlItemType.STRUCT


# Data types, held by ZAP in chip-types.xml and generally by the spec.
__CHIP_SIZED_TYPES__ = {
    "bitmap16": BasicInteger(idl_name="bitmap16", byte_count=2, is_signed=False),
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
    # Specification describes them in section '7.19.2. Derived Data Types'
    "action_id": BasicInteger(idl_name="action_id", byte_count=1, is_signed=False),
    "amperage_ma": BasicInteger(idl_name="amperage_ma", byte_count=8, is_signed=True),
    "attrib_id": BasicInteger(idl_name="attrib_id", byte_count=4, is_signed=False),
    "cluster_id": BasicInteger(idl_name="cluster_id", byte_count=4, is_signed=False),
    "command_id": BasicInteger(idl_name="command_id", byte_count=4, is_signed=False),
    "data_ver": BasicInteger(idl_name="data_ver", byte_count=4, is_signed=False),
    "date": BasicInteger(idl_name="date", byte_count=4, is_signed=False),
    "devtype_id": BasicInteger(idl_name="devtype_id", byte_count=4, is_signed=False),
    "elapsed_s": BasicInteger(idl_name="elapsed_s", byte_count=4, is_signed=False),
    "endpoint_no": BasicInteger(idl_name="endpoint_no", byte_count=2, is_signed=False),
    "energy_mwh":  BasicInteger(idl_name="energy_mwh", byte_count=8, is_signed=True),
    "entry_idx": BasicInteger(idl_name="entry_idx", byte_count=2, is_signed=False),
    "epoch_s": BasicInteger(idl_name="epoch_s", byte_count=4, is_signed=False),
    "epoch_us": BasicInteger(idl_name="epoch_us", byte_count=8, is_signed=False),
    "event_id": BasicInteger(idl_name="event_id", byte_count=4, is_signed=False),
    "event_no": BasicInteger(idl_name="event_no", byte_count=8, is_signed=False),
    "fabric_id": BasicInteger(idl_name="fabric_id", byte_count=8, is_signed=False),
    "fabric_idx": BasicInteger(idl_name="fabric_idx", byte_count=1, is_signed=False),
    "field_id": BasicInteger(idl_name="field_id", byte_count=4, is_signed=False),
    "group_id": BasicInteger(idl_name="group_id", byte_count=2, is_signed=False),
    "namespace": BasicInteger(idl_name="namespace", byte_count=1, is_signed=False),
    "node_id": BasicInteger(idl_name="node_id", byte_count=8, is_signed=False),
    "percent": BasicInteger(idl_name="percent", byte_count=1, is_signed=False),
    "percent100ths": BasicInteger(idl_name="percent100ths", byte_count=2, is_signed=False),
    "posix_ms": BasicInteger(idl_name="posix_ms", byte_count=8, is_signed=False),
    "power_mw": BasicInteger(idl_name="power_mw", byte_count=8, is_signed=True),
    "priority": BasicInteger(idl_name="priority", byte_count=1, is_signed=False),
    "semtag": BasicInteger(idl_name="semtag", byte_count=4, is_signed=False),
    "status": BasicInteger(idl_name="status", byte_count=1, is_signed=False),
    "systime_ms": BasicInteger(idl_name="systime_ms", byte_count=8, is_signed=False),
    "systime_us": BasicInteger(idl_name="systime_us", byte_count=8, is_signed=False),
    "tag": BasicInteger(idl_name="tag", byte_count=1, is_signed=False),
    "temperature": BasicInteger(idl_name="temperature", byte_count=2, is_signed=True),
    "tod": BasicInteger(idl_name="tod", byte_count=4, is_signed=False),
    "trans_id": BasicInteger(idl_name="trans_id", byte_count=4, is_signed=False),
    "vendor_id": BasicInteger(idl_name="vendor_id", byte_count=2, is_signed=False),
    "voltage_mv": BasicInteger(idl_name="voltage_mv", byte_count=8, is_signed=True),
}


class TypeLookupContext:
    """
    Handles type lookups within a scope.

    Generally when looking for a struct/enum, the lookup will be first done
    at a cluster level, then at a global level.

    Example:

    ================ test.matter ==============
    enum A {}

    server cluster X {
      struct A {}
      struct B {}
    }

    server cluster Y {
      enum C {}
    }
    ===========================================

    When considering a lookup context of global (i.e. cluster is not set)
       "A" is defined as an enum (::A)
       "B" is undefined
       "C" is undefined

    When considering a lookup context of cluster X
       "A" is defined as a struct (X::A)
       "B" is defined as a struct (X::B)
       "C" is undefined

    When considering a lookup context of cluster Y
       "A" is defined as an enum (::A)
       "B" is undefined
       "C" is defined as an enum (Y::C)

    """

    def __init__(self, idl: matter_idl_types.Idl, cluster: Optional[matter_idl_types.Cluster]):
        self.idl = idl
        self.cluster = cluster

    def find_enum(self, name) -> Optional[matter_idl_types.Enum]:
        """
        Find the first enumeration matching the given name for the given
        lookup rules (searches cluster first, then global).
        """
        for e in self.all_enums:
            if e.name == name:
                return e

        return None

    def find_struct(self, name) -> Optional[matter_idl_types.Struct]:
        for s in self.all_structs:
            if s.name == name:
                return s

        return None

    def find_bitmap(self, name) -> Optional[matter_idl_types.Bitmap]:
        for s in self.all_bitmaps:
            if s.name == name:
                return s

        return None

    @property
    def all_enums(self):
        """
        All enumerations defined within this lookup context.

        enums are only defined at cluster level. If lookup context does not
        include a cluster, the enum list will be empty.
        """
        if self.cluster:
            for e in self.cluster.enums:
                yield e

    @property
    def all_bitmaps(self):
        """
        All bitmaps defined within this lookup context.

        bitmaps are only defined at cluster level. If lookup context does not
        include a cluster, the bitmap list will be empty.
        """
        if self.cluster:
            for b in self.cluster.bitmaps:
                yield b

    @property
    def all_structs(self):
        """All structs defined within this lookup context.

        structs are only defined at cluster level. If lookup context does not
        include a cluster, the struct list will be empty.
        """
        if self.cluster:
            for e in self.cluster.structs:
                yield e

    def is_enum_type(self, name: str):
        """
        Determine if the given type name is an enumeration.

        Handles both standard names (like enum8) as well as enumerations defined
        within the current lookup context.
        """
        if name.lower() in ["enum8", "enum16"]:
            return True
        return any(map(lambda e: e.name == name, self.all_enums))

    def is_struct_type(self, name: str):
        """
        Determine if the given type name is type that is known to be a struct
        """
        return any(map(lambda s: s.name == name, self.all_structs))

    def is_untyped_bitmap_type(self, name: str):
        """Determine if the given type is a untyped bitmap (just an interger size)."""
        return name.lower() in {"bitmap8", "bitmap16", "bitmap32", "bitmap64"}

    def is_bitmap_type(self, name: str):
        """
        Determine if the given type name is type that is known to be a bitmap.

        Handles both standard/zcl names (like bitmap32) and types defined within
        the current lookup context.
        """
        if self.is_untyped_bitmap_type(name):
            return True

        return any(map(lambda s: s.name == name, self.all_bitmaps))


def ParseDataType(data_type: DataType, lookup: TypeLookupContext) -> Union[BasicInteger, BasicString, FundamentalType, IdlType, IdlEnumType, IdlBitmapType]:
    """
    Given a AST data type and a lookup context, match it to a type that can be later
    be used for generation.

    AST parsing is textual, so it does not understand what "foo" means. This method
    looks up what "foo" actually means: includes basic types (e.g. bool),
    zcl types (like enums or bitmaps) and does lookups to find structs/enums/bitmaps/etc
    that are defined in the given lookup context.
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
    elif lowercase_name in ['enum8', 'enum16']:
        return IdlEnumType(idl_name=lowercase_name, base_type=__CHIP_SIZED_TYPES__[lowercase_name])
    elif lowercase_name in ['bitmap8', 'bitmap16', 'bitmap32', 'bitmap64']:
        return IdlBitmapType(idl_name=lowercase_name, base_type=__CHIP_SIZED_TYPES__[lowercase_name])

    int_type = __CHIP_SIZED_TYPES__.get(lowercase_name, None)
    if int_type is not None:
        return int_type

    # All fast checks done, now check against known data types
    e = lookup.find_enum(data_type.name)
    if e:
        # Valid enum found. it MUST be based on a valid data type
        return IdlEnumType(idl_name=data_type.name, base_type=__CHIP_SIZED_TYPES__[e.base_type.lower()])

    b = lookup.find_bitmap(data_type.name)
    if b:
        # Valid enum found. it MUST be based on a valid data type
        return IdlBitmapType(idl_name=data_type.name, base_type=__CHIP_SIZED_TYPES__[b.base_type.lower()])

    result = IdlType(idl_name=data_type.name, item_type=IdlItemType.UNKNOWN)
    if lookup.find_struct(data_type.name):
        result.item_type = IdlItemType.STRUCT
    else:
        logging.warning(
            "Data type %s is NOT known, but treating it as a generic IDL type." % data_type)

    return result


def IsSignedDataType(data_type: DataType) -> bool:
    """
    Returns if the data type is a signed type.
    Returns if the data type is a signed data type of False if the data type can not be found.
    """
    lowercase_name = data_type.name.lower()
    sized_type = __CHIP_SIZED_TYPES__.get(lowercase_name, None)
    if sized_type is None:
        return False

    return sized_type.is_signed


def GetDataTypeSizeInBits(data_type: DataType) -> Optional[int]:
    """
    Returns the size in bits for a given data type or None if the data type can not be found.
    """

    lowercase_name = data_type.name.lower()
    sized_type = __CHIP_SIZED_TYPES__.get(lowercase_name, None)
    if sized_type is None:
        return None

    return sized_type.power_of_two_bits
