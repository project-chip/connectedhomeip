#!/usr/bin/env python
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

from idl.generators import CodeGenerator, GeneratorStorage
from idl.matter_idl_types import Idl, ClusterSide, Field, Attribute, Cluster, FieldAttribute, Command, DataType, Struct
from idl import matter_idl_types
from idl.generators.types import ParseDataType, BasicString, BasicInteger, FundamentalType, IdlType, IdlItemType, IdlEnumType, IdlBitmapType, TypeLookupContext
from typing import Union, List, Set

import enum
import logging
import re

def CamelToConst(Str):
    return re.sub("([a-z])([A-Z])", lambda y: y.group(1) + "_" + y.group(2), Str).upper()


def CreateLookupContext(idl: Idl, cluster: Cluster) -> TypeLookupContext:
    """
    A filter to mark a lookup context to be within a specific cluster.

    This is used to specify how structure/enum/other names are looked up.
    Generally one looks up within the specific cluster then if cluster does
    not contain a definition, we loop at global namespacing.
    """
    return TypeLookupContext(idl, cluster)

def GetFieldInfo(definition: Field, cluster: Cluster, idl: Idl, list):
    context = CreateLookupContext(idl, cluster)
    actual = ParseDataType(definition.data_type, context)

    orig = actual
    isEnum = type(actual) == IdlEnumType

    if type(actual) == IdlEnumType:
        actual = actual.base_type
    elif type(actual) == IdlBitmapType:
        actual = actual.base_type

    if type(actual) == BasicString:
        return 'OctetString', 'char', actual.max_length, 'ZCL_%s_ATTRIBUTE_TYPE' % orig.idl_name.upper()

    if type(actual) == BasicInteger:
        name = orig.idl_name.upper()
        if isEnum:
            name = actual.idl_name.upper()
        if actual.is_signed:
            return "PrimitiveType", "int%d_t" % actual.power_of_two_bits, actual.byte_count, "ZCL_%s_ATTRIBUTE_TYPE" % name
        return "PrimitiveType", "uint%d_t" % actual.power_of_two_bits, actual.byte_count, "ZCL_%s_ATTRIBUTE_TYPE" % name
    if type(actual) == FundamentalType:
        if actual == FundamentalType.BOOL:
            return "PrimitiveType", "bool", 1, "ZCL_BOOLEAN_ATTRIBUTE_TYPE"
        if actual == FundamentalType.FLOAT:
            return "PrimitiveType", "float", 4, "ZCL_SINGLE_ATTRIBUTE_TYPE"
        if actual == FundamentalType.DOUBLE:
            return "PrimitiveType", "double", 8, "ZCL_DOUBLE_ATTRIBUTE_TYPE"
        logging.warn('Unknown fundamental type: %r' % actual)
        return None
    if type(actual) == IdlType:
        return 'StructType', actual.idl_name, 'sizeof(%s)' % actual.idl_name, 'ZCL_STRUCT_ATTRIBUTE_TYPE'
    logging.warn('Unknown type: %r' % actual)

def GetRawSizeAndType(attr: Attribute, cluster: Cluster, idl: Idl, list=False):
    container, cType, size, matterType = GetFieldInfo(attr.definition, cluster, idl, list)
    return '{}, {}'.format(matterType, size)

def GetFieldType(definition: Field, cluster: Cluster, idl: Idl, list=False):
    container, cType, size, matterType = GetFieldInfo(definition, cluster, idl, list)
    if container == 'StructType':
        return 'StructType<{}>'.format(cType)
    if container == 'OctetString':
        return '{}<{}, {}>'.format(container, size, matterType)
    return '{}<{}, {}, {}>'.format(container, cType, size, matterType)

def GetAttrType(attr: Attribute, cluster: Cluster, idl: Idl):
    return GetFieldType(attr.definition, cluster, idl, attr.definition.is_list)

def GetAttrInit(attr: Attribute, cluster: Cluster, idl: Idl):
    if attr.definition.name == 'clusterRevision':
        return ' = ZCL_' + CamelToConst(cluster.name) + '_CLUSTER_REVISION'
    return ''

def GetAttrMask(attr: Attribute, cluster: Cluster, idl: Idl):
    masks = []
    if attr.is_writable:
        masks.append('ATTRIBUTE_MASK_WRITABLE')
    if masks:
        return ' | '.join(masks)
    return '0'

def GetDynamicEndpoint(idl: Idl):
    for ep in idl.endpoints:
        if ep.number == 1:
            return ep

def IsDynamicCluster(cluster: Cluster, idl: Idl):
    for c in GetDynamicEndpoint(idl).server_clusters:
        if cluster.name == c.name:
            return True
    return False

class CppGenerator(CodeGenerator):
    """
    Generation of cpp code for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        """
        Inintialization is specific for cpp generation and will add
        filters as required by the cpp .jinja templates to function.
        """
        super().__init__(storage, idl)

        self.jinja_env.filters['getType'] = GetAttrType
        self.jinja_env.filters['getRawSizeAndType'] = GetRawSizeAndType
        self.jinja_env.filters['getField'] = GetFieldType
        self.jinja_env.filters['getMask'] = GetAttrMask
        self.jinja_env.filters['getInit'] = GetAttrInit
        self.jinja_env.filters['dynamicCluster'] = IsDynamicCluster
        # constcase will transform ID to I_D which is not what we want
        # instead make the requirement a transition from lower to upper
        self.jinja_env.filters['cameltoconst'] = CamelToConst

    def internal_render_all(self):
        """
        Renders C++
        """

        self.internal_render_one_output(
            template_path="cpp/BridgeClustersCpp.jinja",
            output_file_name="cpp/BridgeClustersImpl.h",
            vars={
                'clusters': self.idl.clusters,
                'idl': self.idl,
                'structs': self.idl.structs,
            }
        )
