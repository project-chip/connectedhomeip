# Copyright (c) 2023 Project CHIP Authors
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
import re
from dataclasses import dataclass
from typing import Optional
from xml.sax.xmlreader import AttributesImpl

from matter_idl.generators.type_definitions import GetDataTypeSizeInBits, IsSignedDataType
from matter_idl.matter_idl_types import AccessPrivilege, Attribute, Command, ConstantEntry, DataType, Event, EventPriority, Field

LOGGER = logging.getLogger('data-model-xml-data-parsing')


@dataclass
class ParsedType:
    name: str
    is_list: bool = False


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


def ParseOptionalInt(value: str) -> Optional[int]:
    """Parses numbers as long as they are in an expected format of numbers.

       "1" parses to 1
       "0x12" parses to 18
       "Min" parses to None
    """
    if re.match("^-?((0x[0-9a-fA-F]*)|([0-9]*))$", value):
        return ParseInt(value)

    return None


_TYPE_REMAP = {
    # unsigned
    "uint8": "int8u",
    "uint16": "int16u",
    "uint24": "int24u",
    "uint32": "int32u",
    "uint48": "int48u",
    "uint52": "int52u",
    "uint64": "int64u",
    # signed (map to what zapxml/matter currently has)
    "int8": "int8s",
    "int16": "int16s",
    "int24": "int24s",
    "int32": "int32s",
    "int48": "int48s",
    "int52": "int52s",
    "int64": "int64s",
    # other
    "bool": "boolean",
    "string": "char_string",
    "octets": "octet_string",
}


def NormalizeDataType(t: str) -> str:
    """Convert data model xml types into matter idl types."""
    return _TYPE_REMAP.get(t.lower(), t.replace("-", "_"))


# Handle oddities in current data model XML schema for nicer diffs
_REF_NAME_MAPPING = {
    "<<ref_DataTypeEndpointNumber>>": "endpoint_no",
    "<<ref_DataTypeEpochUs>>": "epoch_us",
    "<<ref_DataTypeNodeId>>": "node_id",
    "<<ref_DataTypeOctstr>>": "octet_string",
    "<<ref_DataTypeString>>": "char_string",
    "<<ref_DataTypeVendorId>>": "vendor_id",
    "<<ref_FabricIdx>>": "fabric_idx",
}


# Handle odd casing and naming
_CASE_RENAMES_MAPPING = {
    "power_mW": "power_mw",
    "energy_mWh": "energy_mwh"
}


def ParseType(t: str) -> ParsedType:
    """Parse a data type entry.

    Specifically parses a name like "list[Foo Type]".
    """

    # very rough matcher ...
    is_list = False
    if t.startswith("list[") and t.endswith("]"):
        is_list = True
        t = t[5:-1]
    elif t.startswith("<<ref_DataTypeList>>[") and t.endswith("]"):
        is_list = True
        t = t[21:-1]

    if t.endswith(" Type"):
        t = t[:-5]

    if t in _REF_NAME_MAPPING:
        t = _REF_NAME_MAPPING[t]

    if t in _CASE_RENAMES_MAPPING:
        t = _CASE_RENAMES_MAPPING[t]

    return ParsedType(name=NormalizeDataType(t), is_list=is_list)


def NormalizeName(name: str) -> str:
    """Convert a free form name from the spec into a programming language
       name that is appropriate for matter IDL.
    """

    # Trim human name separators
    for separator in " /-":
        name = name.replace(separator, '_')
    while '__' in name:
        name = name.replace('__', '_')

    # NOTE: zapt generators for IDL files use a construct of the form
    #       `{{asUpperCamelCase name preserveAcronyms=true}}`
    #       and it is somewhat unclear what preserveAcronyms will do.
    #
    #      Current assumption is that spec already has acronyms set in
    #      the correct place and at least for some basic tests this method
    #      generates good names
    #
    #      If any acronyms seem off in naming at some point, more logic may
    #      be needed here.

    # At this point, we remove all _ and make sure _ is followed by an uppercase
    while name.endswith('_'):
        name = name[:-1]

    while '_' in name:
        idx = name.find('_')
        name = name[:idx] + name[idx+1].upper() + name[idx+2:]

    return name


def FieldName(input_name: str) -> str:
    """Normalized name with the first letter lowercase. """
    name = NormalizeName(input_name)

    # Some exception handling for nicer diffs
    if name == "ID":
        return "id"

    # If the name starts with a all-uppercase thing, keep it that
    # way. This is typical for "NOC", "IPK", "CSR" and such
    if len(input_name) > 1:
        if input_name[0].isupper() and input_name[1].isupper():
            return name

    return name[0].lower() + name[1:]


