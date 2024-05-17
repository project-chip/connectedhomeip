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

from dataclasses import dataclass
from enum import IntEnum


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


@dataclass
class IdRange:
    min: int
    max: int

    def _contains(self, id: int):
        return id in range(self.min, self.max+1)


@dataclass
class PrefixIdRange(IdRange):
    def contains(self, id: int):
        return super()._contains(id >> 16)


class SuffixIdRange(IdRange):
    def contains(self, id: int):
        return super()._contains(id & 0xFFFF)


STANDARD_PREFIX = PrefixIdRange(0x0000, 0x0000)
MANUFACTURER_PREFIX = PrefixIdRange(0x0001, 0xFFF0)
TEST_PREFIX = PrefixIdRange(0xFFF1, 0xFFF4)

DEVICE_TYPE_ID_RANGE_SUFFIX = SuffixIdRange(0x0000, 0xBFFF)
CLUSTER_ID_STANDARD_RANGE_SUFFIX = SuffixIdRange(0x0000, 0x7FFF)
CLUSTER_ID_MANUFACTURER_RANGE_SUFFIX = SuffixIdRange(0xFC00, 0xFFFE)
ATTRIBUTE_ID_GLOBAL_RANGE_SUFFIX = SuffixIdRange(0xF000, 0xFFFE)
ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX = SuffixIdRange(0x0000, 0x4FFF)


def standard_device_type_range(id: int):
    return STANDARD_PREFIX.contains(id) and DEVICE_TYPE_ID_RANGE_SUFFIX.contains(id)


def manufacturer_device_type_range(id: int):
    return MANUFACTURER_PREFIX.contains(id) and DEVICE_TYPE_ID_RANGE_SUFFIX.contains(id)


def test_device_type_range(id: int):
    return TEST_PREFIX.contains(id) and DEVICE_TYPE_ID_RANGE_SUFFIX.contains(id)


def valid_device_type_range(id: int, allow_test: bool = False):
    ret = manufacturer_device_type_range(id) or standard_device_type_range(id)
    if allow_test:
        ret = ret or test_device_type_range(id)
    return ret


def standard_cluster_range(id: int):
    return STANDARD_PREFIX.contains(id) and CLUSTER_ID_STANDARD_RANGE_SUFFIX.contains(id)


def manufacturer_cluster_range(id: int):
    return MANUFACTURER_PREFIX.contains(id) and CLUSTER_ID_MANUFACTURER_RANGE_SUFFIX.contains(id)


def test_cluster_range(id: int):
    return TEST_PREFIX.contains(id) and CLUSTER_ID_MANUFACTURER_RANGE_SUFFIX.contains(id)


def valid_cluster_range(id: int, allow_test: bool = False):
    ret = standard_cluster_range(id) or manufacturer_cluster_range(id)
    if allow_test:
        ret = ret or test_cluster_range(id)
    return ret


def standard_non_global_attribute_range(id: int):
    return STANDARD_PREFIX.contains(id) and ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX.contains(id)


def standard_global_attribute_range(id: str):
    return STANDARD_PREFIX.contains(id) and ATTRIBUTE_ID_GLOBAL_RANGE_SUFFIX.contains(id)


def manufacturer_attribute_range(id: str):
    return MANUFACTURER_PREFIX.contains(id) and ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX.contains(id)


def test_attribute_range(id: str):
    return TEST_PREFIX.contains(id) and ATTRIBUTE_ID_NON_GLOBAL_RANGE_SUFFIX.contains(id)


def valid_attribute_range(id: str, allow_test: bool = False):
    ret = standard_global_attribute_range(id) or standard_non_global_attribute_range(id) or manufacturer_attribute_range(id)
    if allow_test:
        ret = ret or test_attribute_range(id)
    return ret
