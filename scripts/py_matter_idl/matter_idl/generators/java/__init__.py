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

import dataclasses
import enum
import logging
import os
from typing import List, Set, Union

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.generators.types import (BasicInteger, BasicString, FundamentalType, IdlBitmapType, IdlEnumType, IdlType,
                                         ParseDataType, TypeLookupContext)
from matter_idl.matter_idl_types import (Attribute, Cluster, ClusterSide, Command, DataType, Field, FieldQuality, Idl, Struct,
                                         StructTag)
from stringcase import capitalcase


@dataclasses.dataclass
class GenerateTarget:
    template: str
    output_name: str


@dataclasses.dataclass
class GlobalType:
    name: str       # java name
    cpp_type: str  # underlying type


# types that java should see globally
_GLOBAL_TYPES = [
    GlobalType("Boolean", "bool"),
    GlobalType("CharString", "const chip::CharSpan"),
    GlobalType("Double", "double"),
    GlobalType("Float", "float"),
    GlobalType("Int8s", "int8_t"),
    GlobalType("Int8u", "uint8_t"),
    GlobalType("Int16s", "int16_t"),
    GlobalType("Int16u", "uint16_t"),
    GlobalType("Int32s", "int32_t"),
    GlobalType("Int32u", "uint32_t"),
    GlobalType("Int64s", "int64_t"),
    GlobalType("Int64u", "uint64_t"),
    GlobalType("OctetString", "const chip::ByteSpan"),
]


def _UnderlyingType(field: Field, context: TypeLookupContext) -> Union[str, None]:
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


def FieldToGlobalName(field: Field, context: TypeLookupContext) -> Union[str, None]:
    """Global names are used for generic callbacks shared across
    all clusters (e.g. for bool/float/uint32 and similar)
    """
    if field.is_list:
        return None  # lists are always specific per cluster

    if FieldQuality.NULLABLE & field.qualities:
        return None

    return _UnderlyingType(field, context)


# Based on atomicType in ZAP:
#  src-electron/generator/matter/app/zap-templates/common/override.js
_KNOWN_DECODABLE_TYPES = {
    'action_id': 'chip::ActionId',
    'attrib_id': 'chip::AttributeId',
    'cluster_id': 'chip::ClusterId',
    'command_id': 'chip::CommandId',
    'data_ver': 'chip::DataVersion',
    'devtype_id': 'chip::DeviceTypeId',
    'endpoint_no': 'chip::EndpointId',
    'eui64': 'chip::NodeId',
    'event_id': 'chip::EventId',
    'event_no': 'chip::EventNumber',
    'fabric_id': 'chip::FabricId',
    'fabric_idx': 'chip::FabricIndex',
    'fabric_idx': 'chip::FabricIndex',
    'field_id': 'chip::FieldId',
    'group_id': 'chip::GroupId',
    'node_id': 'chip::NodeId',
    'percent': 'chip::Percent',
    'percent100ths': 'chip::Percent100ths',
    'transaction_id': 'chip::TransactionId',
    'vendor_id': 'chip::VendorId',

    # non-named enums
    'enum8': 'uint8_t',
    'enum16': 'uint16_t',
    'enum32': 'uint32_t',
    'enum64': 'uint64_t',
}


def _CppType(field: Field, context: TypeLookupContext) -> Union[str, None]:
    if field.data_type.name.lower() in _KNOWN_DECODABLE_TYPES:
        return _KNOWN_DECODABLE_TYPES[field.data_type.name.lower()]

    actual = ParseDataType(field.data_type, context)
    if isinstance(actual, BasicString):
        if actual.is_binary:
            return 'chip::ByteSpan'
        else:
            return 'chip::CharSpan'
    elif isinstance(actual, BasicInteger):
        if actual.is_signed:
            return "int{}_t".format(actual.power_of_two_bits)
        else:
            return "uint{}_t".format(actual.power_of_two_bits)
    elif isinstance(actual, FundamentalType):
        if actual == FundamentalType.BOOL:
            return 'bool'
        elif actual == FundamentalType.FLOAT:
            return 'float'
        elif actual == FundamentalType.DOUBLE:
            return 'double'
        else:
            logging.warn('Unknown fundamental type: %r' % actual)
    elif isinstance(actual, IdlType):
        return f"chip::app::Clusters::{context.cluster.name}::Structs::{field.data_type.name}::DecodableType"
    elif isinstance(actual, IdlBitmapType):
        return f"chip::BitMask<chip::app::Clusters::{context.cluster.name}::{field.data_type.name}>"

    # Handles IdlEnumType
    return f"chip::app::Clusters::{context.cluster.name}::{field.data_type.name}"