def AttributesToField(attrs: AttributesImpl) -> Field:
    assert "name" in attrs
    assert "id" in attrs

    if "type" in attrs:
        attr_type = NormalizeDataType(attrs["type"])
    else:
        # TODO: Generally we should not have this, however current implementation
        #       for derived clusters for example want to add things (like conformance
        #       specifically) WITHOUT re-stating things like types
        #
        # https://github.com/csa-data-model/projects/issues/365
        LOGGER.error(f"Attribute {attrs['name']} has no type")
        attr_type = "sint32"
    t = ParseType(attr_type)

    return Field(
        name=FieldName(attrs["name"]),
        code=ParseInt(attrs["id"]),
        is_list=t.is_list,
        data_type=DataType(name=t.name),
    )


def AttributesToBitFieldConstantEntry(attrs: AttributesImpl) -> ConstantEntry:
    """Creates a constant entry appropriate for bitmaps.
    """
    assert "name" in attrs

    if 'bit' not in attrs:
        # TODO: multi-bit fields not supported in XML currently. Be lenient here to have some
        #       diff
        # Issue: https://github.com/csa-data-model/projects/issues/347

        LOGGER.error(
            f"Constant {attrs['name']} has no bit value (may be multibit)")
        return ConstantEntry(name="k" + NormalizeName(attrs["name"]), code=0)

    assert "bit" in attrs

    return ConstantEntry(name="k" + NormalizeName(attrs["name"]), code=1 << ParseInt(attrs["bit"]))


def AttributesToAttribute(attrs: AttributesImpl) -> Attribute:
    assert "name" in attrs
    assert "id" in attrs

    if "type" in attrs:
        attr_type = NormalizeDataType(attrs["type"])
    else:
        # TODO: we should NOT have this, however we are now lenient
        # to bad input data
        LOGGER.error(f"Attribute {attrs['name']} has no type")
        attr_type = "sint32"

    t = ParseType(attr_type)

    return Attribute(
        definition=Field(
            code=ParseInt(attrs["id"]),
            name=FieldName(attrs["name"]),
            is_list=t.is_list,
            data_type=DataType(name=t.name),
        )
    )


def AttributesToEvent(attrs: AttributesImpl) -> Event:
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


def AttributesToCommand(attrs: AttributesImpl) -> Command:
    assert "id" in attrs
    assert "name" in attrs

    if "response" not in attrs:
        LOGGER.warn(f"Command {attrs['name']} has no response set.")
        # Matter IDL has no concept of "no response sent"
        # Example is DoorLock::"Operating Event Notification"
        #
        # However that is not in the impl in general
        # it is unclear what to do here (and what "NOT" is as conformance)

        output_param = "DefaultSuccess"
    else:
        output_param = NormalizeName(attrs["response"])
        if output_param == "Y":
            output_param = "DefaultSuccess"  # IDL name for no specific struct

    return Command(
        name=NormalizeName(attrs["name"]),
        code=ParseInt(attrs["id"]),
        input_param=None,  # not specified YET
        output_param=output_param
    )


def ApplyConstraint(attrs, field: Field):
    """
    Handles constraints according to Matter IDL formats.

    Specifically it does NOT handle min/max values as current IDL
    format does not support having such values defined.
    """
    assert "type" in attrs

    constraint_type = attrs["type"]

    if constraint_type == "allowed":
        pass  # unsure what to do allowed
    elif constraint_type == "desc":
        pass  # free-form description
    elif constraint_type in {"countBetween", "maxCount"}:
        pass  # cannot implement count
    elif constraint_type == "min":
        # field.data_type.min_value = ParseOptionalInt(attrs["value"])
        pass
    elif constraint_type == "max":
        # field.data_type.max_value = ParseOptionalInt(attrs["value"])
        pass
    elif constraint_type == "between":
        # TODO: examples existing in the parsed data which are NOT
        #       handled:
        #         - from="-2.5°C" to="2.5°C"
        #         - from="0%" to="100%"
        # field.data_type.min_value = ParseOptionalInt(attrs["from"])
        # field.data_type.max_value = ParseOptionalInt(attrs["to"])
        pass
    elif constraint_type == "maxLength":
        field.data_type.max_length = ParseOptionalInt(attrs["value"])
    elif constraint_type == "minLength":
        field.data_type.min_length = ParseOptionalInt(attrs["value"])
    elif constraint_type == "lengthBetween":
        field.data_type.min_length = ParseOptionalInt(attrs["from"])
        field.data_type.max_length = ParseOptionalInt(attrs["to"])
    else:
        logging.error(f"UNKNOWN constraint type {constraint_type}")
