#!/usr/bin/env python

import dataclasses
import functools
import logging
from typing import Dict, Optional, Set

from lark import Lark
from lark.lexer import Token
from lark.visitors import Transformer, v_args

try:
    from matter_idl.matter_idl_types import AccessPrivilege
except ModuleNotFoundError:
    import os
    import sys
    sys.path.append(os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))))

    from matter_idl.matter_idl_types import AccessPrivilege

from matter_idl.matter_idl_types import (ApiMaturity, Attribute, AttributeInstantiation, AttributeOperation, AttributeQuality,
                                         AttributeStorage, Bitmap, Cluster, Command, CommandInstantiation, CommandQuality,
                                         ConstantEntry, DataType, DeviceType, Endpoint, Enum, Event, EventPriority, EventQuality,
                                         Field, FieldQuality, Idl, ParseMetaData, ServerClusterInstantiation, Struct, StructQuality,
                                         StructTag)


def UnionOfAllFlags(flags_list):
    if not flags_list:
        return None
    return functools.reduce(lambda a, b: a | b, flags_list)


class PrefixCppDocComment:
    def __init__(self, token):
        self.start_pos = token.start_pos
        # Assume CPP comments: /**...*/
        self.value_len = len(token.value)  # includes /***/ AND whitespace
        self.value = token.value[3:-2].strip()

    def appply_to_idl(self, idl: Idl, content: str):
        if self.start_pos is None:
            return

        actual_pos = self.start_pos + self.value_len
        while content[actual_pos] in ' \t\n\r':
            actual_pos += 1

        # A doc comment will apply to any supported element assuming it immediately
        # preceeds id (skipping whitespace)
        for item in self.supported_types(idl):
            meta = item.parse_meta
            if meta and meta.start_pos == actual_pos:
                item.description = self.value
                return

    def supported_types(self, idl: Idl):
        """List all types supported by doc comments."""
        for cluster in idl.clusters:
            yield cluster

            for command in cluster.commands:
                yield command

    def __repr__(self):
        return ("PREFIXDoc: %r at %r" % (self.value, self.start_pos))


class AddServerClusterToEndpointTransform:
    """Provides an 'apply' method that can be run on endpoints
       to add a server cluster to the given endpoint.
    """

    def __init__(self, cluster: ServerClusterInstantiation):
        self.cluster = cluster

    def apply(self, endpoint):
        endpoint.server_clusters.append(self.cluster)


class AddBindingToEndpointTransform:
    """Provides an 'apply' method that can be run on endpoints
       to add a cluster binding to the given endpoint.
    """

    def __init__(self, name):
        self.name = name

    def apply(self, endpoint):
        endpoint.client_bindings.append(self.name)


class AddDeviceTypeToEndpointTransform:
    """Provides an 'apply' method that can be run on endpoints
       to add a device type to it
    """

    def __init__(self, device_type: DeviceType):
        self.device_type = device_type

    def apply(self, endpoint):
        endpoint.device_types.append(self.device_type)


