#!/usr/bin/env python

import logging

from lark import Lark
from lark.visitors import Transformer, v_args

try:
    from .matter_idl_types import *
except:
    import os
    import sys
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))

    from matter_idl_types import *


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

    def number(self, tokens):
        """Numbers in the grammar are integers or hex numbers.
        """
        if len(tokens) != 1:
            raise Error("Unexpected argument counts")

        n = tokens[0].value
        if n.startswith('0x'):
            return int(n[2:], 16)
        else:
            return int(n)

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
        return FieldAttribute.OPTIONAL

    def nullable(self, _):
        return FieldAttribute.NULLABLE

    def attr_readonly(self, _):
        return AttributeTag.READABLE

    def attr_nosubscribe(self, _):
        return AttributeTag.NOSUBSCRIBE

    def critical_priority(self, _):
        return EventPriority.CRITICAL

    def info_priority(self, _):
        return EventPriority.INFO

    def debug_priority(self, _):
        return EventPriority.DEBUG

    def endpoint_server_cluster(self, _):
        return EndpointContentType.SERVER_CLUSTER

    def endpoint_binding_to_cluster(self, _):
        return EndpointContentType.CLIENT_BINDING

    def timed_command(self, _):
        return CommandAttribute.TIMED_INVOKE

    def command_attributes(self, attrs):
        # List because attrs is a tuple
        return set(list(attrs))

    def struct_field(self, args):
        # Last argument is the named_member, the rest
        # are attributes
        field = args[-1]
        field.attributes = set(args[:-1])
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
        # A command has 4 arguments if no input or
        # 5 arguments if input parameter is available
        param_in = None
        if len(args) > 4:
            param_in = args[2]

        return Command(
            attributes=args[0], input_param=param_in, output_param=args[-2], code=args[-1], **args[1])

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
        return Event(priority=args[0], code=args[2], fields=args[3:], **args[1])

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
                    raise Exception("Unknown attribute operation: %r" % operation)

        return (args[-1], acl)

    def attribute(self, args):
        # Input arguments are:
        #   -  tags (0 or more)
        #   -  attribute_with_access (i.e. pair of definition and acl arguments)
        tags = set(args[:-1])
        (definition, acl) = args[-1]

        # until we support write only (and need a bit of a reshuffle)
        # if the 'attr_readonly == READABLE' is not in the list, we make things
        # read/write
        if AttributeTag.READABLE not in tags:
            tags.add(AttributeTag.READABLE)
            tags.add(AttributeTag.WRITABLE)

        return Attribute(definition=definition, tags=tags, **acl)

    @v_args(inline=True)
    def struct(self, id, *fields):
        return Struct(name=id, fields=list(fields))

    @v_args(inline=True)
    def request_struct(self, value):
        value.tag = StructTag.REQUEST
        return value

    @v_args(inline=True)
    def response_struct(self, id, code, *fields):
        return Struct(name=id, tag=StructTag.RESPONSE, code=code, fields=list(fields))

    @v_args(inline=True)
    def endpoint(self, number, *clusters):
        endpoint = Endpoint(number=number)

        for t, name in clusters:
            if t == EndpointContentType.CLIENT_BINDING:
                endpoint.client_bindings.append(name)
            elif t == EndpointContentType.SERVER_CLUSTER:
                endpoint.server_clusters.append(name)
            else:
                raise Error("Unknown endpoint content: %r" % t)

        return endpoint

    @v_args(inline=True)
    def endpoint_cluster(self, t, id):
        return (t, id)

    @v_args(inline=True)
    def cluster(self, side, name, code, *content):
        result = Cluster(side=side, name=name, code=code)

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


def CreateParser():
    """
    Generates a parser that will process a ".matter" file into a IDL
    """
    return Lark.open('matter_grammar.lark', rel_to=__file__, start='idl', parser='lalr', transformer=MatterIdlTransformer())


if __name__ == '__main__':
    # This Parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.
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
        data = CreateParser().parse(open(filename).read())
        logging.info("Parse completed")

        logging.info("Data:")
        print(data)

    main()
