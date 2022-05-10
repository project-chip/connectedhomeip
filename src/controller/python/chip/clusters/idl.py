#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

from multiprocessing.sharedctypes import Value
import os

from lark import Lark
from chip.idl import matter_idl_parser, matter_idl_types
from . import ClusterObjects
from . import Attribute
import chip.tlv as tlv

import enum
import dataclasses
import typing


def CreateParser():
    """
    Generates a parser that will process a ".matter" file into a IDL
    """
    return Lark.open('../idl/matter_grammar.lark', rel_to=__file__, start='idl', parser='lalr', transformer=matter_idl_parser.MatterIdlTransformer())


def _convert_idl_types(idl_type: str, namespace_types: typing.Mapping[str, type] = None, namespace_enums: typing.Mapping[str, type] = None) -> type:
    idl_type_upper = idl_type.upper()
    if idl_type_upper in ['INT8S', 'INT16S', 'INT24S', 'INT32S', 'INT48S', 'INT56S', 'INT64S']:
        return int
    elif idl_type_upper in ['INT8U', 'INT16U', 'INT24U', 'INT32U', 'INT48U', 'INT56U', 'INT64U']:
        return tlv.uint
    elif idl_type_upper == 'BOOLEAN':
        return bool
    elif idl_type_upper in ['SINGLE', 'DOUBLE']:
        return float
    elif idl_type_upper in ['CHAR_STRING', 'LONG_CHAR_STRING']:
        return str
    elif idl_type_upper in ['OCTET_STRING', 'LONG_OCTET_STRING']:
        return bytes
    elif idl_type_upper in ['BITMAP8', 'BITMAP16', 'BITMAP32', 'BITMAP64', 'ENUM8', 'ENUM16', 'ENUM32', 'ENUM64']:
        return tlv.uint
    elif idl_type_upper in ['FABRIC_IDX', 'FABRIC_ID', 'NODE_ID', 'VENDOR_ID', 'GROUP_ID', 'EPOCH_S', 'EPOCH_US', 'DEVTYPE_ID', 'CLUSTER_ID', 'ENDPOINT_NO', 'PERCENT', 'PERCENT100THS']:
        return tlv.uint
    else:
        namespace_type = namespace_types.get(idl_type, None)
        if namespace_type is not None:
            return namespace_type
        # Unknown type
        raise ValueError(f"{idl_type} cannot be mapped to existing types")


def _make_python_typing_hint(t: matter_idl_types.DataType, is_list: bool = False, is_nullable: bool = False, is_optional: bool = False, namespace_types: typing.Mapping[str, type] = None):
    ret_type = _convert_idl_types(t.name, namespace_types=namespace_types)
    if is_list:
        ret_type = typing.List[ret_type]
    if is_nullable:
        if is_optional:
            ret_type = typing.Union[None, ClusterObjects.Nullable, ret_type]
        else:
            ret_type = typing.Union[ClusterObjects.Nullable, ret_type]
    elif is_optional:
        ret_type = typing.Optional[ret_type]
    return ret_type


def _make_pythong_typing_hint_from_idl_field(t: matter_idl_types.Field, namespace_types: typing.Mapping[str, type] = None):
    return _make_python_typing_hint(t.data_type, is_list=t.is_list, is_nullable=t.is_nullable, is_optional=t.is_optional, namespace_types=namespace_types)


def _make_attribute_class(cluster: matter_idl_types.Cluster, attribute: matter_idl_types.Attribute, namespace_types: typing.Mapping[str, type] = None) -> type:
    attribute_type = _make_pythong_typing_hint_from_idl_field(
        t=attribute.definition, namespace_types=namespace_types)

    res = dataclasses.make_dataclass(cls_name=attribute.definition.name, bases=(
        ClusterObjects.ClusterAttributeDescriptor,), fields=[('value', attribute_type, None)])
    res.cluster_id = cluster.code
    res.attribute_id = attribute.definition.code
    res.attribute_type = ClusterObjects.ClusterObjectFieldDescriptor(Type=attribute_type)
    return res


def _make_structs_descriptor(cluster: matter_idl_types.Cluster, struct: matter_idl_types.Struct, namespace_types: typing.Mapping[str, type]):
    return [
        ClusterObjects.ClusterObjectFieldDescriptor(Label=f.name, Tag=f.code, Type=_make_pythong_typing_hint_from_idl_field(f, namespace_types=namespace_types)) for f in struct.fields
    ]


def _make_struct_class(cluster: matter_idl_types.Cluster, struct: matter_idl_types.Struct, namespace_types: typing.Mapping[str, type]):
    struct_fields = _make_structs_descriptor(cluster, struct, namespace_types=namespace_types)
    res = dataclasses.make_dataclass(cls_name=struct.name, fields=[
        (f.Label, f.Type, None) for f in struct_fields], bases=(ClusterObjects.ClusterObject, ))
    res.descriptor = ClusterObjects.ClusterObjectDescriptor(Fields=struct_fields)
    return res


