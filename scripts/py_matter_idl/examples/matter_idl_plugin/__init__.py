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
from typing import List

import jinja2
from matter_idl import matter_idl_types
from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Attribute, Cluster, ClusterSide, Command, DataType, Field, Idl


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


def toUpperAcronym(s):
    """ Remove lower case letters and numbers from the given string"""
    return ''.join([i for i in s if i.isupper() or i.isnumeric()]).upper()


def toEnumEntryName(enumEntry, enumName):
    """ Create enum entry name by prepending that acronym of the enum name and converting to upper snake case """
    prefix = toUpperAcronym(enumName)
    if (enumEntry[0] == 'k'):
        enumEntry = enumEntry[1:]
    return prefix + '_' + toUpperSnakeCase(enumEntry)

def toProtobufType(zapType: str) -> str:
    """ Convert zap type to protobuf type """
    u32Types = ["uint32", "enum8", "enum16", "enum32", "bitmap8", "bitmap16", "bitmap32", "cluster_id", "attrib_id", "event_id", "command_id", "endpoint_no", "group_id", "devtype_id", "fabric_idx", "vendor_id", "status_code", "faulttype", "levelcontroloptions", "percent100ths", "percent"]
    u64Types = ["uint64", "enum64", "bitmap64", "node_id", "fabric_id", "int40u", "int48u", "int56u", "int64u"]
    i32Types = ["int32", "int8s", "int16s", "int24s", "int32s"]
    i64Types = ["int64", "int40s", "int48s", "int56s", "int64s"]
    floatTypes = ["float", "double"]
    stringTypes = ["char_string", "long_char_string"]
    bytesTypes = ["octet_string", "long_octet_string"]

    zapTypeLower = zapType.lower()
    if zapTypeLower in u32Types:
        return "uint32"
    if zapTypeLower in u64Types:
        return "uint64"
    if zapTypeLower in i32Types:
        return "int32"
    if zapTypeLower in i64Types:
        return "int64"
        
    match(zapTypeLower):
        case "boolean":
            return "bool"


        case "double":
            return "double"

    # If no match, return the original type name for the Struct, Enum, or Bitmap.
    return zapType


# Enum for encoding type into protobuf field tag for stateless translation.
class DataType:
    UINT = 1
    INT = 2
    BOOL = 3
    CHAR_STRING = 4
    OCT_STRING = 5
    STRUCT = 6
    FLOAT = 7
    DOUBLE = 8


def typeToTypeNum(s):
    match(s):
        case "uint32": return DataType.UINT
        case "uint64": return DataType.UINT
        case "int32": return DataType.INT
        case "int64": return DataType.INT
        case "bool": return DataType.BOOL
        case "string": return DataType.CHAR_STRING
        case "bytes": return DataType.OCT_STRING
        case "float": return DataType.FLOAT
        case "double": return DataType.DOUBLE

    return DataType.STRUCT


def commandArgs(command: Command, cluster: Cluster):
    # Find command.input_arg in cluster.structs
    for struct in cluster.structs:
        if struct.name == command.input_param:
            return struct.fields

    return []


def commandResponseArgs(command: Command, cluster: Cluster):
    # Find command.input_arg in cluster.structs
    for struct in cluster.structs:
        if struct.name == command.output_param:
            return struct.fields

    return []


def toTypeEncodedTag(tag, typeNum):
    """ Encode the type into the upper range of the protobuf field tag for stateless translation. """
    tag = (int(typeNum) << 19) | int(tag)
    return tag


def toFieldType(field: Field):
    prefix = ""
    protobufType = toProtobufType(field.data_type.name)
    if field.is_list:
        prefix = "repeated " + prefix
    elif field.is_optional:
        prefix = "optional " + prefix
    return prefix + protobufType


def toFieldTag(field: Field):
    protobufType = toProtobufType(field.data_type.name)
    typeNum = typeToTypeNum(protobufType)
    tag = toTypeEncodedTag(field.code, typeNum)
    return tag


def toFieldComment(field: Field):
    protobufType = toProtobufType(field.data_type.name)
    typeNum = typeToTypeNum(protobufType)
    tagComment = "/** %s Type: %d IsList: %d FieldId: %d */" % (
        field.data_type.name, typeNum, field.is_list, field.code)
    return tagComment


class CustomGenerator(CodeGenerator):
    """
    Example of a custom generator.  Outputs protobuf representation of MAtter clusters.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        """
        Inintialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl)

        # Override the template path to use local templates within this plugin directory
        self.jinja_env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(
                searchpath=os.path.dirname(__file__)),
            keep_trailing_newline=True)

        # String helpers
        self.jinja_env.filters['toLowerSnakeCase'] = toLowerSnakeCase
        self.jinja_env.filters['toUpperSnakeCase'] = toUpperSnakeCase

        # Type helpers
        self.jinja_env.filters['toEnumEntryName'] = toEnumEntryName
        self.jinja_env.filters['toProtobufType'] = toProtobufType
        self.jinja_env.filters['toTypeEncodedTag'] = toTypeEncodedTag

        # Tag helpers
        self.jinja_env.filters['toFieldTag'] = toFieldTag
        self.jinja_env.filters['toFieldType'] = toFieldType
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
            if cluster.side != ClusterSide.CLIENT:
                continue

            filename = "proto/%s_cluster.proto" % toLowerSnakeCase(cluster.name)

            # Header containing a macro to initialize all cluster plugins
            self.internal_render_one_output(
                template_path="./matter_cluster_proto.jinja",
                output_file_name=filename,
                vars={
                    'cluster': cluster,
                }
            )