def DecodableJniType(field: Field, context: TypeLookupContext) -> str:
    actual = _CppType(field, context)

    if field.is_list:
        return f"const chip::app::DataModel::DecodableList<{actual}> &"

    if field.is_nullable:
        return f"const chip::app::DataModel::Nullable<{actual}> &"

    return actual


def GlobalNameToJavaName(name: str) -> str:
    if name in {'Int8u', 'Int8s', 'Int16u', 'Int16s'}:
        return 'Integer'

    if name.startswith('Int'):
        return 'Long'

    # Double/Float/Booleans/CharString/OctetString
    return name


def DelegatedCallbackName(attr: Attribute, context: TypeLookupContext) -> str:
    """
    Figure out what callback name to use for delegate callback construction.
    """
    global_name = FieldToGlobalName(attr.definition, context)

    if global_name:
        return 'Delegated{}AttributeCallback'.format(GlobalNameToJavaName(global_name))

    return 'Delegated{}Cluster{}AttributeCallback'.format(context.cluster.name, capitalcase(attr.definition.name))


def ChipClustersCallbackName(attr: Attribute, context: TypeLookupContext) -> str:
    """
    Figure out what callback name to use when building a ChipCluster.*AttributeCallback
    in java codegen.
    """
    global_name = FieldToGlobalName(attr.definition, context)

    if global_name:
        return 'ChipClusters.{}AttributeCallback'.format(GlobalNameToJavaName(global_name))

    return 'ChipClusters.{}Cluster.{}AttributeCallback'.format(context.cluster.name, capitalcase(attr.definition.name))


def CallbackName(attr: Attribute, context: TypeLookupContext) -> str:
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
        capitalcase(context.cluster.name),
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


def _IsUsingGlobalCallback(field: Field, context: TypeLookupContext):
    """Test to determine if the data type of a field can use one of
    the global callbacks (i.e. it is a basic double/integer/bool etc.)
    """
    if field.is_list:
        return False

    if field.is_nullable:
        return False

    return field.data_type.name in {
        "boolean",
        "single",
        "double",
        "int8s",
        "int8u",
        "int16s",
        "int16u",
        "int24s",
        "int24u",
        "int32s",
        "int32u",
        "int40s",
        "int40u",
        "int48s",
        "int48u",
        "int56s",
        "int56u",
        "int64s",
        "int64u",
        "enum8",
        "enum16",
        "enum32",
        "enum64",
        "bitmap8",
        "bitmap16",
        "bitmap32",
        "bitmap64",
        "char_string",
        "long_char_string",
        "octet_string",
        "long_octet_string",
    }


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
                raise Exception("Unknown fundamental type")
        elif type(t) == BasicInteger:
            # the >= 3 will include int24_t to be considered "long"
            if t.byte_count >= 3:
                return "Long"
            else:
                return "Integer"
        elif type(t) == BasicString:
            if t.is_binary:
                return "byte[]"
            else:
                return "String"
        elif type(t) == IdlEnumType:
            if t.base_type.byte_count >= 3:
                return "Long"
            else:
                return "Integer"
        elif type(t) == IdlBitmapType:
            if t.base_type.byte_count >= 3:
                return "Long"
            else:
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
                raise Exception("Unknown fundamental type")
        elif type(t) == BasicInteger:
            if t.byte_count >= 3:
                return "Ljava/lang/Long;"
            else:
                return "Ljava/lang/Integer;"
        elif type(t) == BasicString:
            if t.is_binary:
                return "[B"
            else:
                return "Ljava/lang/String;"
        elif type(t) == IdlEnumType:
            if t.base_type.byte_count >= 3:
                return "Ljava/lang/Long;"
            else:
                return "Ljava/lang/Integer;"
        elif type(t) == IdlBitmapType:
            if t.base_type.byte_count >= 3:
                return "Ljava/lang/Long;"
            else:
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


def IsResponseStruct(s: Struct) -> bool:
    return s.tag == StructTag.RESPONSE


