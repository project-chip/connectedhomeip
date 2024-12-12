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

# This file should be removed once we have a good way to get this from the codegen or XML

from enum import Enum, IntEnum, auto


class GlobalAttributeIds(IntEnum):
    ATTRIBUTE_LIST_ID = 0xFFFB
    ACCEPTED_COMMAND_LIST_ID = 0xFFF9
    GENERATED_COMMAND_LIST_ID = 0xFFF8
    FEATURE_MAP_ID = 0xFFFC
    CLUSTER_REVISION_ID = 0xFFFD

    def to_name(self) -> str:
        if self == GlobalAttributeIds.ATTRIBUTE_LIST_ID:
            return "AttributeList"
        if self == GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID:
            return "AcceptedCommandList"
        if self == GlobalAttributeIds.GENERATED_COMMAND_LIST_ID:
            return "GeneratedCommandList"
        if self == GlobalAttributeIds.FEATURE_MAP_ID:
            return "FeatureMap"
        if self == GlobalAttributeIds.CLUSTER_REVISION_ID:
            return "ClusterRevision"


class DeviceTypeIdType(Enum):
    kInvalid = auto(),
    kStandard = auto(),
    kManufacturer = auto(),
    kTest = auto(),


class ClusterIdType(Enum):
    kInvalid = auto()
    kStandard = auto(),
    kManufacturer = auto(),
    kTest = auto(),


class AttributeIdType(Enum):
    kInvalid = auto()
    kStandardGlobal = auto(),
    kStandardNonGlobal = auto(),
    kManufacturer = auto(),
    kTest = auto(),


class CommandIdType(Enum):
    kInvalid = auto()
    kStandardGlobal = auto(),
    kScopedNonGlobal = auto(),
    kManufacturer = auto(),
    kTest = auto(),


# ID helper classes - this allows us to use the values from the prefix and suffix table directly
# because the class handles the non-inclusive range.


class IdRange():
    def __init__(self, min, max):
        self.min_max = range(min, max+1)

    def __contains__(self, key):
        return key in self.min_max


class PrefixIdRange(IdRange):
    def __contains__(self, id: int):
        return super().__contains__(id >> 16)


class SuffixIdRange(IdRange):
    def __contains__(self, id: int):
        return super().__contains__(id & 0xFFFF)


STANDARD_PREFIX = PrefixIdRange(0x0000, 0x0000)
MANUFACTURER_PREFIX = PrefixIdRange(0x0001, 0xFFF0)
TEST_PREFIX = PrefixIdRange(0xFFF1, 0xFFF4)

DEVICE_TYPE_ID_RANGE_SUFFIX = SuffixIdRange(0x0000, 0xBFFF)
CLUSTER_ID_STANDARD_RANGE_SUFFIX = SuffixIdRange(0x0000, 0x7FFF)
CLUSTER_ID_MANUFACTURER_RANGE_SUFFIX = SuffixIdRange(0xFC00, 0xFFFE)
ATTRIBUTE_ID_GLOBAL_RANGE_SUFFIX = SuffixIdRange(0xF000, 0xFFFE)
ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX = SuffixIdRange(0x0000, 0x4FFF)
COMMAND_ID_GLOBAL_STANDARD_SUFFIX = SuffixIdRange(0x00E0, 0x00FF)
COMMAND_ID_NON_GLOBAL_SCOPED_SUFFIX = SuffixIdRange(0x0000, 0x00DF)
COMMAND_ID_SUFFIX = SuffixIdRange(0x0000, 0x00FF)


def device_type_id_type(id: int) -> DeviceTypeIdType:
    if id in STANDARD_PREFIX and id in DEVICE_TYPE_ID_RANGE_SUFFIX:
        return DeviceTypeIdType.kStandard
    if id in MANUFACTURER_PREFIX and id in DEVICE_TYPE_ID_RANGE_SUFFIX:
        return DeviceTypeIdType.kManufacturer
    if id in TEST_PREFIX and id in DEVICE_TYPE_ID_RANGE_SUFFIX:
        return DeviceTypeIdType.kTest
    return DeviceTypeIdType.kInvalid


