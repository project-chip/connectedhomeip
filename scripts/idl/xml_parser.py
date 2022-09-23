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
import os
import typing
import xml.sax
import xml.sax.handler

from dataclasses import dataclass, field
from typing import Optional, Union, List

try:
    from idl.matter_idl_types import *
except:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from idl.matter_idl_types import *


class HandledDepth:
    """Defines how deep a XML element has been handled."""
    NOT_HANDLED = enum.auto()  # Unknown/parsed element
    ENTIRE_TREE = enum.auto()  # Entire tree can be ignored
    SINGLE_TAG = enum.auto()  # Single tag processed, but not sub-items


def ParseInt(value: str) -> int:
    if value.startswith('0x'):
        return int(value[2:], 16)
    else:
        return int(value)


def ParseAclRole(attrs) -> AccessPrivilege:
    # XML seems to use both role and privilege to mean the same thing
    # they are used interchangeably
    if 'role' in attrs:
        role = attrs['role']
    else:
        role = attrs['privilege']

    if role.lower() == 'view':
        return AccessPrivilege.VIEW
    elif role.lower() == 'operate':
        return AccessPrivilege.OPERATE
    elif role.lower() == 'manage':
        return AccessPrivilege.MANAGE
    elif role.lower() == 'administer':
        return AccessPrivilege.ADMINISTER
    else:
        raise Exception('Unknown ACL role: %r' % role)


def AttrsToAttribute(attrs) -> Attribute:
    if attrs['type'].lower() == 'array':
        data_type = DataType(name=attrs['entryType'])
    else:
        data_type = DataType(name=attrs['type'])

    if 'length' in attrs:
        data_type.max_length = ParseInt(attrs['length'])

    field = Field(
        data_type=data_type,
        code=ParseInt(attrs['code']),
        name=None,
        is_list=(attrs['type'].lower() == 'array')
    )

    attribute = Attribute(definition=field)

    if attrs.get('optional', "false").lower() == 'true':
        attribute.definition.attributes.add(FieldAttribute.OPTIONAL)

    if attrs.get('isNullable', "false").lower() == 'true':
        attribute.definition.attributes.add(FieldAttribute.NULLABLE)

    if attrs.get('readable', "true").lower() == 'true':
        attribute.tags.add(AttributeTag.READABLE)

    if attrs.get('writable', "false").lower() == 'true':
        attribute.tags.add(AttributeTag.WRITABLE)

    # TODO: XML does not seem to contain information about
    #   - NOSUBSCRIBE

    # TODO: do we care about default value at all?
    #       General storage of default only applies to instantiation

    return attribute


class ProcessingPath:
    def __init__(self, paths: List[str] = None):
        if paths is None:
            paths = []
        self.paths = paths

    def push(self, name: str):
        self.paths.append(name)

    def pop(self):
        self.paths.pop()

    def __str__(self):
        return '::'.join(self.paths)

    def __repr__(self):
        return 'ProcessingPath(%r)' % self.paths


class IdlPostProcessor:
    """Defines a callback that will apply after an entire parsing
       is complete.
    """

    def FinalizeProcessing(self, idl: Idl):
        """Update idl with any post-processing directives."""
        pass


class ProcessingContext:
    def __init__(self, locator: Optional[xml.sax.xmlreader.Locator] = None):
        self.path = ProcessingPath()
        self.locator = locator
        self._not_handled = set()
        self._idl_post_processors = []

        # Map of code -> attribute
        self._global_attributes = {}

    def GetCurrentLocationMeta(self) -> ParseMetaData:
        if not self.locator:
            return None

        return ParseMetaData(line=self.locator.getLineNumber(), column=self.locator.getColumnNumber())

    def GetGlobalAttribute(self, code):
        if code in self._global_attributes:
            return self._global_attributes[code]

        raise Exception(
            'Global attribute 0x%X (%d) not found. You probably need to load global-attributes.xml' % (code, code))

    def AddGlobalAttribute(self, attribute: Attribute):
        # NOTE: this may get added several times as both 'client' and 'server'
        #       however matter should not differentiate between the two
        code = attribute.definition.code
        logging.info('Adding global attribute 0x%X (%d): %s' % (code, code, attribute.definition.name))

        self._global_attributes[code] = attribute

    def MarkTagNotHandled(self):
        path = str(self.path)
        if path not in self._not_handled:
            logging.warning("TAG %s was not handled/recognized" % path)
            self._not_handled.add(path)

    def AddIdlPostProcessor(self, processor: IdlPostProcessor):
        self._idl_post_processors.append(processor)

    def PostProcess(self, idl: Idl):
        for p in self._idl_post_processors:
            p.FinalizeProcessing(idl)

        self._idl_post_processors = []


