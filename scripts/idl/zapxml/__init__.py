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
import typing
import xml.sax.handler

from dataclasses import dataclass
from typing import Optional, Union, List

from idl.zapxml.handlers import Context, ZapXmlHandler
from idl.matter_idl_types import Idl


class ParseHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        super().__init__()
        self._idl = Idl()
        self._processing_stack = []
        # Context persists across all
        self._context = Context()

    def PrepareParsing(self, filename):
        # This is a bit ugly: filename keeps changing during parse
        # IDL meta is not prepared for this (as source is XML and .matter is
        # single file)
        self._idl.parse_file_name = filename

    def Finish(self) -> Idl:
        self._context.PostProcess(self._idl)
        return self._idl

    def startDocument(self):
        self._context.locator = self._locator
        self._processing_stack = [ZapXmlHandler(self._context, self._idl)]

    def endDocument(self):
        if len(self._processing_stack) != 1:
            raise Exception("Unexpected nesting!")

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

    @ property
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

    return handler.Finish()
