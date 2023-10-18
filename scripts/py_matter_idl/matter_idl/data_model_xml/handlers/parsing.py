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
from typing import Optional

from matter_idl.generators.types import GetDataTypeSizeInBits, IsSignedDataType
from matter_idl.matter_idl_types import (AccessPrivilege, Attribute, AttributeQuality, ConstantEntry, DataType, Event,
                                         EventPriority, Field, FieldQuality)

LOGGER = logging.getLogger('data-model-xml-data-parsing')


def ParseInt(value: str, data_type: Optional[DataType] = None) -> int:
    """
    Convert a string that is a known integer into an actual number.

    Supports decimal or hex values prefixed with '0x'
    """
    if value.startswith('0x'):
        parsed = int(value[2:], 16)
        if data_type and IsSignedDataType(data_type):
            bits = GetDataTypeSizeInBits(data_type)
            assert (bits)  # size MUST be known
            if parsed & (1 << (bits - 1)):
                parsed -= 1 << bits
        return parsed
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


def AttributesToField(attrs) -> Field:
    assert ("name" in attrs)
    assert ("id" in attrs)
    assert ("type" in attrs)

    return Field(
        name=NormalizeName(attrs["name"]),
        code=ParseInt(attrs["id"]),
        data_type=DataType(name=attrs["type"])
    )


def AttributesToBitFieldConstantEntry(attrs) -> ConstantEntry:
    """Creates a constant entry appropriate for bitmaps.
    """
    assert ("name" in attrs)
    assert ("bit" in attrs)

    return ConstantEntry(name="k" + NormalizeName(attrs["name"]), code=1 << ParseInt(attrs["bit"]))


def AttributesToAttribute(attrs) -> Attribute:
    assert "name" in attrs
    assert "id" in attrs

    if "type" in attrs:
        attr_type = attrs["type"]
    else:
        # TODO: we should NOT have this, however we are now lenient
        # to bad input data
        LOGGER.error(f"Attribute {attrs['name']} has no type")
        attr_type = "sint32"

    return Attribute(
        definition=Field(
            code=ParseInt(attrs["id"]),
            name=NormalizeName(attrs["name"]),
            data_type=DataType(name=attr_type),
        )
    )


def AttributesToEvent(attrs) -> Event:
    assert "name" in attrs
    assert "id" in attrs
    assert "priority" in attrs

    if attrs["priority"] == "critical":
        priority = EventPriority.CRITICAL
    elif attrs["priority"] == "info":
        priority = EventPriority.INFO
    elif attrs["priority"] == "debug":
        priority = EventPriority.DEBUG
    elif attrs["priority"] == "desc":
        LOGGER.warn("Found an event with 'desc' priority: %s" %
                    [item for item in attrs.items()])
        priority = EventPriority.CRITICAL
    else:
        raise Exception("UNKNOWN event priority: %r" % attrs["priority"])

    return Event(
        name=NormalizeName(attrs["name"]),
        code=ParseInt(attrs["id"]),
        priority=priority,
        fields=[])


def StringToAccessPrivilege(value: str) -> AccessPrivilege:
    if value == "view":
        return AccessPrivilege.VIEW
    elif value == "operate":
        return AccessPrivilege.OPERATE
    elif value == "manage":
        return AccessPrivilege.MANAGE
    elif value == "admin":
        return AccessPrivilege.ADMINISTER
    else:
        raise Exception("UNKNOWN privilege level: %r" % value)
