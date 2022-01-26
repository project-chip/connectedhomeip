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
from idl.matter_idl_types import Idl, ClusterSide, Field, Attribute, Cluster, FieldAttribute
from idl import matter_idl_types
from idl.generators.types import ParseDataType, BasicString, BasicInteger, FundamentalType, IdlType, IdlEnumType
from typing import Union, List
from stringcase import capitalcase

import logging


def FieldToGlobalName(field: Field, known_enum_types: List[matter_idl_types.Enum]) -> Union[str, None]:
    """Global names are used for generic callbacks shared across
    all clusters (e.g. for bool/float/uint32 and similar)
    """
    if field.is_list:
        return None  # lists are always specific per cluster

    if FieldAttribute.NULLABLE in field.attributes:
        return None

    actual = ParseDataType(field.data_type, known_enum_types)
    if type(actual) == IdlEnumType:
        actual = actual.base_type

    if type(actual) == BasicString:
        if actual.is_binary:
            return 'OctetString'
        else:
            return 'CharString'
    elif type(actual) == BasicInteger:
        # TODO: unclear why this, but tries to match zap:
        if actual.idl_name.lower() in ['vendor_id', 'fabric_idx']:
            return None

        if actual.is_signed:
            return "Int{}s".format(actual.power_of_two_bits)
        else:
            return "Int{}u".format(actual.power_of_two_bits)
    elif type(actual) == FundamentalType:
        if actual == FundamentalType.BOOL:
            return 'Boolean'
        elif actual == FundamentalType.FLOAT:
            return 'Float'
        elif actual == FundamentalType.DOUBLE:
            return 'Double'
        else:
            logging.warn('Unknown fundamental type: %r' % actual)

    return None


def CallbackName(attr: Attribute, cluster: Cluster, known_enum_types: List[matter_idl_types.Enum]) -> str:
    global_name = FieldToGlobalName(attr.definition, known_enum_types)

    if global_name:
        return 'CHIP{}AttributeCallback'.format(capitalcase(global_name))

    return 'CHIP{}{}AttributeCallback'.format(
        capitalcase(cluster.name),
        capitalcase(attr.definition.name)
    )


def attributesWithSupportedCallback(attrs, known_enum_types: List[matter_idl_types.Enum]):
    for attr in attrs:
        # Attributes will be generated for all types
        # except non-list structures
        if not attr.definition.is_list:
            underlying = ParseDataType(
                attr.definition.data_type, known_enum_types)
            if type(underlying) == IdlType:
                continue

        yield attr


class JavaGenerator(CodeGenerator):
    """
    Generation of java code for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        super().__init__(storage, idl)

        self.jinja_env.filters['attributesWithCallback'] = attributesWithSupportedCallback
        self.jinja_env.filters['callbackName'] = CallbackName

    def internal_render_all(self):
        known_enums = self.idl.enums[:]
        for cluster in self.idl.clusters:
            known_enums.extend(cluster.enums)

        # Every cluster has its own impl, to avoid
        # very large compilations (running out of RAM)
        for cluster in self.idl.clusters:
            if cluster.side != ClusterSide.CLIENT:
                continue

            self.internal_render_one_output(
                template_path="java/ChipClustersRead.jinja",
                output_file_name="jni/%sClient-ReadImpl.cpp" % cluster.name,
                vars={
                    'cluster': cluster,
                    'known_enums': known_enums,
                }
            )
