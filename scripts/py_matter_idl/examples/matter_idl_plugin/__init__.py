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

import os
from typing import List

import jinja2
from matter_idl import matter_idl_types
from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Attribute, Cluster, ClusterSide, Command, DataType, Field, Idl


def toUpperSnakeCase(s):
    """ Convert camelCaseString to UPPER_SNAKE_CASE """
    lastLower = False
    letters = []
    for c in s:
        #if c == '_': next
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
    """ Convert camelCaseString to lower_snake_case whenever a transition from lower to upper case is detected """
    return toUpperSnakeCase(s).lower()


def toUpperAcronym(s):
    """ Remove lower case letters and numbers from the given string"""
    return ''.join([i for i in s if i.isupper() or i.isnumeric()]).upper()


def toEnumConstant(enumEntry, enumName):
    """ Create enum entry name by prepending that acronym of the enum name and converting to upper snake case """
    prefix = toUpperAcronym(enumName)
    if (enumEntry[0] == 'k'):
        enumEntry = enumEntry[1:]
    return prefix + '_' + toUpperSnakeCase(enumEntry)


def toEnumName(s):
    """ Add En umto the end of the string if it is not already there """
    return s + "Enum" if not s.endswith("Enum") else s


def toStructName(s):
    """ Add Struct to the end of the string if it is not already there """
    return s + "Struct" if not s.endswith("Struct") else s


def toEventName(s):
    """ Add Event to the end of the string if it is not already there """
    return s + "Event" if not s.endswith("Event") else s


def toProtobufType(s):
    """ Convert zap type to protobuf type """

    match(s.lower()):
        case "boolean":
            return "bool"

        case "enum8":
            return "uint32"
        case "enum16":
            return "uint32"
        case "enum32":
            return "uint32"
        case "bitmap8":
            return "uint32"
        case "bitmap16":
            return "uint32"
        case "bitmap32":
            return "uint32"
        case "cluster_id":
            return "uint32"
        case "attrib_id":
            return "uint32"
        case "event_id":
            return "uint32"
        case "command_id":
            return "uint32"
        case "endpoint_no":
            return "uint32"
        case "group_id":
            return "uint32"
        case "devtype_id":
            return "uint32"
        case "fabric_idx":
            return "uint32"
        case "vendor_id":
            return "uint32"
        case "status_code":
            return "uint32"
        case "faulttype":
            return "uint32"
        case 'epoch_s':
            return "uint32"
        case "levelcontroloptions":
            return "uint32"
        case "percent100ths":
            return "uint32"
        case "percent":
            return "uint32"

        case "int8u":
            return "uint32"
        case "int16u":
            return "uint32"
        case "int24u":
            return "uint32"
        case "int32u":
            return "uint32"

        case "enum64":
            return "uint64"
        case "bitmap64":
            return "uint64"
        case "node_id":
            return "uint64"
        case "fabric_id":
            return "uint64"
        case "epoch_us":
            return "uint64"
        case "int40u":
            return "uint64"
        case "int48u":
            return "uint64"
        case "int56u":
            return "uint64"
        case "int64u":
            return "uint64"

        case "int8s":
            return "int32"
        case "int16s":
            return "int32"
        case "int24s":
            return "int32"
        case "int32s":
            return "int32"

        case "int40s":
            return "int64"
        case "int48s":
            return "int64"
        case "int56s":
            return "int64"
        case "int64s":
            return "int64"

        case "long_char_string":
            return "string"
        case "char_string":
            return "string"

        case "long_octet_string":
            return "bytes"
        case "octet_string":
            return "bytes"

        case "single":
            return "float"
        case "float":
            return "float"
        case "double":
            return "double"

    return s


# Python enum of types
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
    tag = (int(typeNum) << 19) | int(tag)
    #tag = tag + 1
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
    Generation of cpp code for application implementation for matter.
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
        self.jinja_env.filters['toStructName'] = toStructName
        self.jinja_env.filters['toEventName'] = toEventName
        self.jinja_env.filters['toEnumName'] = toEnumName
        self.jinja_env.filters['toEnumConstant'] = toEnumConstant
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

            filename = "proto/%s_trait.proto" % toLowerSnakeCase(cluster.name)

            # Header containing a macro to initialize all cluster plugins
            self.internal_render_one_output(
                template_path="./matter_cluster_proto.jinja",
                output_file_name=filename,
                vars={
                    'cluster': cluster,
                }
            )