def is_standard_device_type_id(id: int) -> bool:
    id_type = device_type_id_type(id)
    return id_type == DeviceTypeIdType.kStandard


def is_valid_device_type_id(id: int, allow_test=False) -> bool:
    id_type = device_type_id_type(id)
    valid = [DeviceTypeIdType.kStandard, DeviceTypeIdType.kManufacturer]
    if allow_test:
        valid.append(DeviceTypeIdType.kTest)
    return id_type in valid


def cluster_id_type(id: int) -> ClusterIdType:
    if id in STANDARD_PREFIX and id in CLUSTER_ID_STANDARD_RANGE_SUFFIX:
        return ClusterIdType.kStandard
    if id in MANUFACTURER_PREFIX and id in CLUSTER_ID_MANUFACTURER_RANGE_SUFFIX:
        return ClusterIdType.kManufacturer
    if id in TEST_PREFIX and id in CLUSTER_ID_MANUFACTURER_RANGE_SUFFIX:
        return ClusterIdType.kTest
    return ClusterIdType.kInvalid


def is_standard_cluster_id(id: int) -> bool:
    id_type = cluster_id_type(id)
    return id_type == ClusterIdType.kStandard


def is_valid_cluster_id(id: int, allow_test: bool = False) -> bool:
    id_type = cluster_id_type(id)
    valid = [ClusterIdType.kStandard, ClusterIdType.kManufacturer]
    if allow_test:
        valid.append(ClusterIdType.kTest)
    return id_type in valid


def attribute_id_type(id: int) -> AttributeIdType:
    if id in STANDARD_PREFIX and id in ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX:
        return AttributeIdType.kStandardNonGlobal
    if id in STANDARD_PREFIX and id in ATTRIBUTE_ID_GLOBAL_RANGE_SUFFIX:
        return AttributeIdType.kStandardGlobal
    if id in MANUFACTURER_PREFIX and id in ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX:
        return AttributeIdType.kManufacturer
    if id in TEST_PREFIX and id in ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX:
        return AttributeIdType.kTest
    return AttributeIdType.kInvalid


def is_valid_attribute_id(id: int, allow_test: bool = False):
    id_type = attribute_id_type(id)
    valid = [AttributeIdType.kStandardGlobal, AttributeIdType.kStandardNonGlobal, AttributeIdType.kManufacturer]
    if allow_test:
        valid.append(AttributeIdType.kTest)
    return id_type in valid


def is_standard_attribute_id(id: int):
    id_type = attribute_id_type(id)
    return id_type in [AttributeIdType.kStandardGlobal, AttributeIdType.kStandardNonGlobal]


def command_id_type(id: int) -> CommandIdType:
    if id in STANDARD_PREFIX and id in COMMAND_ID_GLOBAL_STANDARD_SUFFIX:
        return CommandIdType.kStandardGlobal
    if id in STANDARD_PREFIX and id in COMMAND_ID_NON_GLOBAL_SCOPED_SUFFIX:
        return CommandIdType.kScopedNonGlobal
    if id in MANUFACTURER_PREFIX and id in COMMAND_ID_SUFFIX:
        return CommandIdType.kManufacturer
    if id in TEST_PREFIX and id in COMMAND_ID_SUFFIX:
        return CommandIdType.kTest
    return CommandIdType.kInvalid


def is_standard_command_id(id: int):
    id_type = command_id_type(id)
    return id_type in [CommandIdType.kScopedNonGlobal, CommandIdType.kStandardGlobal]


def is_valid_command_id(id: int, allow_test: bool = False):
    id_type = command_id_type(id)
    valid = [CommandIdType.kStandardGlobal, CommandIdType.kScopedNonGlobal, CommandIdType.kManufacturer]
    if allow_test:
        valid.append(CommandIdType.kTest)
    return id_type in valid
