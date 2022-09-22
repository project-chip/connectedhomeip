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

import logging
import os
import xml.sax;
import xml.sax.handler;

try:
    from idl.matter_idl_types import *
except:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from idl.matter_idl_types import *


class ElementProcessor:
    """A generic element processor.

       XML processing will be done in the form of a stack. whenever
       a new element comes in, its processor is moved to the top of
       the stack and poped once the element ends.
    """
    def GetNextProcessor(self, name, attrs):
        """Get the next processor to use for the given name"""
        return ElementProcessor()

    def HandleContent(self, content):
        """Processes some content"""
        pass

    def EndProcessing(self):
        """Finalizes the processing of the current element"""
        pass

class ClusterNameProcessor(ElementProcessor):
    def __init__(self, cluster):
        self._cluster = cluster

    def HandleContent(self, content):
        self._cluster.name = content.replace(' ','')
        


class ClusterCodeProcessor(ElementProcessor):
    def __init__(self, cluster):
        self._cluster = cluster

    def HandleContent(self, content):
        if content.startswith('0x'):
            code = int(content[2:], 16)
        else:
            code = int(content)
        self._cluster.code = code

class ClusterProcessor(ElementProcessor):
    """Handles configurator/cluster processing"""
    def __init__(self, idl):
        self._cluster = Cluster(
            side = ClusterSide.CLIENT,
            name = None,
            code = None,
        )
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'code':
            return ClusterCodeProcessor(self._cluster)
        elif name.lower() == 'name':
            return ClusterNameProcessor(self._cluster)
        else:
            return ElementProcessor()

    def EndProcessing(self):
        if self._cluster.name is None:
            raise Exception("Missing cluster name")
        elif self._cluster.code is None:
            raise Exception("Missing cluster code")

        self._idl.clusters.append(self._cluster)


class ConfiguratorProcessor(ElementProcessor):
    def __init__(self, idl):
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'cluster':
            return ClusterProcessor(self._idl)
        else:
            return ElementProcessor()


class ZapXmlProcessor(ElementProcessor):
    def __init__(self, idl):
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'configurator':
            return ConfiguratorProcessor(self._idl)
        else:
            return ElementProcessor()

class ParseHandler(xml.sax.handler.ContentHandler):
    def __init__(self, filename):
        super(xml.sax.handler.ContentHandler, self).__init__()
        self._idl = Idl(parse_file_name = filename)
        self._processing_stack = []

    def ProcessResults(self):
        return self._idl

    def startDocument(self):
        self._processing_stack = [ZapXmlProcessor(self._idl)]

    def endDocument(self):
        if len(self._processing_stack) != 1:
            raise Exception("Unexpected nesting!")

    def startElement(self, name, attrs):
        logging.debug("ELEMENT START: %r / %r" % (name, attrs))
        self._processing_stack.append(self._processing_stack[-1].GetNextProcessor(name, attrs))

    def endElement(self, name):
        logging.debug("ELEMENT END: %r" % name)
        last = self._processing_stack.pop()
        last.EndProcessing()

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
