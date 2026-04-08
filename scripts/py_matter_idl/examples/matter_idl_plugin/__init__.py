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

import os

from matter.idl.generators import CodeGenerator
from matter.idl.generators.storage import GeneratorStorage
from matter.idl.generators.type_definitions import GetDataTypeSizeInBits, IsSignedDataType
from matter.idl.matter_idl_types import Cluster, Command, DataType, Field, Idl

_PRIMITIVE_PROTOBUF_TYPES = {
    "boolean": "bool",
    "float": "float",
    "single": "float",
    "double": "double",
    "char_string": "string",
    "long_char_string": "string",
    "octet_string": "bytes",
    "long_octet_string": "bytes",
    "uint32": "uint32",
    "uint64": "uint64",
    "int32": "int32",
    "int64": "int64",
}


def toUpperSnakeCase(s):
    """ Convert camelCaseString to UPPER_SNAKE_CASE with proper handling of acronyms and numerals. """
    lastLower = False
    letters = []
    for c in s:
        if c.isupper():
            if lastLower:
                letters.append('_')
            letters.append(c)
            lastLower = False
        else:
            letters.append(c.upper())
            lastLower = True

    return ''.join(letters)


def toLowerSnakeCase(s):
    """ Convert camelCaseString to lower_snake_case with proper handling of acronyms and numerals. """
    return toUpperSnakeCase(s).lower()


def toEnumEntryName(enumEntry, enumName):
    """Create enum entry name with a stable cluster-local unique prefix."""
    prefix = toUpperSnakeCase(enumName)
    if (enumEntry[0] == 'k'):
        enumEntry = enumEntry[1:]
    return prefix + '_' + toUpperSnakeCase(enumEntry)


def toProtobufType(zapType: str) -> str:
    """ Convert zap type to protobuf type """
    zapTypeLower = zapType.lower()
    primitiveType = _PRIMITIVE_PROTOBUF_TYPES.get(zapTypeLower)
    if primitiveType is not None:
        return primitiveType

    # Reuse Matter IDL's canonical type-size table so derived integer aliases
    # (e.g. epoch_s, elapsed_s, int16u, status, temperature) map correctly.
    typeInfo = DataType(name=zapTypeLower)
    sizeBits = GetDataTypeSizeInBits(typeInfo)
    if sizeBits is not None:
        isSigned = IsSignedDataType(typeInfo)
        if isSigned:
            return "int32" if sizeBits <= 32 else "int64"
        return "uint32" if sizeBits <= 32 else "uint64"

    # If no match, return the original type name for the Struct, Enum, or Bitmap.
    return zapType


# Enum for encoding the type information into protobuf field tag for stateless translation.
# These values encoded to the upper range of the protobuf field tag.
class EncodingDataType:
    UINT = 1
    INT = 2
    BOOL = 3
    CHAR_STRING = 4
    OCT_STRING = 5
    STRUCT = 6
    FLOAT = 7
    DOUBLE = 8

    @staticmethod
    def fromType(protobufType: str):
        if protobufType == "uint32":
            return EncodingDataType.UINT
        if protobufType == "uint64":
            return EncodingDataType.UINT
        if protobufType == "int32":
            return EncodingDataType.INT
        if protobufType == "int64":
            return EncodingDataType.INT
        if protobufType == "bool":
            return EncodingDataType.BOOL
        if protobufType == "string":
            return EncodingDataType.CHAR_STRING
        if protobufType == "bytes":
            return EncodingDataType.OCT_STRING
        if protobufType == "float":
            return EncodingDataType.FLOAT
        if protobufType == "double":
            return EncodingDataType.DOUBLE

        # If not a primitive type, it is a named type; assume it is a Struct.
        # NOTE: the actual type may be an Enum or Bitmap.
        return EncodingDataType.STRUCT


def commandArgs(command: Command, cluster: Cluster):
    """Return the list of fields for the command request for the given command and cluster."""
    for struct in cluster.structs:
        if struct.name == command.input_param:
            return struct.fields

    # If the command has no input parameters, just return an empty list.
    return []


def commandResponseArgs(command: Command, cluster: Cluster):
    """Return the list of fields for the command response for the given command and cluster."""
    for struct in cluster.structs:
        if struct.name == command.output_param:
            return struct.fields

    return []


def toEncodedTag(tag, typeNum: EncodingDataType):
    """ Return the final encoded tag from the given field number and field encoded data type.
        The Matter field type information is encoded into the upper range of the protobuf field
        tag for stateless translation to Matter TLV. """
    return (int(typeNum) << 19) | int(tag)


def toProtobufFullType(field: Field):
    """Return the full protobuf type for the given field, including repeated and optional specifiers."""
    prefix = ""
    protobufType = toProtobufType(field.data_type.name)
    if field.is_list:
        prefix = "repeated " + prefix
    elif field.is_optional:
        prefix = "optional " + prefix
    return prefix + protobufType


def toFieldTag(field: Field):
    protobufType = toProtobufType(field.data_type.name)
    typeNum = EncodingDataType.fromType(protobufType)
    return toEncodedTag(field.code, typeNum)


def toFieldComment(field: Field):
    protobufType = toProtobufType(field.data_type.name)
    typeNum = EncodingDataType.fromType(protobufType)
    return "/** %s Type: %d IsList: %d FieldId: %d */" % (
        field.data_type.name, typeNum, field.is_list, field.code)


class CustomGenerator(CodeGenerator):
    """
    Example of a custom generator.  Outputs protobuf representation of Matter clusters.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, **kargs):
        """
        Initialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl, fs_loader_searchpath=os.path.dirname(__file__))

        if 'package' not in kargs:
            raise Exception(
                'Please provide a "--option package:<name>" argument')
        self.package = kargs['package']

        # String helpers
        self.jinja_env.filters['toLowerSnakeCase'] = toLowerSnakeCase
        self.jinja_env.filters['toUpperSnakeCase'] = toUpperSnakeCase

        # Type helpers
        self.jinja_env.filters['toEnumEntryName'] = toEnumEntryName
        self.jinja_env.filters['toProtobufType'] = toProtobufType
        self.jinja_env.filters['toEncodedTag'] = toEncodedTag

        # Tag helpers
        self.jinja_env.filters['toFieldTag'] = toFieldTag
        self.jinja_env.filters['toProtobufFullType'] = toProtobufFullType
        self.jinja_env.filters['toFieldComment'] = toFieldComment

        # Command helpers
        self.jinja_env.filters['commandArgs'] = commandArgs
        self.jinja_env.filters['commandResponseArgs'] = commandResponseArgs

    def internal_render_all(self):
        """
        Renders the given custom template to the given output filename.
        """

        # Every cluster has its own impl, to avoid
        # very large compilations (running out of RAM)
        for cluster in self.idl.clusters:
            filename = "proto/%s_cluster.proto" % toLowerSnakeCase(
                cluster.name)

            # Header containing a macro to initialize all cluster plugins
            self.internal_render_one_output(
                template_path="matter_cluster_proto.jinja",
                output_file_name=filename,
                vars={
                    'cluster': cluster,
                    'package': self.package,
                }
            )