class MatterIdlTransformer(Transformer):
    """
    A transformer capable to transform data parsed by Lark according to
    matter_grammar.lark.

    Generally transforms a ".matter" file into an Abstract Syntax Tree (AST).
    End result will be a `matter_idl_types.Idl` value that represents the
    entire parsed .matter file.

    The content of this file closely resembles the .lark input file and its
    purpose is to convert LARK tokens (that ar generally inputted by name)
    into underlying python types.

    Some documentation to get started is available at
    https://lark-parser.readthedocs.io/en/latest/visitors.html#transformer

    TLDR would be:
      When the ".lark" defines a token like `foo: number`, the transformer
      has the option to define a method called `foo` which will take the
      parsed input (as strings unless transformed) and interpret them.

      Actual parametes to the methods depend on the rules multiplicity and/or
      optionally.
    """

    def __init__(self, skip_meta):
        self.skip_meta = skip_meta
        self.doc_comments = []

    def positive_integer(self, tokens):
        """Numbers in the grammar are integers or hex numbers.
        """
        if len(tokens) != 1:
            raise Exception("Unexpected argument counts")

        n = tokens[0].value
        if n.startswith('0x'):
            return int(n[2:], 16)
        else:
            return int(n)

    @v_args(inline=True)
    def negative_integer(self, value):
        return -value

    @v_args(inline=True)
    def integer(self, value):
        return value

    def bool_default_true(self, _):
        return True

    def bool_default_false(self, _):
        return False

    def provisional_api_maturity(self, _):
        return ApiMaturity.PROVISIONAL

    def internal_api_maturity(self, _):
        return ApiMaturity.INTERNAL

    def deprecated_api_maturity(self, _):
        return ApiMaturity.DEPRECATED

    def stable_api_maturity(self, _):
        return ApiMaturity.STABLE

    def id(self, tokens):
        """An id is a string containing an identifier
        """
        if len(tokens) != 1:
            raise Exception("Unexpected argument counts")
        return tokens[0].value

    def type(self, tokens):
        """A type is just a string for the type
        """
        if len(tokens) != 1:
            raise Exception("Unexpected argument counts")
        return tokens[0].value

    def data_type(self, tokens):
        if len(tokens) == 1:
            return DataType(name=tokens[0])
            # Just a string for data type
        elif len(tokens) == 2:
            return DataType(name=tokens[0], max_length=tokens[1])
        else:
            raise Exception("Unexpected size for data type")

    @v_args(inline=True)
    def constant_entry(self, api_maturity, id, number):
        if api_maturity is None:
            api_maturity = ApiMaturity.STABLE
        return ConstantEntry(name=id, code=number, api_maturity=api_maturity)

    @v_args(inline=True)
    def enum(self, id, type, *entries):
        return Enum(name=id, base_type=type, entries=list(entries))

    @v_args(inline=True)
    def bitmap(self, id, type, *entries):
        return Bitmap(name=id, base_type=type, entries=list(entries))

    def field(self, args):
        data_type, name = args[0], args[1]
        is_list = (len(args) == 4)
        code = args[-1]

        return Field(data_type=data_type, name=name, code=code, is_list=is_list)

    def optional(self, _):
        return FieldQuality.OPTIONAL

    def nullable(self, _):
        return FieldQuality.NULLABLE

    def fabric_sensitive(self, _):
        return FieldQuality.FABRIC_SENSITIVE

    def attr_readonly(self, _):
        return AttributeQuality.READABLE

    def attr_nosubscribe(self, _):
        return AttributeQuality.NOSUBSCRIBE

    def attr_timed(self, _):
        return AttributeQuality.TIMED_WRITE

    def attribute_qualities(self, qualities):
        return UnionOfAllFlags(qualities) or AttributeQuality.NONE

    def struct_fabric_scoped(self, _):
        return StructQuality.FABRIC_SCOPED

    def struct_qualities(self, qualities):
        return UnionOfAllFlags(qualities) or StructQuality.NONE

    def critical_priority(self, _):
        return EventPriority.CRITICAL

    def info_priority(self, _):
        return EventPriority.INFO

    def debug_priority(self, _):
        return EventPriority.DEBUG

    def event_fabric_sensitive(self, _):
        return EventQuality.FABRIC_SENSITIVE

    def event_qualities(selt, qualities):
        return UnionOfAllFlags(qualities) or EventQuality.NONE

    def timed_command(self, _):
        return CommandQuality.TIMED_INVOKE

    def fabric_scoped_command(self, _):
        return CommandQuality.FABRIC_SCOPED

    def command_qualities(self, attrs):
        return UnionOfAllFlags(attrs) or CommandQuality.NONE

    def struct_field(self, args):
        # Last argument is the named_member, the rest
        # are qualities
        field = args[-1]
        field.qualities = UnionOfAllFlags(args[1:-1]) or FieldQuality.NONE
        if args[0] is not None:
            field.api_maturity = args[0]
        return field

    def command_access(self, privilege):
        return privilege[0]

    def command_with_access(self, args):
        # Arguments
        #   - optional access for invoke
        #   - event identifier (name)
        init_args = {
            "name": args[-1]
        }
        if len(args) > 1:
            init_args["invokeacl"] = args[0]

        return init_args

    # NOTE: awkward inline because the order of 'meta, children' vs 'children, meta' was flipped
    #       between lark versions in https://github.com/lark-parser/lark/pull/993
    @v_args(meta=True, inline=True)
    def command(self, meta, *tuple_args):
        # The command takes 4 arguments if no input argument, 5 if input
        # argument is provided
        args = list(tuple_args)  # convert from tuple
        if len(args) != 5:
            args.insert(2, None)

        meta = None if self.skip_meta else ParseMetaData(meta)

        cmd = Command(
            parse_meta=meta,
            qualities=args[0],
            input_param=args[2], output_param=args[3], code=args[4],
            **args[1],
        )

        return cmd

    def event_access(self, privilege):
        return privilege[0]

    def event_with_access(self, args):
        # Arguments
        #   - optional access for read
        #   - event identifier (name)
        init_args = {
            "name": args[-1]
        }
        if len(args) > 1:
            init_args["readacl"] = args[0]

        return init_args

    @v_args(inline=True)
    def cluster_revision(self, revision):
        return revision

    def event(self, args):
        return Event(qualities=args[0], priority=args[1], code=args[3], fields=args[4:], **args[2])

    def view_privilege(self, args):
        return AccessPrivilege.VIEW

    def operate_privilege(self, args):
        return AccessPrivilege.OPERATE

    def manage_privilege(self, args):
        return AccessPrivilege.MANAGE

    def administer_privilege(self, args):
        return AccessPrivilege.ADMINISTER

    def read_access(self, args):
        return AttributeOperation.READ

    def write_access(self, args):
        return AttributeOperation.WRITE

    @v_args(inline=True)
    def attribute_access_entry(self, operation, access):
        return (operation, access)

    def attribute_access(self, value):
        # return value as-is to not need to deal with trees in `attribute_with_access`
        return value

    def attribute_with_access(self, args):
        # Input arguments are:
        #   - acl (optional list of pairs operation + access)
        #   - field definition
        acl = {}
        if len(args) > 1:
            for operation, access in args[0]:
                if operation == AttributeOperation.READ:
                    acl['readacl'] = access
                elif operation == AttributeOperation.WRITE:
                    acl['writeacl'] = access
                else:
                    raise Exception(
                        "Unknown attribute operation: %r" % operation)

        return (args[-1], acl)

    def ram_attribute(self, _):
        return AttributeStorage.RAM

    def persist_attribute(self, _):
        return AttributeStorage.PERSIST

    def callback_attribute(self, _):
        return AttributeStorage.CALLBACK

    @v_args(meta=True, inline=True)
    def endpoint_attribute_instantiation(self, meta, storage, id, default=None):
        meta = None if self.skip_meta else ParseMetaData(meta)
        return AttributeInstantiation(parse_meta=meta, name=id, storage=storage, default=default)

    @v_args(meta=True, inline=True)
    def endpoint_command_instantiation(self, meta, id):
        meta = None if self.skip_meta else ParseMetaData(meta)
        return CommandInstantiation(parse_meta=meta, name=id)

    @v_args(meta=True, inline=True)
    def endpoint_emitted_event(self, meta, id):
        meta = None if self.skip_meta else ParseMetaData(meta)
        return id

    def ESCAPED_STRING(self, s):
        # handle escapes, skip the start and end quotes
        return s.value[1:-1].encode('utf-8').decode('unicode-escape')

    @v_args(inline=True)
    def attribute(self, qualities, definition_tuple):
        (definition, acl) = definition_tuple

        # until we support write only (and need a bit of a reshuffle)
        # if the 'attr_readonly == READABLE' is not in the list, we make things
        # read/write
        if AttributeQuality.READABLE not in qualities:
            qualities |= AttributeQuality.READABLE
            qualities |= AttributeQuality.WRITABLE

        return Attribute(definition=definition, qualities=qualities, **acl)

    @v_args(inline=True)
    def struct(self, qualities, id, *fields):
        return Struct(name=id, qualities=qualities, fields=list(fields))

    @v_args(inline=True)
    def request_struct(self, value):
        value.tag = StructTag.REQUEST
        return value

    @v_args(inline=True)
    def response_struct(self, id, code, *fields):
        return Struct(name=id, tag=StructTag.RESPONSE, code=code, fields=list(fields))

    @v_args(inline=True)
    def endpoint(self, number, *transforms):
        endpoint = Endpoint(number=number)

        for t in transforms:
            t.apply(endpoint)

        return endpoint

    @v_args(inline=True)
    def endpoint_device_type(self, name, code, version):
        return AddDeviceTypeToEndpointTransform(DeviceType(name=name, code=code, version=version))

    @v_args(inline=True)
    def endpoint_cluster_binding(self, id):
        return AddBindingToEndpointTransform(id)

    @v_args(meta=True, inline=True)
    def endpoint_server_cluster(self, meta, id, *content):
        meta = None if self.skip_meta else ParseMetaData(meta)

        attributes = []
        commands = []
        events = set()

        for item in content:
            if isinstance(item, AttributeInstantiation):
                attributes.append(item)
            elif isinstance(item, CommandInstantiation):
                commands.append(item)
            else:
                events.add(item)
        return AddServerClusterToEndpointTransform(
            ServerClusterInstantiation(parse_meta=meta, name=id, attributes=attributes, events_emitted=events, commands=commands))

    @v_args(inline=True)
    def cluster_content(self, api_maturity, element):
        if api_maturity is not None:
            element.api_maturity = api_maturity
        return element

    @v_args(inline=True, meta=True)
    def cluster(self, meta, api_maturity, name, code, revision, *content):
        meta = None if self.skip_meta else ParseMetaData(meta)

        if api_maturity is None:
            api_maturity = ApiMaturity.STABLE

        if not revision:
            revision = 1

        result = Cluster(parse_meta=meta, name=name, code=code,
                         revision=revision, api_maturity=api_maturity)

        for item in content:
            if isinstance(item, Enum):
                result.enums.append(item)
            elif isinstance(item, Bitmap):
                result.bitmaps.append(item)
            elif isinstance(item, Event):
                result.events.append(item)
            elif isinstance(item, Attribute):
                result.attributes.append(item)
            elif isinstance(item, Struct):
                result.structs.append(item)
            elif isinstance(item, Command):
                result.commands.append(item)
            else:
                raise Exception("UNKNOWN cluster content item: %r" % item)

        return result

    def idl(self, items):
        clusters = []
        endpoints = []

        global_bitmaps = []
        global_enums = []
        global_structs = []

        for item in items:
            if isinstance(item, Cluster):
                clusters.append(item)
            elif isinstance(item, Endpoint):
                endpoints.append(item)
            elif isinstance(item, Enum):
                global_enums.append(dataclasses.replace(item, is_global=True))
            elif isinstance(item, Bitmap):
                global_bitmaps.append(dataclasses.replace(item, is_global=True))
            elif isinstance(item, Struct):
                global_structs.append(dataclasses.replace(item, is_global=True))
            else:
                raise Exception("UNKNOWN idl content item: %r" % item)

        return Idl(clusters=clusters, endpoints=endpoints, global_bitmaps=global_bitmaps, global_enums=global_enums, global_structs=global_structs)

    def prefix_doc_comment(self):
        print("TODO: prefix")

    # Processing of (potential-doc)-comments:
    def c_comment(self, token: Token):
        """Processes comments starting with "/*" """
        if token.value.startswith("/**"):
            self.doc_comments.append(PrefixCppDocComment(token))