class ElementProcessor:
    """A generic element processor.

       XML processing will be done in the form of a stack. whenever
       a new element comes in, its processor is moved to the top of
       the stack and poped once the element ends.
    """

    def __init__(self, context: ProcessingContext, handled=HandledDepth.NOT_HANDLED):
        self.context = context
        self._handled = handled

    def GetNextProcessor(self, name, attrs):
        """Get the next processor to use for the given name"""

        if self._handled == HandledDepth.SINGLE_TAG:
            handled = HandledDepth.NOT_HANDLED
        else:
            handled = self._handled

        return ElementProcessor(context=self.context, handled=handled)

    def HandleContent(self, content):
        """Processes some content"""
        pass

    def EndProcessing(self):
        """Finalizes the processing of the current element"""
        if self._handled == HandledDepth.NOT_HANDLED:
            self.context.MarkTagNotHandled()


class ClusterNameProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def HandleContent(self, content):
        self._cluster.name = content.replace(' ', '')


class AttributeDescriptionProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, attribute):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._attribute = attribute

    def HandleContent(self, content):
        self._attribute.definition.name = content.replace(' ', '')


class ClusterCodeProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def HandleContent(self, content):
        self._cluster.code = ParseInt(content)


class EventProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, cluster, attrs):
        super().__init__(context)
        self._cluster = cluster

        if attrs['priority'] == 'debug':
            priority = EventPriority.DEBUG
        elif attrs['priority'] == 'info':
            priority = EventPriority.INFO
        elif attrs['priority'] == 'critical':
            priority = EventPriority.CRITICAL
        else:
            raise Exception("Unknown event priority: %s" % attrs['priority'])

        self._event = Event(
            priority=priority,
            code=ParseInt(attrs['code']),
            name=attrs['name'],
            fields=[],
        )

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'field':
            data_type = DataType(name=attrs['type'])
            if 'length' in attrs:
                data_type.max_length = ParseInt(attrs['length'])

            field = Field(
                data_type=data_type,
                code=ParseInt(attrs['id']),
                name=attrs['name'],
                is_list=(attrs.get('array', 'false').lower() == 'true'),
            )

            if attrs.get('optional', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.OPTIONAL)

            if attrs.get('isNullable', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.NULLABLE)

            self._event.fields.append(field)
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return ElementProcessor(self.context)

    def EndProcessing(self):
        self._cluster.events.append(self._event)


class AttributeProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, cluster, attrs):
        super().__init__(context)
        self._cluster = cluster
        self._attribute = AttrsToAttribute(attrs)

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'access':
            if 'modifier' in attrs:
                if attrs['modifier'] != 'fabric-scoped':
                    raise Exception("UNKNOWN MODIFIER: %s" % attrs['modifier'])
                self._attribute.tags.add(AttributeTag.FABRIC_SCOPED)
            else:
                role = ParseAclRole(attrs)

                if attrs['op'] == 'read':
                    self._attribute.readacl = role
                elif attrs['op'] == 'write':
                    self._attribute.writeacl = role
                else:
                    logging.error("Unknown access: %r" % attrs['op'])
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return AttributeDescriptionProcessor(self.context, self._attribute)
        else:
            return ElementProcessor(self.context)

    def HandleContent(self, content):
        # Content generally is the name EXCEPT if access controls
        # exist, in which case `description` contains the name
        content = content.strip()
        if content and not self._attribute.definition.name:
            self._attribute.definition.name = content

    def EndProcessing(self):
        if self._attribute.definition.name is None:
            raise Exception("Name for attribute was not parsed.")

        self._cluster.attributes.append(self._attribute)


