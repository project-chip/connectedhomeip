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
    from idl.matter_idl_types import Idl
except:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from idl.matter_idl_types import Idl

import idl.zapxml


@ dataclass
class ParseSource:
    source: Union[str, typing.IO]  # filename or stream
    name: Optional[str] = None  # actual filename to use, None if the source is a filename already

    @ property
    def source_file_name(self):
        if self.name:
            return self.name
        return self.source  # assume string


def ParseXmls(sources: List[ParseSource]) -> Idl:
    handler = idl.zapxml.ParseHandler()

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

    @ click.command()
    @ click.option(
        '--log-level',
        default='INFO',
        type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
        help='Determines the verbosity of script output.')
    @ click.argument('filenames', nargs=-1)
    def main(log_level, filenames):
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')

        logging.info("Starting to parse ...")

        sources = [ParseSource(source=name) for name in filenames]
        data = ParseXmls(sources)
        logging.info("Parse completed")

        logging.info("Data:")
        pprint.pp(data)

    main()
