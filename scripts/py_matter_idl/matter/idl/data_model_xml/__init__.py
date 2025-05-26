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
import sys
import typing
import xml.sax.handler
from dataclasses import dataclass
from typing import List, Optional, Union

import click

from matter.idl.data_model_xml.handlers import Context, DataModelXmlHandler
from matter.idl.generators.idl import IdlGenerator
from matter.idl.generators.storage import InMemoryStorage
from matter.idl.matter_idl_parser import CreateParser
from matter.idl.matter_idl_types import Idl


class ParseHandler(xml.sax.handler.ContentHandler):
    """A parser for data model XML data definitions.

    Defers its processing to DataModelXmlHandler and keeps track of:
       - an internal context for all handlers
       - the parsed Idl structure that is incrementally built
       - sets up parsing location within the context
       - keeps track of ParsePath

    Overall converts a python SAX handler into matter.idl.zapxml.handlers
    """

    def __init__(self, include_meta_data=True):
        super().__init__()
        self._idl = Idl()
        self._processing_stack = []
        # Context persists across all
        self._context = Context()
        self._include_meta_data = include_meta_data
        self._locator = None

    def PrepareParsing(self, filename):
        # This is a bit ugly: filename keeps changing during parse
        # IDL meta is not prepared for this (as source is XML and .matter is
        # single file)
        if self._include_meta_data:
            self._idl.parse_file_name = filename

        self._context.file_name = filename

    def Finish(self) -> Idl:
        self._context.PostProcess(self._idl)
        return self._idl

    def startDocument(self):
        if self._include_meta_data and self._locator:
            self._context.locator = self._locator
        self._processing_stack = [
            DataModelXmlHandler(self._context, self._idl)]

    def endDocument(self):
        if len(self._processing_stack) != 1:
            raise Exception("Unexpected nesting!")

    def startElement(self, name: str, attrs):
        logging.debug("ELEMENT START: %r / %r" % (name, attrs))
        self._context.path.push(name)
        self._processing_stack.append(
            self._processing_stack[-1].GetNextProcessor(name, attrs))

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
    """Represents an input source for ParseXmls.

    Allows for named data sources to be parsed.
    """
    source: Union[str, typing.IO]  # filename or stream
    # actual filename to use, None if the source is a filename already
    name: Optional[str] = None

    @property
    def source_file_name(self):
        if self.name:
            return self.name
        return self.source  # assume string


def ParseXmls(sources: List[ParseSource], include_meta_data=True) -> Idl:
    """Parse one or more XML inputs and return the resulting Idl data.

    Params:
       sources - what to parse
       include_meta_data - if parsing location data should be included in the Idl
    """
    handler = ParseHandler(include_meta_data=include_meta_data)

    for source in sources:
        logging.info('Parsing %s...' % source.source_file_name)
        handler.PrepareParsing(source.source_file_name)

        parser = xml.sax.make_parser()
        parser.setContentHandler(handler)
        try:
            parser.parse(source.source)
        except AssertionError as e:
            logging.error("AssertionError %s at %r", e,
                          handler._context.GetCurrentLocationMeta())
            raise

    return handler.Finish()


def normalize_order(idl: Idl):
    """Re-sorts contents of things inside a cluster so that
       output is easily diffed by humans.
    """

    # This method exists because `zapt` generation of IDL files
    # are generally based on SQL select query ordering, likely
    # with some sort fields to achieve determinism
    #
    # However overall, especially if manual editing, it seems
    # easier to just fix a sort order instead of trying to
    # match another tool ordering that resides in another
    # code location.

    idl.clusters.sort(key=lambda c: c.name)

    for cluster in idl.clusters:
        cluster.enums.sort(key=lambda e: e.name)
        cluster.bitmaps.sort(key=lambda b: b.name)
        cluster.events.sort(key=lambda e: e.code)
        cluster.attributes.sort(key=lambda a: a.definition.code)
        cluster.structs.sort(key=lambda s: s.name)
        cluster.commands.sort(key=lambda c: c.code)


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
    show_default=True,
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--no-print',
    show_default=True,
    default=False,
    is_flag=True,
    help='Do not pring output data (parsed data)')
@click.option(
    "-o",
    "--output",
    default=None,
    type=click.Path(),
    help="Where to output the parsed IDL."
)
@click.option(
    "--compare",
    default=None,
    type=click.Path(exists=True),
    help="An input .matter IDL to compare with."
)
@click.option(
    "--compare-output",
    default=None,
    type=click.Path(),
    help="Where to output the compare IDL"
)
@click.argument('filenames', nargs=-1)
def main(log_level, no_print, output, compare, compare_output, filenames):
    """
    A program supporting parsing of CSA data model XML files and generating them
    as human readable IDL output.

    Also supports parsing and generating a diff against an existing .matter file,
    such as using:

    \b
       matter-data-model-xml-parser                                      \\
          --compare src/controller/data_model/controller-clusters.matter \\
          --compare-output out/orig.matter                               \\
          --output out/from_xml.matter                                   \\
          data_model/clusters/Switch.xml
    """
    logging.basicConfig(
        level=__LOG_LEVELS__[log_level],
        format='%(asctime)s %(levelname)-7s %(message)s',
    )

    if (compare is None) != (compare_output is None):
        logging.error(
            "Either both or none of --compare AND --compare-output must be set")
        sys.exit(1)

    logging.info("Starting to parse ...")

    sources = [ParseSource(source=name) for name in filenames]
    data = ParseXmls(sources)
    logging.info("Parse completed")

    if compare:
        other_idl = CreateParser(skip_meta=True).parse(
            open(compare).read(), file_name=compare)

        # ensure that input file is filtered to only interesting
        # clusters
        loaded_clusters = set([c.code for c in data.clusters])
        other_idl.clusters = [
            c for c in other_idl.clusters if c.code in loaded_clusters]

        # Ensure consistent ordering for compares
        normalize_order(data)
        normalize_order(other_idl)

        storage = InMemoryStorage()
        IdlGenerator(storage=storage, idl=other_idl).render(dry_run=False)
        with open(compare_output, 'wt', encoding="utf8") as o:
            o.write(storage.content)

    storage = InMemoryStorage()
    IdlGenerator(storage=storage, idl=data).render(dry_run=False)

    if output:
        with open(output, 'wt', encoding="utf8") as o:
            o.write(storage.content)
    elif not no_print:
        print(storage.content)