def _make_request_command_class(cluster: matter_idl_types.Cluster, command: matter_idl_types.Command, fields: typing.List[ClusterObjects.ClusterObjectFieldDescriptor]):
    res = dataclasses.make_dataclass(cls_name=command.name, fields=[
        (f.Label, f.Type) for f in fields], bases=(ClusterObjects.ClusterCommand, ))
    res.descriptor = ClusterObjects.ClusterObjectDescriptor(Fields=fields)
    res.cluster_id = cluster.code
    res.command_id = command.code
    if command.is_timed_invoke:
        res.must_use_timed_invoke = True
    res.is_client = False
    return res


def _make_response_command_class(cluster: matter_idl_types.Cluster, struct: matter_idl_types.Struct, fields: typing.List[ClusterObjects.ClusterObjectFieldDescriptor]):
    res = dataclasses.make_dataclass(cls_name=struct.name, fields=[
        (f.Label, f.Type) for f in fields], bases=(ClusterObjects.ClusterCommand, ))
    res.descriptor = ClusterObjects.ClusterObjectDescriptor(Fields=fields)
    res.cluster_id = cluster.code
    res.command_id = struct.code
    res.is_client = True
    return res


def _parse_single_cluster(cluster: matter_idl_types.Cluster):
    enums = {e.name: enum.IntEnum(e.name, {v.name: v.code for v in e.entries}) for e in cluster.enums}
    bitmaps = {e.name: enum.IntFlag(e.name, {v.name: v.code for v in e.entries}) for e in cluster.bitmaps}
    structs = {}
    namespace_types = {}
    namespace_types.update(enums)
    namespace_types.update(bitmaps)
    command_fields = {}
    remaining_structs = cluster.structs[:]
    while remaining_structs:
        next_remaining_structs = []
        processed_structs = 0
        for s in remaining_structs:
            if s.tag in [matter_idl_types.StructTag.REQUEST, matter_idl_types.StructTag.RESPONSE]:
                continue
            try:
                new_type = _make_struct_class(cluster, s, namespace_types)
                structs[s.name] = new_type
                namespace_types[s.name] = new_type
                processed_structs += 1
            except ValueError as e:
                next_remaining_structs.append(s)
        if processed_structs == 0:
            break
        remaining_structs = next_remaining_structs

    attributes = {}
    for a in cluster.attributes:
        try:
            attributes[a.definition.name] = _make_attribute_class(cluster, a, namespace_types=namespace_types)
        except:
            pass

    events = {}
    for e in cluster.events:
        try:
            event = _make_struct_class(cluster, event, namespace_types)
            event.event_id = e.code
            event.cluster_id = cluster.code
            events[e.name] = e
        except:
            pass

    commands = {}
    for s in cluster.structs:
        if s.tag not in [matter_idl_types.StructTag.REQUEST, matter_idl_types.StructTag.RESPONSE]:
            continue
        try:
            fields = _make_structs_descriptor(cluster, s, namespace_types)
            if s.tag == matter_idl_types.StructTag.RESPONSE:
                commands[s.name] = _make_response_command_class(cluster, s, fields)
            else:
                command_fields[s.name] = fields
        except ValueError:
            pass

    for c in cluster.commands:
        try:
            req = []
            if c.input_param is not None:
                req = command_fields[c.input_param]
            res = None
            if c.output_param != 'DefaultSuccess':
                res = commands[c.output_param]
            command = _make_request_command_class(cluster, c, req)
            command.response_type = res
            commands[c.name] = command
        except Exception:
            pass

    cluster_namespace = dataclasses.make_dataclass(
        cls_name=cluster.name,
        fields=[
            (k, v.attribute_type.Type, None) for k, v in attributes.items()
        ],
        bases=(ClusterObjects.Cluster, ),
    )
    cluster_namespace.descriptor = ClusterObjects.ClusterObjectDescriptor(
        Fields=[
            ClusterObjects.ClusterObjectFieldDescriptor(Label=k, Tag=v.attribute_id, Type=v.attribute_type.Type) for (k, v) in attributes.items()
        ]
    )
    cluster_namespace.id = cluster.code

    def set_namespace_items(obj: type, name: str, items: typing.Mapping[str, type]):
        namespace = type(name, (), {})
        for k, v in items.items():
            type.__setattr__(namespace, k[0].upper() + k[1:], v)
        type.__setattr__(obj, name, namespace)

    if len(bitmaps) != 0:
        set_namespace_items(cluster_namespace, 'Bitmaps', bitmaps)

    if len(enums) != 0:
        set_namespace_items(cluster_namespace, 'Enums', enums)

    if len(commands) != 0:
        set_namespace_items(cluster_namespace, 'Commands', commands)

    if len(attributes) != 0:
        set_namespace_items(cluster_namespace, 'Attributes', attributes)

    if len(attributes) != 0:
        set_namespace_items(cluster_namespace, 'Events', events)

    if len(structs) != 0:
        set_namespace_items(cluster_namespace, 'Structs', structs)

    return cluster_namespace


def LoadIDL(file: str):
    with open(file) as fp:
        loaded_clusters = type('Clusters', (), {})
        for c in CreateParser().parse(fp.read()).clusters:
            type.__setattr__(loaded_clusters, c.name, _parse_single_cluster(c))
        Attribute.UpdateIndex(loaded_clusters)
        return loaded_clusters


_default_clusters = None


def DefaultClusters():
    global _default_clusters
    if _default_clusters is None:
        _default_clusters = LoadIDL(os.path.join(os.path.dirname(__file__), "default.matter"))
    return _default_clusters
