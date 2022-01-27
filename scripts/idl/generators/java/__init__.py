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
from idl.matter_idl_types import Idl, ClusterSide, Field, Attribute, Cluster, FieldAttribute, Command, DataType
from idl import matter_idl_types
from idl.generators.types import ParseDataType, BasicString, BasicInteger, FundamentalType, IdlType, IdlEnumType
from typing import Union, List, Set
from stringcase import capitalcase

import enum
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

def CommandCallbackName(command: Command, cluster: Cluster):
  if command.output_param.lower() == 'defaultsuccess':
    return 'DefaultSuccess'
  return '{}Cluster{}'.format(cluster.name, command.output_param)


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

def ClientClustersOnly(clusters: List[Cluster]):
    for cluster in clusters:
        if cluster.side == ClusterSide.CLIENT:
            yield cluster

def NamedFilter(choices: List, name: str):
    print("%r %r\n" % (choices, name))
    for choice in choices:
        if choice.name == name:
            return choice
    raise Exception("No item named %s in %r" % (name, choices))

def ToBoxedJavaType(field: Field):
  if field.is_nullable:
    return 'jobject'
  elif field.data_type.name.lower() == 'octet_string':
    return 'jbyteArray'
  elif field.data_type.name.lower() == 'char_string':
    return 'jstring'
  else:
    return 'jobject'

def LowercaseFirst(name: str):
    if len(name) > 1 and name[1].lower() != name[1]:
        # Odd workaround: PAKEVerifier should not become pAKEVerifier
        return name
    return name[0].lower() + name[1:]

class LookupContext:
    """Ability to lookup enumerations and structure types"""

    def __init__(self, idl: Idl, cluster: Cluster):
        self.idl = idl
        self.cluster = cluster


    def all_enums(self):
        """All enumerations, ordered by lookup prioroty."""
        for e in self.cluster.enums:
            yield e
        for e in self.idl.enums:
            yield e

    def all_structs(self):
        """All structs, ordered by lookup prioroty."""
        for e in self.cluster.structs:
            yield e
        for e in self.idl.structs:
            yield e

    def is_enum_type(self, name: str):
        return any(map(lambda e: e.name == name, self.all_enums()))

    def is_struct_type(self, name: str):
        return any(map(lambda s: s.name == name, self.all_structs()))

def CreateContext(idl: Idl, cluster: Cluster):
    return LookupContext(idl, cluster)


class EncodableValueAttr(enum.Enum):
    LIST     = enum.auto()
    NULLABLE = enum.auto()
    OPTIONAL = enum.auto()