def _referenced_type_names(cluster: Cluster) -> Set[str]:
    """
    Return the names of all data types referenced by the given cluster.
    """
    types = set()
    for s in cluster.structs:
        for f in s.fields:
            types.add(f.data_type.name)

    for e in cluster.events:
        for f in e.fields:
            types.add(f.data_type.name)

    for a in cluster.attributes:
        types.add(a.definition.data_type.name)

    return types


class GlobalMapping:
    """
    Maintains global type mapping from an IDL
    """

    def __init__(self, idl: Idl):
        self.bitmap_map = {b.name: b for b in idl.global_bitmaps}
        self.enum_map = {e.name: e for e in idl.global_enums}
        self.struct_map = {s.name: s for s in idl.global_structs}

        self.global_types = set(self.bitmap_map.keys()).union(set(self.enum_map.keys())).union(set(self.struct_map.keys()))

        # Spec does not enforce unique naming in bitmap/enum/struct, however in practice
        # if we have both enum Foo and bitmap Foo for example, it would be impossible
        # to disambiguate `attribute Foo foo = 1` for the actual type we want.
        #
        # As a result, we do not try to namespace this and just error out
        if len(self.global_types) != len(self.bitmap_map) + len(self.enum_map) + len(self.struct_map):
            raise ValueError("Global type names are not unique.")

    def merge_global_types_into_cluster(self, cluster: Cluster) -> Cluster:
        """
        Merges all referenced global types (bitmaps/enums/structs) into the cluster types.
        This happens recursively.
        """
        global_types_added = set()

        changed = True
        while changed:
            changed = False
            for type_name in _referenced_type_names(cluster):
                if type_name not in self.global_types:
                    continue  # not a global type name

                if type_name in global_types_added:
                    continue  # already added

                # check if this is a global type
                if type_name in self.bitmap_map:
                    global_types_added.add(type_name)
                    changed = True
                    cluster.bitmaps.append(self.bitmap_map[type_name])
                elif type_name in self.enum_map:
                    global_types_added.add(type_name)
                    changed = True
                    cluster.enums.append(self.enum_map[type_name])
                elif type_name in self.struct_map:
                    global_types_added.add(type_name)
                    changed = True
                    cluster.structs.append(self.struct_map[type_name])

        return cluster


