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

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Idl, ClusterSide, Field, Attribute, Cluster, FieldQuality, Command, DataType
from matter_idl import matter_idl_types
from matter_idl.generators.types import ParseDataType, BasicString, BasicInteger, FundamentalType, IdlType, IdlEnumType, IdlBitmapType, TypeLookupContext
from typing import Union, List, Set
from stringcase import capitalcase

import enum
import logging


def FieldToGlobalName(field: Field, context: TypeLookupContext) -> Union[str, None]:
    """Global names are used for generic callbacks shared across
    all clusters (e.g. for bool/float/uint32 and similar)
    """
    if field.is_list:
        return None  # lists are always specific per cluster

    if FieldQuality.NULLABLE & field.qualities:
        return None

    if FieldQuality.OPTIONAL & field.qualities:
        return None

    actual = ParseDataType(field.data_type, context)
    if type(actual) == IdlEnumType:
        actual = actual.base_type
    elif type(actual) == IdlBitmapType:
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


def CallbackName(attr: Attribute, cluster: Cluster, context: TypeLookupContext) -> str:
    """
    Figure out what callback name to use when a variable requires a read callback.

    These are split into native types, like Boolean/Float/Double/CharString, where
    one callback type can support anything. 

    For specific types (e.g. A struct) codegen will generate its own callback name
    specific to that type.
    """
    global_name = FieldToGlobalName(attr.definition, context)

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


def attributesWithSupportedCallback(attrs, context: TypeLookupContext):
    for attr in attrs:
        # Attributes will be generated for all types
        # except non-list structures
        if not attr.definition.is_list:
            underlying = ParseDataType(attr.definition.data_type, context)
            if type(underlying) == IdlType:
                continue

        yield attr


def NamedFilter(choices: List, name: str):
    for choice in choices:
        if choice.name == name:
            return choice
    raise Exception("No item named %s in %r" % (name, choices))


def ToBoxedJavaType(field: Field):
    if field.is_optional:
        return 'jobject'
    elif field.data_type.name.lower() in ['octet_string', 'long_octet_string']:
        return 'jbyteArray'
    elif field.data_type.name.lower() in ['char_string', 'long_char_string']:
        return 'jstring'
    else:
        return 'jobject'


def LowercaseFirst(name: str) -> str:
    """
    Change the first letter of a string to lowercase as long as the 2nd
    letter is not uppercase.

    Can be used for variable naming, eg insider structures, codegen will
    call things "Foo foo" (notice variable name is lowercase).
    """
    if len(name) > 1 and name[1].lower() != name[1]:
        # Odd workaround: PAKEVerifier should not become pAKEVerifier
        return name
    return name[0].lower() + name[1:]


class EncodableValueAttr(enum.Enum):
    LIST = enum.auto()
    NULLABLE = enum.auto()
    OPTIONAL = enum.auto()


class EncodableValue:
    """
    Contains helpers for encoding values, specifically lookups
    for optionality, lists and recursive data type lookups within
    the IDL and cluster

    Intended use is to be able to:
      - derive types (see clone and without_* methods) such that codegen
        can implement things like 'if x != null { treat non-null x}'
      - Java specific conversions: get boxed types and JNI string signautes
        for the underlying types.
    """

    def __init__(self, context: TypeLookupContext, data_type: DataType, attrs: Set[EncodableValueAttr]):
        self.context = context
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
        return self.data_type.name.lower() in ['octet_string', 'long_octet_string']

    @property
    def is_char_string(self):
        return self.data_type.name.lower() in ['char_string', 'long_char_string']

    @property
    def is_struct(self):
        return self.context.is_struct_type(self.data_type.name)

    @property
    def is_enum(self):
        return self.context.is_enum_type(self.data_type.name)

    @property
    def is_bitmap(self):
        self.context.is_bitmap_type(self.data_type.name)

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
        s = self.context.find_struct(self.data_type.name)
        if not s:
            raise Exception("Struct %s not found" % self.data_type.name)
        return s

    def get_underlying_enum(self):
        e = self.context.find_enum(self.data_type.name)
        if not e:
            raise Exception("Enum %s not found" % self.data_type.name)
        return e

    @property
    def boxed_java_type(self):
        t = ParseDataType(self.data_type, self.context)

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
            return "Integer"
        elif type(t) == IdlBitmapType:
            return "Integer"
        else:
            return "Object"

    @property
    def boxed_java_signature(self):
        # Optional takes precedence over list - Optional<ArrayList> compiles down to just java.util.Optional.
        if self.is_optional:
            return "Ljava/util/Optional;"

        if self.is_list:
            return "Ljava/util/ArrayList;"

        t = ParseDataType(self.data_type, self.context)

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
        elif type(t) == IdlBitmapType:
            return "Ljava/lang/Integer;"
        else:
            return "Lchip/devicecontroller/ChipStructs${}Cluster{};".format(self.context.cluster.name, self.data_type.name)


def EncodableValueFrom(field: Field, context: TypeLookupContext) -> EncodableValue:
    """
    Filter to convert a standard field to an EncodableValue.

    This converts the AST information (field name/info + lookup context) into
    a java-generator specific wrapper that can be manipulated and 
    queried for properties like java native name or JNI string signature.
    """
    attrs = set()

    if field.is_optional:
        attrs.add(EncodableValueAttr.OPTIONAL)

    if field.is_nullable:
        attrs.add(EncodableValueAttr.NULLABLE)

    if field.is_list:
        attrs.add(EncodableValueAttr.LIST)

    return EncodableValue(context, field.data_type, attrs)


def CreateLookupContext(idl: Idl, cluster: Cluster) -> TypeLookupContext:
    """
    A filter to mark a lookup context to be within a specific cluster.

    This is used to specify how structure/enum/other names are looked up.
    Generally one looks up within the specific cluster then if cluster does
    not contain a definition, we loop at global namespacing.
    """
    return TypeLookupContext(idl, cluster)


def CanGenerateSubscribe(attr: Attribute, lookup: TypeLookupContext) -> bool:
    """
    Filter that returns if an attribute can be subscribed to.

    Uses the given attribute and the lookupContext to figure out the attribute
    type.
    """
    # For backwards compatibility, we do not subscribe to structs
    # (although list of structs is ok ...)
    if attr.definition.is_list:
        return True

    return not lookup.is_struct_type(attr.definition.data_type.name)


class JavaGenerator(CodeGenerator):
    """
    Generation of java code for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        """
        Inintialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl)

        self.jinja_env.filters['attributesWithCallback'] = attributesWithSupportedCallback
        self.jinja_env.filters['callbackName'] = CallbackName
        self.jinja_env.filters['commandCallbackName'] = CommandCallbackName
        self.jinja_env.filters['named'] = NamedFilter
        self.jinja_env.filters['toBoxedJavaType'] = ToBoxedJavaType
        self.jinja_env.filters['lowercaseFirst'] = LowercaseFirst
        self.jinja_env.filters['asEncodable'] = EncodableValueFrom
        self.jinja_env.filters['createLookupContext'] = CreateLookupContext
        self.jinja_env.filters['canGenerateSubscribe'] = CanGenerateSubscribe

    def internal_render_all(self):
        """
        Renders .CPP files required for JNI support.
        """
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
                    'typeLookup': TypeLookupContext(self.idl, cluster),
                }
            )

            self.internal_render_one_output(
                template_path="java/ChipClustersCpp.jinja",
                output_file_name="jni/%sClient-InvokeSubscribeImpl.cpp" % cluster.name,
                vars={
                    'cluster': cluster,
                    'typeLookup': TypeLookupContext(self.idl, cluster),
                }
            )
