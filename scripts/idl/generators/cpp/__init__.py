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

import enum
import logging
import re

from idl.generators import CodeGenerator, GeneratorStorage
from idl.matter_idl_types import (Idl, ClusterSide, Field, Attribute, Cluster,
                                  FieldAttribute, Command, DataType, Struct)
from idl import matter_idl_types
from idl.generators.types import (ParseDataType, BasicString, BasicInteger, FundamentalType,
                                  IdlType, IdlItemType, IdlEnumType, IdlBitmapType, TypeLookupContext)
from typing import Union, List, Set


def camel_to_const(s):
    return re.sub("([a-z])([A-Z])", lambda y: y.group(1) + "_" + y.group(2), s).upper()


def create_lookup_context(idl: Idl, cluster: Cluster) -> TypeLookupContext:
    """
    A filter to mark a lookup context to be within a specific cluster.

    This is used to specify how structure/enum/other names are looked up.
    Generally one looks up within the specific cluster then if cluster does
    not contain a definition, we loop at global namespacing.
    """
    return TypeLookupContext(idl, cluster)


def get_field_info(definition: Field, cluster: Cluster, idl: Idl):
    context = create_lookup_context(idl, cluster)
    actual = ParseDataType(definition.data_type, context)

    orig = actual
    is_enum = type(actual) == IdlEnumType

    if type(actual) == IdlEnumType:
        actual = actual.base_type
    elif type(actual) == IdlBitmapType:
        actual = actual.base_type

    if type(actual) == BasicString:
        return 'OctetString', 'char', actual.max_length, \
            'ZCL_%s_ATTRIBUTE_TYPE' % orig.idl_name.upper()

    if type(actual) == BasicInteger:
        name = orig.idl_name.upper()
        if is_enum:
            name = actual.idl_name.upper()
        ty = "int%d_t" % actual.power_of_two_bits
        if not actual.is_signed:
            ty = "u" + ty
        return "PrimitiveType", ty, actual.byte_count, "ZCL_%s_ATTRIBUTE_TYPE" % name
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
        return 'StructType', actual.idl_name, 'sizeof(%s)' % actual.idl_name, \
            'ZCL_STRUCT_ATTRIBUTE_TYPE'
    logging.warn('Unknown type: %r' % actual)
    return None


def get_array_count(attr: Attribute):
    # TBD how to determine array lengths?
    return 1


def get_raw_size_and_type(attr: Attribute, cluster: Cluster, idl: Idl):
    container, cType, size, matterType = get_field_info(attr.definition, cluster, idl)
    if attr.definition.is_list:
        return 'ZCL_ARRAY_ATTRIBUTE_TYPE, {} * {} + 2'.format(size, get_array_count(attr))
    return '{}, {}'.format(matterType, size)


def get_field_type(definition: Field, cluster: Cluster, idl: Idl):
    container, cType, size, matterType = get_field_info(definition, cluster, idl)
    if container == 'StructType':
        return 'StructType<{}>'.format(cType)
    if container == 'OctetString':
        return '{}<{}, {}>'.format(container, size, matterType)
    return '{}<{}, {}, {}>'.format(container, cType, size, matterType)


def get_attr_type(attr: Attribute, cluster: Cluster, idl: Idl):
    decl = get_field_type(attr.definition, cluster, idl)
    if attr.definition.is_list:
        count = get_array_count(attr)
        return 'ArrayType<{}, {}>'.format(count, decl)
    return decl


def get_attr_init(attr: Attribute, cluster: Cluster, idl: Idl):
    if attr.definition.name == 'clusterRevision':
        return ' = ZCL_' + camel_to_const(cluster.name) + '_CLUSTER_REVISION'
    return ''


def get_attr_mask(attr: Attribute, cluster: Cluster, idl: Idl):
    masks = []
    if attr.is_writable:
        masks.append('ATTRIBUTE_MASK_WRITABLE')
    if masks:
        return ' | '.join(masks)
    return '0'


def get_dynamic_endpoint(idl: Idl):
    for ep in idl.endpoints:
        if ep.number == 2:
            return ep


def is_dynamic_cluster(cluster: Cluster, idl: Idl):
    ep = get_dynamic_endpoint(idl)
    if not ep:
        return True
    for c in ep.server_clusters:
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

        self.jinja_env.filters['getType'] = get_attr_type
        self.jinja_env.filters['getRawSizeAndType'] = get_raw_size_and_type
        self.jinja_env.filters['getField'] = get_field_type
        self.jinja_env.filters['getMask'] = get_attr_mask
        self.jinja_env.filters['getInit'] = get_attr_init
        self.jinja_env.filters['dynamicCluster'] = is_dynamic_cluster
        # constcase will transform ID to I_D which is not what we want
        # instead make the requirement a transition from lower to upper
        self.jinja_env.filters['cameltoconst'] = camel_to_const

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