def _merge_global_types_into_clusters(idl: Idl) -> Idl:
    """
    Adds bitmaps/enums/structs from idl.global_* into clusters as long as
    clusters reference those type names
    """
    mapping = GlobalMapping(idl)
    return dataclasses.replace(idl, clusters=[mapping.merge_global_types_into_cluster(cluster) for cluster in idl.clusters])


class ParserWithLines:
    def __init__(self, skip_meta: bool, merge_globals: bool):
        self.transformer = MatterIdlTransformer(skip_meta)
        self.merge_globals = merge_globals

        # NOTE: LALR parser is fast. While Earley could parse more ambigous grammars,
        #       earley is much slower:
        #    - 0.39s LALR parsing of all-clusters-app.matter
        #    - 2.26s Earley parsing of the same thing.
        # For this reason, every attempt should be made to make the grammar context free
        self.parser = Lark.open(
            'matter_grammar.lark', rel_to=__file__, start='idl', parser='lalr', propagate_positions=True,
            maybe_placeholders=True,
            # separate callbacks to ignore from regular parsing (no tokens)
            # while still getting notified about them
            lexer_callbacks={
                'C_COMMENT': self.transformer.c_comment,
            }
        )

    def parse(self, file: str, file_name: Optional[str] = None):
        idl = self.transformer.transform(self.parser.parse(file))
        idl.parse_file_name = file_name

        # ZAP may generate the same definition of clusters several times.
        # Validate that if a cluster is defined, its definition is IDENTICAL
        #
        # TODO: this is not ideal and zap provides some iteration that seems
        #       to not care about side: `all_user_clusters_irrespective_of_side`
        #       however that one loses at least `description` and switches
        #       ordering.
        #
        # As a result, for now allow multiple definitions IF AND ONLY IF identical
        #
        # A zap PR to allow us to not need this is:
        #    https://github.com/project-chip/zap/pull/1216
        clusters: Dict[int, Cluster] = {}
        for c in idl.clusters:
            if c.code in clusters:
                if c != clusters[c.code]:
                    raise Exception(
                        f"Different cluster definition for {c.name}/{c.code}")
            else:
                clusters[c.code] = c
        idl.clusters = [c for c in clusters.values()]

        for comment in self.transformer.doc_comments:
            comment.appply_to_idl(idl, file)

        if self.merge_globals:
            idl = _merge_global_types_into_clusters(idl)

        return idl


