#!/usr/bin/env python

import functools
import logging
from enum import Enum

from lark import Lark
from lark.visitors import Transformer, v_args

try:
    from .matter_idl_types import *
except:
    import os
    import sys
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))

    from matter_idl_types import *


def UnionOfAllFlags(flags_list):
    if not flags_list:
        return None
    return functools.reduce(lambda a, b: a | b, flags_list)


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
      optionality.
    """

    def __init__(self, skip_meta):
        self.skip_meta = skip_meta

    def positive_integer(self, tokens):
        """Numbers in the grammar are integers or hex numbers.
        """
        if len(tokens) != 1:
            raise Error("Unexpected argument counts")

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

    def id(self, tokens):
        """An id is a string containing an identifier
        """
        if len(tokens) != 1:
            raise Error("Unexpected argument counts")
        return tokens[0].value

    def type(self, tokens):
        """A type is just a string for the type
        """
        if len(tokens) != 1:
            raise Error("Unexpected argument counts")
        return tokens[0].value

    def data_type(self, tokens):
        if len(tokens) == 1:
            return DataType(name=tokens[0])
            # Just a string for data type
        elif len(tokens) == 2:
            return DataType(name=tokens[0], max_length=tokens[1])
        else:
            raise Error("Unexpected size for data type")

    @v_args(inline=True)
    def constant_entry(self, id, number):
        return ConstantEntry(name=id, code=number)

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
        field.qualities = UnionOfAllFlags(args[:-1]) or FieldQuality.NONE
        return field

    def server_cluster(self, _):
        return ClusterSide.SERVER

    def client_cluster(self, _):
        return ClusterSide.CLIENT

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

    def command(self, args):
        # The command takes 4 arguments if no input argument, 5 if input
        # argument is provided
        if len(args) != 5:
            args.insert(2, None)

        return Command(
            qualities=args[0],
            input_param=args[2], output_param=args[3], code=args[4],
            **args[1]
        )

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
    def endpoint_server_cluster(self, meta, id, *attributes):
        meta = None if self.skip_meta else ParseMetaData(meta)
        return AddServerClusterToEndpointTransform(ServerClusterInstantiation(parse_meta=meta, name=id, attributes=list(attributes)))

    @v_args(inline=True, meta=True)
    def cluster(self, meta, side, name, code, *content):
        meta = None if self.skip_meta else ParseMetaData(meta)

        result = Cluster(parse_meta=meta, side=side, name=name, code=code)

        for item in content:
            if type(item) == Enum:
                result.enums.append(item)
            elif type(item) == Bitmap:
                result.bitmaps.append(item)
            elif type(item) == Event:
                result.events.append(item)
            elif type(item) == Attribute:
                result.attributes.append(item)
            elif type(item) == Struct:
                result.structs.append(item)
            elif type(item) == Command:
                result.commands.append(item)
            else:
                raise Exception("UNKNOWN cluster content item: %r" % item)

        return result

    def idl(self, items):
        idl = Idl()

        for item in items:
            if type(item) == Enum:
                idl.enums.append(item)
            elif type(item) == Struct:
                idl.structs.append(item)
            elif type(item) == Cluster:
                idl.clusters.append(item)
            elif type(item) == Endpoint:
                idl.endpoints.append(item)
            else:
                raise Exception("UNKNOWN idl content item: %r" % item)

        return idl


class ParserWithLines:
    def __init__(self, parser, skip_meta: bool):
        self.parser = parser
        self.skip_meta = skip_meta

    def parse(self, file, file_name: str = None):
        idl = MatterIdlTransformer(self.skip_meta).transform(
            self.parser.parse(file))
        idl.parse_file_name = file_name
        return idl


def CreateParser(skip_meta: bool = False):
    """
    Generates a parser that will process a ".matter" file into a IDL
    """

    # NOTE: LALR parser is fast. While Earley could parse more ambigous grammars,
    #       earley is much slower:
    #    - 0.39s LALR parsing of all-clusters-app.matter
    #    - 2.26s Earley parsing of the same thing.
    # For this reason, every attempt should be made to make the grammar context free
    return ParserWithLines(Lark.open('matter_grammar.lark', rel_to=__file__, start='idl', parser='lalr', propagate_positions=True), skip_meta)


if __name__ == '__main__':
    # This Parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.
    import pprint

    import click
    import coloredlogs

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
        type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
        help='Determines the verbosity of script output.')
    @click.argument('filename')
    def main(log_level, filename=None):
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')

        logging.info("Starting to parse ...")
        data = CreateParser().parse(open(filename).read(), file_name=filename)
        logging.info("Parse completed")

        logging.info("Data:")
        pprint.pp(data)

    main(auto_envvar_prefix='CHIP')
