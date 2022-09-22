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
import xml.sax
import xml.sax.handler

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


def ParseAclRole(role: str) -> AccessPrivilege:
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


class ProcessingContext:
    def __init__(self, locator: Optional[xml.sax.xmlreader.Locator] = None):
        self.path = ProcessingPath()
        self.locator = locator
        self._not_handled = set()

    def GetCurrentLocationMeta(self) -> ParseMetaData:
        if not self.locator:
            return None

        return ParseMetaData(line=self.locator.getLineNumber(), column=self.locator.getColumnNumber())

    def MarkTagNotHandled(self):
        path = str(self.path)
        if path not in self._not_handled:
            logging.warning("TAG %s was not handled/recognized" % path)
            self._not_handled.add(path)


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
        super().__init__(context)
        self._cluster = cluster

    def HandleContent(self, content):
        self._cluster.code = ParseInt(content)


class AttributeProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, cluster, attrs):
        super().__init__(context)
        self._cluster = cluster

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

        self._attribute = Attribute(definition=field)

        if attrs.get('optional', "false").lower() == 'true':
            self._attribute.definition.attributes.add(FieldAttribute.OPTIONAL)

        if attrs.get('isNullable', "false").lower() == 'true':
            self._attribute.definition.attributes.add(FieldAttribute.NULLABLE)

        if attrs.get('readable', "true").lower() == 'true':
            self._attribute.tags.add(AttributeTag.READABLE)

        if attrs.get('writable', "false").lower() == 'true':
            self._attribute.tags.add(AttributeTag.WRITABLE)

        # TODO: XML does not seem to contain information about
        #   - NOSUBSCRIBE
        #   - FABRIC_SCOPED

        # TODO: do we care about default value at all?
        #       General storage of default only applies to instantiation

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'access':
            if attrs['op'] == 'read':
                self._attribute.readacl = ParseAclRole(attrs['role'])
            elif attrs['op'] == 'write':
                self._attribute.writeacl = ParseAclRole(attrs['role'])
            else:
                logging.warning("Unknown access: %r" % attrs['op'])
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
        elif name.lower() in ['define', 'description', 'domain']:
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return ElementProcessor(self.context)

    def EndProcessing(self):
        if self._cluster.name is None:
            raise Exception("Missing cluster name")
        elif self._cluster.code is None:
            raise Exception("Missing cluster code")

        self._idl.clusters.append(self._cluster)


class ConfiguratorProcessor(ElementProcessor):
    def __init__(self, context: ProcessingContext, idl: Idl):
        super().__init__(context)
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'cluster':
            return ClusterProcessor(self.context, self._idl)
        elif name.lower() == 'domain':
            return ElementProcessor(self.context, handled=HandledDepth.ENTIRE_TREE)
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
    def __init__(self, filename):
        super().__init__()
        self._idl = Idl(parse_file_name=filename)
        self._processing_stack = []
        self._context = None

    def ProcessResults(self):
        return self._idl

    def startDocument(self):
        self._context = ProcessingContext(self._locator)
        self._processing_stack = [ZapXmlProcessor(self._context, self._idl)]

    def endDocument(self):
        if len(self._processing_stack) != 1:
            raise Exception("Unexpected nesting!")

    def startElement(self, name, attrs):
        logging.debug("ELEMENT START: %r / %r" % (name, attrs))
        self._context.path.push(name)
        self._processing_stack.append(self._processing_stack[-1].GetNextProcessor(name, attrs))

    def endElement(self, name):
        logging.debug("ELEMENT END: %r" % name)

        last = self._processing_stack.pop()
        last.EndProcessing()

        # important to pop AFTER processing end to allow processing
        # end to access the current context
        self._context.path.pop()

    def characters(self, content):
        self._processing_stack[-1].HandleContent(content)


def ParseXml(filename_or_stream, filename):
    handler = ParseHandler(filename)
    parser = xml.sax.make_parser()
    parser.setContentHandler(handler)
    parser.parse(filename_or_stream)

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
    @click.argument('filename')
    def main(log_level, filename=None):
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')

        logging.info("Starting to parse ...")
        data = ParseXml(filename, filename)
        logging.info("Parse completed")

        logging.info("Data:")
        pprint.pp(data)

    main()