def CreateParser(skip_meta: bool = False, merge_globals=True):
    """
    Generates a parser that will process a ".matter" file into a IDL

    Arguments:
       skip_meta - do not add metadata (line position) for items. Metadata is
                   useful for error reporting, however it does not work well
                   for unit test comparisons

       merge_globals - places global items (enums/bitmaps/structs) into any
                       clusters that reference them, so that cluster types
                       are self-sufficient. Useful as a backwards-compatible
                       code generation if global definitions are not supported.

    """
    return ParserWithLines(skip_meta, merge_globals)


if __name__ == '__main__':
    # This Parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.
    import pprint

    import click

    # Supported log levels, mapping string values required for argument
    # parsing into logging constants
    __LOG_LEVELS__ = {
        'debug': logging.DEBUG,
        'info': logging.INFO,
        'warn': logging.WARN,
        'fatal': logging.FATAL,
    }

    @click.command()
    @click.option(
        '--log-level',
        default='INFO',
        type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
        help='Determines the verbosity of script output.')
    @click.argument('filename')
    def main(log_level, filename=None):
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
        )

        logging.info("Starting to parse ...")
        data = CreateParser().parse(open(filename).read(), file_name=filename)
        logging.info("Parse completed")

        logging.info("Data:")
        pprint.pp(data)

    main(auto_envvar_prefix='CHIP')