class __JavaCodeGenerator(CodeGenerator):
    """
    Code generation for java-specific files.

    Registers filters used by all java generators.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, **kargs):
        """
        Inintialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl, fs_loader_searchpath=os.path.dirname(__file__))

        self.jinja_env.filters['attributesWithCallback'] = attributesWithSupportedCallback
        self.jinja_env.filters['callbackName'] = CallbackName
        self.jinja_env.filters['chipClustersCallbackName'] = ChipClustersCallbackName
        self.jinja_env.filters['delegatedCallbackName'] = DelegatedCallbackName
        self.jinja_env.filters['commandCallbackName'] = CommandCallbackName
        self.jinja_env.filters['named'] = NamedFilter
        self.jinja_env.filters['toBoxedJavaType'] = ToBoxedJavaType
        self.jinja_env.filters['lowercaseFirst'] = LowercaseFirst
        self.jinja_env.filters['asEncodable'] = EncodableValueFrom
        self.jinja_env.filters['createLookupContext'] = CreateLookupContext
        self.jinja_env.filters['canGenerateSubscribe'] = CanGenerateSubscribe
        self.jinja_env.filters['decodableJniType'] = DecodableJniType

        self.jinja_env.tests['is_response_struct'] = IsResponseStruct
        self.jinja_env.tests['is_using_global_callback'] = _IsUsingGlobalCallback


class JavaJNIGenerator(__JavaCodeGenerator):
    """Generates JNI java files (i.e. C++ source/headers)."""

    def __init__(self, *args, **kargs):
        super().__init__(*args, **kargs)

    def internal_render_all(self):
        """
        Renders .CPP files required for JNI support.
        """

        large_targets = [
            GenerateTarget(template="CHIPCallbackTypes.jinja",
                           output_name="jni/CHIPCallbackTypes.h"),
            GenerateTarget(template="CHIPReadCallbacks_h.jinja",
                           output_name="jni/CHIPReadCallbacks.h")
        ]

        for target in large_targets:
            self.internal_render_one_output(
                template_path=target.template,
                output_file_name=target.output_name,
                vars={
                    'idl': self.idl,
                    'clientClusters': [c for c in self.idl.clusters if c.side == ClusterSide.CLIENT],
                    'globalTypes': _GLOBAL_TYPES,
                }
            )

        cluster_targets = [
            GenerateTarget(template="ChipClustersRead.jinja",
                           output_name="jni/{cluster_name}Client-ReadImpl.cpp"),
            GenerateTarget(template="ChipClustersCpp.jinja",
                           output_name="jni/{cluster_name}Client-InvokeSubscribeImpl.cpp"),
        ]

        self.internal_render_one_output(
            template_path="CHIPCallbackTypes.jinja",
            output_file_name="jni/CHIPCallbackTypes.h",
            vars={
                'idl': self.idl,
                'clientClusters': [c for c in self.idl.clusters if c.side == ClusterSide.CLIENT],
            }
        )

        # Every cluster has its own impl, to avoid
        # very large compilations (running out of RAM)
        for cluster in self.idl.clusters:
            if cluster.side != ClusterSide.CLIENT:
                continue

            for target in cluster_targets:
                self.internal_render_one_output(
                    template_path=target.template,
                    output_file_name=target.output_name.format(
                        cluster_name=cluster.name),
                    vars={
                        'cluster': cluster,
                        'typeLookup': TypeLookupContext(self.idl, cluster),
                        'globalTypes': _GLOBAL_TYPES,
                    }
                )


class JavaClassGenerator(__JavaCodeGenerator):
    """Generates .java files """

    def __init__(self, *args, **kargs):
        super().__init__(*args, **kargs)

    def internal_render_all(self):
        """
        Renders .java files required for java matter support
        """

        clientClusters = [
            c for c in self.idl.clusters if c.side == ClusterSide.CLIENT]

        self.internal_render_one_output(
            template_path="ClusterReadMapping.jinja",
            output_file_name="java/chip/devicecontroller/ClusterReadMapping.java",
            vars={
                'idl': self.idl,
                'clientClusters': clientClusters,
            }
        )

        self.internal_render_one_output(
            template_path="ClusterWriteMapping.jinja",
            output_file_name="java/chip/devicecontroller/ClusterWriteMapping.java",
            vars={
                'idl': self.idl,
                'clientClusters': clientClusters,
            }
        )