class StructProcessor(ElementProcessor, IdlPostProcessor):
    def __init__(self, context: ProcessingContext, attrs):
        super().__init__(context)
        self._cluster_code = None  # if set, struct belongs to a specific cluster
        self._struct = Struct(name=attrs['name'], fields=[])
        self._field_index = 0
        # The following are not set:
        #    - tag not set because not a request/response
        #    - code not set because not a response

        # TODO: handle this isFabricScoped attribute
        self._is_fabric_scoped = (attrs.get('isFabricScoped', "false").lower() == 'true')

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'item':
            data_type = DataType(
                name=attrs['type']
            )

            # TODO: handle isFabricSensitive

            if 'fieldId' in attrs:
                self._field_index = ParseInt(attrs['fieldId'])
            else:
                # NOTE: code does NOT exist, so the number is incremental here
                #       this seems a defficiency in XML format.
                self._field_index += 1

            if 'length' in attrs:
                data_type.max_length = ParseInt(attrs['length'])

            field = Field(
                data_type=data_type,
                code=self._field_index,
                name=attrs['name'],
                is_list=(attrs.get('array', 'false').lower() == 'true'),
            )

            if attrs.get('optional', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.OPTIONAL)

            if attrs.get('isNullable', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.NULLABLE)

            self._struct.fields.append(field)
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'cluster':
            if self._cluster_code is not None:
                raise Exception('Multiple cluster codes for structr %s' % self._struct.name)
            self._cluster_code = ParseInt(attrs['code'])
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return ElementProcessor(self.context)

    def FinalizeProcessing(self, idl: Idl):
        # We have two choices of adding an enum:
        #   - inside a cluster if a code exists
        #   - inside top level if a code does not exist

        if self._cluster_code is None:
            idl.structs.append(self._struct)
        else:
            found = False
            for c in idl.clusters:
                if c.code == self._cluster_code:
                    c.structs.append(self._struct)
                    found = True

            if not found:
                logging.error('Enum %s could not find its cluster (code %d/0x%X)' %
                              (self._struct.name, self._cluster_code, self._cluster_code))

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)


class EnumProcessor(ElementProcessor, IdlPostProcessor):
    def __init__(self, context: ProcessingContext, attrs):
        super().__init__(context)
        self._cluster_code = None  # if set, enum belongs to a specific cluster
        self._enum = Enum(name=attrs['name'], base_type=attrs['type'], entries=[])

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'item':
            self._enum.entries.append(ConstantEntry(
                name=attrs['name'],
                code=ParseInt(attrs['value'])
            ))
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'cluster':
            if self._cluster_code is not None:
                raise Exception('Multiple cluster codes for enum %s' % self._enum.name)
            self._cluster_code = ParseInt(attrs['code'])
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return ElementProcessor(self.context)

    def FinalizeProcessing(self, idl: Idl):
        # We have two choices of adding an enum:
        #   - inside a cluster if a code exists
        #   - inside top level if a code does not exist

        if self._cluster_code is None:
            idl.enums.append(self._enum)
        else:
            found = False
            for c in idl.clusters:
                if c.code == self._cluster_code:
                    c.enums.append(self._enum)
                    found = True

            if not found:
                logging.error('Enum %s could not find its cluster (code %d/0x%X)' %
                              (self._enum.name, self._cluster_code, self._cluster_code))

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)


class BitmapProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, attrs):
        super().__init__(context)
        self._cluster_code = None
        self._bitmap = Bitmap(name=attrs['name'], base_type=attrs['type'], entries=[])

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'cluster':
            if self._cluster_code is not None:
                raise Exception('Multiple cluster codes for structr %s' % self._struct.name)
            self._cluster_code = ParseInt(attrs['code'])
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'field':
            self._bitmap.entries.append(ConstantEntry(
                name=attrs['name'],
                code=ParseInt(attrs['mask'])
            ))
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return ElementProcessor(self.context)

    def FinalizeProcessing(self, idl: Idl):
        # We have two choices of adding an enum:
        #   - inside a cluster if a code exists
        #   - inside top level if a code does not exist
        if self._cluster_code is None:
            # Log only instead of critical, as not our XML is well formed.
            # For example at the time of writing this, SwitchFeature in switch-cluster.xml
            # did not have a code associated with it.
            logging.error("Bitmap %r has no cluster code" % self._bitmap)
            return

        found = False
        for c in idl.clusters:
            if c.code == self._cluster_code:
                c.bitmaps.append(self._bitmap)
                found = True

        if not found:
            logging.error('Enum %s could not find its cluster (code %d/0x%X)' %
                          (self._bitmap.name, self._cluster_code, self._cluster_code))

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)


class CommandProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, cluster, attrs):
        super().__init__(context)
        self._cluster = cluster
        self._command = None
        self._struct = Struct(name=attrs['name'], fields=[])
        self._field_index = 0  # commands DO NOT support field index it seems

        if attrs['source'].lower() == 'client':
            self._struct.tag = StructTag.REQUEST

            name = attrs['name']

            if name.endswith('Request'):
                request_name = name
                command_name = name[:-7]
            else:
                request_name = name+'Request'
                command_name = name

            if 'response' in attrs:
                response_name = attrs['response']
            else:
                response_name = 'DefaultResponse'

            # TODO: what if no response? DefaultResponse?
            self._command = Command(
                name=name,
                code=ParseInt(attrs['code']),
                input_param=request_name,
                output_param=response_name,
            )

            # TODO: command attributes:
            #    - timed invoke
            #    - fabric scoped
        else:
            self._struct.tag = StructTag.RESPONSE
            self._struct.code = ParseInt(attrs['code'])

    def GetArgumentField(self, attrs):
        data_type = DataType(name=attrs['type'])

        if 'length' in attrs:
            data_type.max_length = ParseInt(attrs['length'])

        self._field_index += 1

        field = Field(
            data_type=data_type,
            code=self._field_index,
            name=attrs['name'],
            is_list=(attrs.get('array', 'false') == 'true')
        )

        if attrs.get('optional', "false").lower() == 'true':
            field.attributes.add(FieldAttribute.OPTIONAL)

        if attrs.get('isNullable', "false").lower() == 'true':
            field.attributes.add(FieldAttribute.NULLABLE)

        return field

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'access':
            if attrs['op'] != 'invoke':
                raise Exception('Unknown access for %r' % self._struct)

            if self._command:
                self._command.invokeacl = ParseAclRole(attrs)
            else:
                logging.warning("Ignored access role for reply %r" % self._struct)
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'arg':
            self._struct.fields.append(self.GetArgumentField(attrs))
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            # TODO: implement
            return ElementProcessor(self.context)

    def EndProcessing(self):

        if self._struct.fields:
            self._cluster.structs.append(self._struct)
        else:
            # no input
            self._command.input_param = None

        if self._command:
            self._cluster.commands.append(self._command)


class ClusterProcessor(ElementProcessor):
    """Handles configurator/cluster processing"""

    def __init__(self, context: ProcessingContext, idl: Idl):
        super().__init__(context)
        self._cluster = Cluster(
            side=ClusterSide.CLIENT,
            name=None,
            code=None,
            parse_meta=context.GetCurrentLocationMeta()
        )
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'code':
            return ClusterCodeProcessor(self.context, self._cluster)
        elif name.lower() == 'name':
            return ClusterNameProcessor(self.context, self._cluster)
        elif name.lower() == 'attribute':
            return AttributeProcessor(self.context, self._cluster, attrs)
        elif name.lower() == 'event':
            return EventProcessor(self.context, self._cluster, attrs)
        elif name.lower() == 'globalattribute':
            # We ignore 'side' and 'value' since they do not seem useful
            self._cluster.attributes.append(self.context.GetGlobalAttribute(ParseInt(attrs['code'])))
            return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'command':
            return CommandProcessor(self.context, self._cluster, attrs)
        elif name.lower() in ['define', 'description', 'domain', 'tag', 'client', 'server']:
            # NOTE: we COULD use client and server to create separate definitions
            #       of each, but the usefulness of this is unclear as the definitions are
            #       likely identical and matter has no concept of differences between the two
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return ElementProcessor(self.context)

    def EndProcessing(self):
        if self._cluster.name is None:
            raise Exception("Missing cluster name")
        elif self._cluster.code is None:
            raise Exception("Missing cluster code")

        self._idl.clusters.append(self._cluster)


class GlobalAttributeProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, attribute: Attribute):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._attribute = attribute

    def HandleContent(self, content):
        # Content generally is the name EXCEPT if access controls
        # exist, in which case `description` contains the name
        #
        # Global attributes do not currently have access controls, so this
        # case is not handled here
        content = content.strip()
        if content and not self._attribute.definition.name:
            self._attribute.definition.name = content

    def EndProcessing(self):
        if self._attribute.definition.name is None:
            raise Exception("Name for attribute was not parsed.")

        self.context.AddGlobalAttribute(self._attribute)


class GlobalProcessor(ElementProcessor):
    """Processes configurator/global """

    def __init__(self, context: ProcessingContext):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'attribute':
            if attrs['side'].lower() == 'client':
                # We expect to also have 'server' equivalent, so ignore client
                # side attributes
                logging.debug('Ignoring global client-side attribute %s' % (attrs['code']))
                return ElementProcessor(self.context, handled=HandledDepth.SINGLE_TAG)

            return GlobalAttributeProcessor(self.context, AttrsToAttribute(attrs))
        else:
            return ElementProcessor(self.context)


class ConfiguratorProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, idl: Idl):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'cluster':
            return ClusterProcessor(self.context, self._idl)
        elif name.lower() == 'enum':
            return EnumProcessor(self.context, attrs)
        elif name.lower() == 'struct':
            return StructProcessor(self.context, attrs)
        elif name.lower() == 'bitmap':
            return BitmapProcessor(self.context, attrs)
        elif name.lower() == 'domain':
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name.lower() == 'global':
            return GlobalProcessor(self.context)
        else:
            return ElementProcessor(self.context)


class ZapXmlProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, idl: Idl):
        super().__init__(context)
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'configurator':
            return ConfiguratorProcessor(self.context, self._idl)
        else:
            return ElementProcessor(self.context)


class ParseHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        super().__init__()
        self._idl = Idl()
        self._processing_stack = []
        # Context persists across all
        self._context = ProcessingContext()

    def PrepareParsing(self, filename):
        # This is a bit ugly: filename keeps changing during parse
        # IDL meta is not prepared for this (as source is XML and .matter is
        # single file)
        self._idl.parse_file_name = filename

    def ProcessResults(self) -> Idl:
        return self._idl

    def startDocument(self):
        self._context.locator = self._locator
        self._processing_stack = [ZapXmlProcessor(self._context, self._idl)]

    def endDocument(self):
        if len(self._processing_stack) != 1:
            raise Exception("Unexpected nesting!")

        self._context.PostProcess(self._idl)

    def startElement(self, name: str, attrs):
        logging.debug("ELEMENT START: %r / %r" % (name, attrs))
        self._context.path.push(name)
        self._processing_stack.append(self._processing_stack[-1].GetNextProcessor(name, attrs))

    def endElement(self, name: str):
        logging.debug("ELEMENT END: %r" % name)

        last = self._processing_stack.pop()
        last.EndProcessing()

        # important to pop AFTER processing end to allow processing
        # end to access the current context
        self._context.path.pop()

    def characters(self, content):
        self._processing_stack[-1].HandleContent(content)


@dataclass
class ParseSource:
    source: Union[str, typing.IO]  # filename or stream
    name: Optional[str] = None  # actual filename to use, None if the source is a filename already

    @property
    def source_file_name(self):
        if self.name:
            return self.name
        return self.source  # assume string


def ParseXmls(sources: List[ParseSource]) -> Idl:
    handler = ParseHandler()

    for source in sources:
        logging.info('Parsing %s...' % source.source_file_name)
        handler.PrepareParsing(source.source_file_name)

        parser = xml.sax.make_parser()
        parser.setContentHandler(handler)
        parser.parse(source.source)

    return handler.ProcessResults()


if __name__ == '__main__':
    # This Parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.
    import click
    import coloredlogs
    import pprint

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
    @click.option(
        '--global-attributes',
        help='What global attributes file to preload')
    @click.argument('filename')
    def main(log_level, global_attributes=None, filename=None):
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')

        logging.info("Starting to parse ...")

        sources = []
        if global_attributes is not None:
            sources.append(ParseSource(source=global_attributes))
        if filename is not None:
            sources.append(ParseSource(source=filename))

        data = ParseXmls(sources)
        logging.info("Parse completed")

        logging.info("Data:")
        pprint.pp(data)

    main()