class EncodableValue:
    """
    Contains helpers for encoding values, specifically lookups
    for optionality, lists and recursive data type lookups within
    the IDL and cluster
    """
    def __init__(self, context: LookupContext, data_type: DataType, attrs: Set[EncodableValueAttr]):
        self.context = context  # 
        self.data_type = data_type
        self.attrs = attrs

    @property
    def is_nullable(self):
        return EncodableValueAttr.NULLABLE in self.attrs

    @property
    def is_optional(self):
        return EncodableValueAttr.OPTIONAL in self.attrs
    
    @property
    def is_list(self):
        return EncodableValueAttr.LIST in self.attrs

    @property
    def is_octet_string(self):
        return self.data_type.name.lower() == 'octet_string'

    @property
    def is_char_string(self):
        return self.data_type.name.lower() == 'char_string'

    @property
    def is_struct(self):
        return self.context.is_struct_type(self.data_type.name)

    @property
    def is_enum(self):
        return self.context.is_enum_type(self.data_type.name)

    @property
    def is_bitmap(self):
        return self.data_type.name in [
           "bitmap8",
           "bitmap16",
           "bitmap24",
           "bitmap32",
           "bitmap64",
        ]

    def clone(self):
        return EncodableValue(self.context, self.data_type, self.attrs)

    def without_nullable(self):
        result = self.clone()
        result.attrs.remove(EncodableValueAttr.NULLABLE)   
        return result

    def without_optional(self):
        result = self.clone()
        result.attrs.remove(EncodableValueAttr.OPTIONAL)   
        return result

    def without_list(self):
        result = self.clone()
        result.attrs.remove(EncodableValueAttr.LIST)   
        return result

    def get_underlying_struct(self):
        for s in self.context.all_structs():
            if s.name == self.data_type.name:
                return s
        raise Exception("Struct %s not found" % self.data_type.name)

    def get_underlying_enum(self):
        for e in self.context.all_enums():
            if e.name == self.data_type.name:
                return e
        raise Exception("Struct %s not found" % self.data_type.name)

    @property
    def boxed_java_type(self):
        t = ParseDataType(self.data_type, self.context.all_enums())

        if type(t) == FundamentalType:
            if t == FundamentalType.BOOL:
                return "Boolean"
            elif t == FundamentalType.FLOAT:
                return "Float"
            elif t == FundamentalType.DOUBLE:
                return "Double"
            else:
                raise Error("Unknown fundamental type")
        elif type(t) == BasicInteger:
            if t.byte_count >= 4:
               return "Long"
            else:
               return "Integer"
        elif type(t) == BasicString:
            if t.is_binary:
                return "byte[]"
            else:
                return "String"
        elif type(t) == IdlEnumType:
            return "Integer;"
        else:
            return "Object"

    @property
    def boxed_java_signature(self):
        if self.is_list:
           return "Ljava/lang/ArrayList;"
           
        if self.is_optional:
           return "Ljava/util/Optional;"

        t = ParseDataType(self.data_type, self.context.all_enums())

        if type(t) == FundamentalType:
            if t == FundamentalType.BOOL:
                return "Ljava/lang/Boolean;"
            elif t == FundamentalType.FLOAT:
                return "Ljava/lang/Float;"
            elif t == FundamentalType.DOUBLE:
                return "Ljava/lang/Double;"
            else:
                raise Error("Unknown fundamental type")
        elif type(t) == BasicInteger:
            if t.byte_count >= 4:
               return "Ljava/lang/Long;"
            else:
               return "Ljava/lang/Integer;"
        elif type(t) == BasicString:
            if t.is_binary:
                return "[B"
            else:
                return "Ljava/lang/String;"
        elif type(t) == IdlEnumType:
            return "Ljava/lang/Integer;"
        else:
            return "Lchip/devicecontroller/ChipStructs${}Cluster{};".format(self.context.cluster.name, self.data_type.name)



def EncodableValueFrom(field: Field, context: LookupContext):
    attrs = set()

    if field.is_optional:
        attrs.add(EncodableValueAttr.OPTIONAL)

    if field.is_nullable:
        attrs.add(EncodableValueAttr.NULLABLE)

    if field.is_list:
        attrs.add(EncodableValueAttr.LIST)

    return EncodableValue(context, field.data_type, attrs)



class JavaGenerator(CodeGenerator):
    """
    Generation of java code for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        super().__init__(storage, idl)

        self.jinja_env.filters['attributesWithCallback'] = attributesWithSupportedCallback
        self.jinja_env.filters['callbackName'] = CallbackName
        self.jinja_env.filters['commandCallbackName'] = CommandCallbackName
        self.jinja_env.filters['clientClustersOnly'] = ClientClustersOnly
        self.jinja_env.filters['named'] = NamedFilter
        self.jinja_env.filters['toBoxedJavaType'] = ToBoxedJavaType
        self.jinja_env.filters['lowercaseFirst'] = LowercaseFirst
        self.jinja_env.filters['asEncodable'] = EncodableValueFrom
        self.jinja_env.filters['lookupContext'] = CreateContext


    def internal_render_all(self):
        known_enums = self.idl.enums[:]
        for cluster in self.idl.clusters:
            known_enums.extend(cluster.enums)

        # Single generation for compatibility check
        # Split expected in a future PR
        self.internal_render_one_output(
            template_path="java/ChipClustersCpp.jinja",
            output_file_name="jni/CHIPClusters.cpp",
            vars={
                'idl': self.idl,
                'clusters': self.idl.clusters,
                'known_enums': known_enums,
            }
        )

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
